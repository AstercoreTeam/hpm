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

// dev/src/commands/publish_cmd.cpp
#include "publish_cmd.h"

#include <hpm/json/json_io.h>
#include <hpm/manifest/chip_loader.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/registry/signature.h>
#include <hpm/util/process.h>

#include <filesystem>
#include <fstream>
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

// 读文件全部内容
std::string readAll(const fs::path& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// 检测私钥算法
std::string detectAlgorithm(const std::string& keyPath) {
    // 方法 1：看文件头
    std::string content = readAll(keyPath);

    if (content.find("BEGIN ED25519 PRIVATE KEY") != std::string::npos) {
        return "ed25519";
    }
    if (content.find("BEGIN EC PRIVATE KEY") != std::string::npos) {
        return "ec";
    }
    if (content.find("BEGIN RSA PRIVATE KEY") != std::string::npos) {
        return "rsa";
    }

    // PKCS#8 格式（BEGIN PRIVATE KEY），需要 openssl 检测
    if (content.find("BEGIN PRIVATE KEY") != std::string::npos) {
        auto result = hpm::util::runProcess("openssl",
            {"pkey", "-in", keyPath, "-noout", "-text"}, true);
        // verbose=true 时输出到 stderr，拿不到
        // 改用临时文件
    }

    return "";
}

// 用 openssl 检测 PKCS#8 私钥的算法
std::string detectAlgorithmViaOpenssl(const std::string& keyPath) {
    // 临时文件捕获输出
    auto tmp = fs::temp_directory_path() / "hpm_keytype.txt";

    std::string cmd = "openssl pkey -in \"" + keyPath +
                      "\" -noout -text > \"" + tmp.string() + "\" 2>&1";
    std::system(cmd.c_str());

    std::string content = readAll(tmp);
    fs::remove(tmp);

    if (content.find("ED25519") != std::string::npos) return "ed25519";
    if (content.find("ED448") != std::string::npos) return "ed448";
    if (content.find("ASN1 OID: prime256v1") != std::string::npos) return "ec";
    if (content.find("ASN1 OID: secp384r1") != std::string::npos) return "ec";
    if (content.find("ASN1 OID: secp521r1") != std::string::npos) return "ec";
    if (content.find("Modulus") != std::string::npos) return "rsa";
    if (content.find("RSA") != std::string::npos) return "rsa";

    return "";
}

} // namespace

void PublishCmd::setup(CLI::App& app) {
    app.add_option("path", pkgDir_, "驱动包目录")->required();
    app.add_option("-k,--key", keyPath_, "签名私钥路径（PEM 格式）")->required();
    app.add_option("-o,--out", outDir_, "输出目录（默认 ./publish）");
    app.add_option("-a,--algorithm", algorithm_,
        "签名算法（auto / rsa / ec / ed25519）")
        ->check(CLI::IsMember({"auto", "rsa", "ec", "ed25519"}));
}

int PublishCmd::run(DevContext& ctx) {
    (void)ctx;

    fs::path pkgDir = fs::absolute(pkgDir_);
    if (!fs::exists(pkgDir)) {
        err("目录不存在: " + pkgDir.string());
        return 1;
    }

    // 1. 读 manifest.json 或 chip.json
    std::string name;
    std::string version;

    auto manifestPath = pkgDir / "manifest.json";
    auto chipPath = pkgDir / "chip.json";

    if (fs::exists(manifestPath)) {
        auto dm = hpm::manifest::loadDriverManifestFile(manifestPath);
        if (!dm) {
            err("manifest.json: " + dm.error().message());
            return 1;
        }
        name = dm->name;
        version = dm->version.toString();
    } else if (fs::exists(chipPath)) {
        auto chip = hpm::manifest::loadChipFile(chipPath);
        if (!chip) {
            err("chip.json: " + chip.error().message());
            return 1;
        }
        name = chip->name;
        auto dirName = pkgDir.filename().string();
        auto dash = dirName.rfind('-');
        if (dash == std::string::npos) {
            err("无法从目录名解析版本: " + dirName);
            return 1;
        }
        version = dirName.substr(dash + 1);
    } else {
        err("缺少 manifest.json 或 chip.json");
        return 1;
    }

    ok("包: " + name + "@" + version);

    // 2. 检查私钥
    if (!fs::exists(keyPath_)) {
        err("私钥不存在: " + keyPath_);
        return 1;
    }

    // 3. 创建输出目录
    fs::path out = outDir_.empty() ? fs::path("publish") : fs::path(outDir_);
    std::error_code ec;
    fs::create_directories(out, ec);

    // 4. 检查包目录名
    auto expectedDirName = name + "-" + version;
    if (pkgDir.filename().string() != expectedDirName) {
        err("包目录名必须是 " + expectedDirName +
            "，实际是 " + pkgDir.filename().string());
        return 1;
    }

    // 5. 打包 tar.gz
    auto tarPath = out / (name + "-" + version + ".tar.gz");
    auto st = hpm::util::runProcess("tar",
        {"-czf", tarPath.string(),
         "-C", pkgDir.parent_path().string(),
         pkgDir.filename().string()},
        false);
    if (!st.ok()) {
        err("打包失败: " + st.error().message());
        return 1;
    }
    ok("打包: " + tarPath.string());

    // 6. 检测算法
    std::string alg = algorithm_;
    if (alg == "auto") {
        alg = detectAlgorithm(keyPath_);
        if (alg.empty()) {
            alg = detectAlgorithmViaOpenssl(keyPath_);
        }
        if (alg.empty()) {
            err("无法自动检测私钥算法，请用 --algorithm 指定");
            err("可用：rsa / ec / ed25519");
            return 1;
        }
    }
    ok("算法: " + alg);

    // 7. 签名
    auto sigPath = out / (name + "-" + version + ".tar.gz.sig");

    if (alg == "ed25519" || alg == "ed448") {
        st = hpm::util::runProcess("openssl",
            {"pkeyutl", "-sign", "-inkey", keyPath_,
             "-rawin", "-in", tarPath.string(), "-out", sigPath.string()},
            false);
    } else {
        // RSA / EC：SHA-256
        st = hpm::util::runProcess("openssl",
            {"dgst", "-sha256", "-sign", keyPath_,
             "-out", sigPath.string(), tarPath.string()},
            false);
    }

    if (!st.ok()) {
        err("签名失败: " + st.error().message());
        return 1;
    }
    ok("签名: " + sigPath.string());

    // 8. 计算 SHA256
    auto sha = hpm::registry::sha256File(tarPath);
    auto size = fs::file_size(tarPath);

    std::cout << "\n";
    std::cout << "包信息（用于 index.json）：\n";
    std::cout << "  name:      " << name << "\n";
    std::cout << "  version:   " << version << "\n";
    std::cout << "  algorithm: " << alg << "\n";
    std::cout << "  sha256:    " << sha << "\n";
    std::cout << "  size:      " << size << "\n";
    std::cout << "\n";
    std::cout << "下一步：\n";
    std::cout << "  1. 把 " << tarPath.filename().string() << " 和 .sig 上传到仓库\n";
    std::cout << "  2. 在 index.json 里加包信息\n";
    std::cout << "  3. 重新签 index.json\n";

    return 0;
}

} // namespace hpm::cli