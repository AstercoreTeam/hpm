// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// core/src/registry/official_key.cpp
#include <hpm/registry/official_key.h>

#include <hpm/registry/signature.h>

#include <curl/curl.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include "hpm/config.h"

namespace hpm::registry {

namespace {

// 官方公钥默认地址（硬编码）
constexpr const char* kOfficialUrl = HPM_OFFICIAL_PUB_URL;

size_t writeToFile(void* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* f = static_cast<std::ofstream*>(userdata);
    f->write(static_cast<char*>(ptr), static_cast<std::streamsize>(size * nmemb));
    return size * nmemb;
}

} // namespace

const char* OfficialKeyManager::officialUrl() {
    // 环境变量覆盖（测试用）
    if (const char* env = std::getenv(HPM_ENV_OFFICIAL_PUB); env && *env) {
        return env;
    }
    return kOfficialUrl;
}

OfficialKeyManager::OfficialKeyManager(std::filesystem::path cacheDir)
    : cacheDir_(std::move(cacheDir)) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::filesystem::path OfficialKeyManager::localPath() const {
    return cacheDir_ / HPM_FILE_OFFICIAL_PUB;
}

std::filesystem::path OfficialKeyManager::fingerprintPath() const {
    return cacheDir_ / HPM_FILE_OFFICIAL_FP;
}

Result<std::filesystem::path> OfficialKeyManager::ensure() {
    auto local = localPath();
    auto fpPath = fingerprintPath();
    auto url = officialUrl();

    // 下载到临时文件
    auto tmp = local;
    tmp += ".tmp";

    auto dl = download(tmp);
    if (!dl) return dl.error();

    // 验证格式
    if (!isValidPublicKey(tmp)) {
        std::filesystem::remove(tmp);
        return Error(ErrorCode::InvalidField,
                     "下载的公钥格式无效: " + std::string(url));
    }

    // 计算指纹
    auto newFp = publicKeyFingerprint(tmp);

    // 本地已有公钥？
    bool hasLocal = std::filesystem::exists(local);
    std::string oldFp;
    if (hasLocal) {
        std::ifstream f(fpPath);
        if (f) std::getline(f, oldFp);
    }

    // 指纹变化或首次
    bool needPrompt = !hasLocal || oldFp != newFp;

    if (needPrompt) {
        bool ok = promptFingerprint(newFp, hasLocal && oldFp != newFp);
        if (!ok) {
            std::filesystem::remove(tmp);
            return Error(ErrorCode::PermissionDenied,
                         "用户拒绝官方公钥指纹");
        }

        // 保存指纹
        std::ofstream f(fpPath);
        f << newFp << "\n";
    }

    // 替换本地公钥
    std::error_code ec;
    std::filesystem::rename(tmp, local, ec);
    if (ec) {
        std::filesystem::remove(tmp);
        return Error(ErrorCode::IoError,
                     "无法保存公钥: " + ec.message());
    }

    return local;
}

Result<void> OfficialKeyManager::download(const std::filesystem::path& dest) {
    std::error_code ec;
    std::filesystem::create_directories(dest.parent_path(), ec);

    std::ofstream f(dest, std::ios::binary);
    if (!f) {
        return Error(ErrorCode::IoError, "无法写入: " + dest.string());
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return Error(ErrorCode::NetworkError, "curl_easy_init 失败");
    }

    auto url = officialUrl();
    bool isFile = (std::string(url).rfind("file://", 0) == 0);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &f);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HPM_HTTP_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, HPM_USER_AGENT);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    f.close();

    if (res != CURLE_OK) {
        std::filesystem::remove(dest);
        return Error(ErrorCode::NetworkError,
                     "官方公钥下载失败: " + std::string(curl_easy_strerror(res)));
    }
    // file:// 协议没有 HTTP 状态码，跳过检查
    if (!isFile && httpCode != 200) {
        std::filesystem::remove(dest);
        return Error(ErrorCode::NetworkError,
                     "官方公钥下载失败: HTTP " + std::to_string(httpCode));
    }

    return {};
}

bool OfficialKeyManager::promptFingerprint(const std::string& fingerprint,
                                            bool changed) {
    std::cout << "\n";
    if (changed) {
        std::cout << "\033[33m警告：官方公钥指纹已变化！\033[0m\n";
        std::cout << "这可能是官方轮换了密钥，也可能是中间人攻击。\n";
        std::cout << "请通过官方渠道（官网、GitHub、邮件列表）核对新指纹。\n\n";
    } else {
        std::cout << "首次使用，请验证官方公钥指纹：\n\n";
    }
    std::cout << "  SHA256: " << fingerprint << "\n\n";
    std::cout << "请通过官方渠道核对以上指纹。\n";
    std::cout << "确认无误吗？[y/n]";

    std::string answer;
    std::getline(std::cin, answer);

    if (answer == "y" || answer == "Y" || answer == "yes" || answer == "YES" || answer == "Yes") {
        return true;
    }
    return false;
}

} // namespace hpm::registry