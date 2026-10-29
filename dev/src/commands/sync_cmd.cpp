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

// dev/src/commands/sync_cmd.cpp
#include "sync_cmd.h"

#include "../dev_config.h"

#include <hpm/config.h>
#include <hpm/json/json_io.h>
#include <hpm/registry/signature.h>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <zip.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hpm::cli {

namespace fs = std::filesystem;

namespace {

void ok(const std::string& msg) {
    std::cout << "\033[32mok\033[0m " << msg << "\n";
}

void err(const std::string& msg) {
    std::cerr << "\033[31merror\033[0m " << msg << "\n";
}

void info(const std::string& msg) {
    std::cout << "\033[34minfo\033[0m " << msg << "\n";
}

void detail(const std::string& msg) {
    std::cout << "    " << msg << "\n";
}

// 检查 token 是否过期
bool isTokenExpired(const std::string& expiresAt) {
    std::tm tm = {};
    std::istringstream ss(expiresAt);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) return true;

    auto expiry = std::mktime(&tm);
    auto now = std::time(nullptr);
    return now >= expiry;
}

// 解析 zip 文件名
std::pair<std::string, std::string> parseZipName(const std::string& fileName) {
    std::string base = fileName;
    if (base.size() > 4 && base.substr(base.size() - 4) == ".zip") {
        base = base.substr(0, base.size() - 4);
    }
    auto dash = base.rfind('-');
    if (dash == std::string::npos) {
        return {"", ""};
    }
    return {base.substr(0, dash), base.substr(dash + 1)};
}

// 读 zip 里的 chip.json / manifest.json
struct PackageMeta {
    std::string kind;
    std::string requiresJson;
    std::string license;
};

Result<PackageMeta> readPackageMeta(const fs::path& zipPath) {
    int errCode = 0;
    zip_t* zip = zip_open(zipPath.string().c_str(), ZIP_RDONLY, &errCode);
    if (!zip) {
        return Error(ErrorCode::IoError, "无法打开 zip: " + zipPath.string());
    }

    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
    for (zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(numEntries); i++) {
        const char* name = zip_get_name(zip, i, 0);
        if (!name) continue;

        std::string n = name;
        auto slash = n.find_last_of('/');
        if (slash != std::string::npos) {
            n = n.substr(slash + 1);
        }

        if (n != "chip.json" && n != "manifest.json") continue;

        zip_file_t* f = zip_fopen_index(zip, i, 0);
        if (!f) continue;

        std::string content;
        char buf[4096];
        zip_int64_t bytesRead;
        while ((bytesRead = zip_fread(f, buf, sizeof(buf))) > 0) {
            content.append(buf, bytesRead);
        }
        zip_fclose(f);
        zip_close(zip);

        auto json = json::parse(content);
        if (!json) return json.error();

        PackageMeta meta;
        meta.license = json->getString("license", "MIT");
        if (n == "chip.json") {
            meta.kind = "chip";
            meta.requiresJson = "{}";
        } else {
            meta.kind = "driver";
            auto reqs = json->get("requires");
            if (reqs) {
                meta.requiresJson = json::dump(*reqs, -1);
            } else {
                meta.requiresJson = "{}";
            }
        }
        return meta;
    }

    zip_close(zip);
    return Error(ErrorCode::NotFound, "zip 里没有 chip.json 或 manifest.json");
}

// 用 httplib::Client 发 POST JSON
struct HttpResult {
    int status = 0;
    std::string body;
};

HttpResult httpPostJson(const std::string& url,
                         const std::string& body,
                         const std::string& authHeader) {
    HttpResult result;

    std::string s = url;
    bool isHttps = true;
    if (s.rfind("https://", 0) == 0) {
        s = s.substr(8);
    } else if (s.rfind("http://", 0) == 0) {
        s = s.substr(7);
        isHttps = false;
    }

    std::string host;
    std::string path;
    auto slash = s.find('/');
    if (slash == std::string::npos) {
        host = s;
        path = "/";
    } else {
        host = s.substr(0, slash);
        path = s.substr(slash);
    }

    httplib::Client cli(isHttps ? ("https://" + host) : ("http://" + host));
    cli.set_connection_timeout(30);
    cli.set_read_timeout(30);

    httplib::Headers headers;
    headers.emplace("Content-Type", "application/json");
    if (!authHeader.empty()) {
        auto colon = authHeader.find(':');
        if (colon != std::string::npos) {
            std::string key = authHeader.substr(0, colon);
            std::string value = authHeader.substr(colon + 1);
            while (!value.empty() && value.front() == ' ') value.erase(value.begin());
            headers.emplace(key, value);
        }
    }

    auto res = cli.Post(path.c_str(), headers, body, "application/json");
    if (!res) {
        result.status = 0;
        result.body = "HTTP 请求失败: " + httplib::to_string(res.error());
        return result;
    }

    result.status = res->status;
    result.body = res->body;
    return result;
}

} // namespace

void SyncCmd::setup(CLI::App& app) {
    app.add_option("zip", zipFile_, "zip 文件路径")->required();
}

int SyncCmd::run(DevContext& ctx) {
    (void)ctx;

    // 1. 读 dev.json
    auto cfg = loadDevConfig();
    if (!cfg) {
        err("dev.json 不存在，请先运行 hpm-dev login");
        return 1;
    }

    std::cout << "==> 读取 dev.json\n";
    detail("developer: " + cfg->developer);
    detail("repoUrl: " + cfg->repoUrl);

    // 2. 检查 token
    std::cout << "==> 检查 token\n";
    if (isTokenExpired(cfg->expiresAt)) {
        err("AccessToken 已过期，请重新运行 hpm-dev login");
        return 1;
    }
    ok("未过期");

    // 3. 解析 zip
    fs::path zipPath = zipFile_;
    if (!fs::exists(zipPath)) {
        err("zip 文件不存在: " + zipPath.string());
        return 1;
    }

    auto fileName = zipPath.filename().string();
    auto [name, version] = parseZipName(fileName);
    if (name.empty() || version.empty()) {
        err("zip 文件名不符合 <name>-<version>.zip");
        return 1;
    }

    std::cout << "==> 解析 zip: " + fileName + "\n";
    detail("name: " + name);
    detail("version: " + version);

    // 4. 读元数据
    auto meta = readPackageMeta(zipPath);
    if (!meta) {
        err(meta.error().message());
        return 1;
    }
    detail("kind: " + meta->kind);
    detail("license: " + meta->license);

    // 5. 算 sha256 / size
    auto sha256 = hpm::registry::sha256File(zipPath);
    auto size = fs::file_size(zipPath);
    detail("size: " + std::to_string(size));
    detail("sha256: " + sha256);

    // 6. 拼 URL
    std::string fileNameOnly = name + "-" + version + ".zip";
    std::string url = cfg->repoUrl + "/" + cfg->developer + "/" + fileNameOnly;
    std::string sigUrl = url + ".sig";

    // 7. 调 API
    std::cout << "==> 更新索引\n";

    auto payload = json::Value::object();
    payload.set("name", json::Value(name));
    payload.set("kind", json::Value(meta->kind));
    payload.set("requiresJson", json::Value(meta->requiresJson));
    payload.set("version", json::Value(version));
    payload.set("url", json::Value(url));
    payload.set("sigUrl", json::Value(sigUrl));
    payload.set("sha256", json::Value(sha256));
    payload.set("size", json::Value(static_cast<int64_t>(size)));
    payload.set("license", json::Value(meta->license));

    auto body = json::Value::object();
    body.set("accessToken", json::Value(cfg->accessToken));
    body.set("action", json::Value("update_package"));
    body.set("payload", payload);

    auto resp = httpPostJson(HPM_DEV_CENTER_API,
                              json::dump(body, -1),
                              "Authorization: Bearer " + std::string(HPM_DEV_CENTER_APP_KEY));

    if (resp.status == 401) {
        err("AccessToken 过期，请重新登录！ hpm-dev login");
        return 1;
    }
    if (resp.status != 200) {
        err("更新索引失败: HTTP " + std::to_string(resp.status) +
            " " + resp.body);
        return 1;
    }

    auto respJson = json::parse(resp.body);
    if (respJson && respJson->getBool("success", false)) {
        ok(name + "@" + version + " 索引已更新");
    } else {
        err("更新失败: " + resp.body);
        return 1;
    }

    // 8. TODO：上传 zip
    info("请手动上传 zip 和 .sig 到:");
    detail(url);
    detail(sigUrl);

    return 0;
}

} // namespace hpm::cli