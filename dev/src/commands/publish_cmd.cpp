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
#include <iostream>

namespace hpm::cli {

namespace fs = std::filesystem;

namespace {

void ok(const std::string& msg) {
    std::cout << "\033[32mok\033[0m " << msg << "\n";
}

void err(const std::string& msg) {
    std::cerr << "\033[31merror\033[0m " << msg << "\n";
}

} // namespace

void PublishCmd::setup(CLI::App& app) {
    app.add_option("path", pkgDir_, "驱动包目录")->required();
    app.add_option("-k,--key", keyPath_, "签名私钥路径（PEM 格式）")->required();
    app.add_option("-o,--out", outDir_, "输出目录（默认 ./publish）");
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
        // chip.json 没有 version 字段，从目录名取
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

    // 4. 打包 tar.gz
    auto tarPath = out / (name + "-" + version + ".tar.gz");

    // 检查包目录名是不是 <name>-<version>
    auto expectedDirName = name + "-" + version;
    if (pkgDir.filename().string() != expectedDirName) {
        err("包目录名必须是 " + expectedDirName + "，实际是 " + pkgDir.filename().string());
        return 1;
    }

    // tar -czf <tar> -C <pkgDir.parent> <pkgDir.filename>
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

    // 5. 签名
    auto sigPath = out / (name + "-" + version + ".tar.gz.sig");
    st = hpm::util::runProcess("openssl",
        {"dgst", "-sha256",
         "-sign", keyPath_,
         "-out", sigPath.string(),
         tarPath.string()},
        false);
    if (!st.ok()) {
        err("签名失败: " + st.error().message());
        return 1;
    }
    ok("签名: " + sigPath.string());

    // 6. 计算 SHA256
    auto sha = hpm::registry::sha256File(tarPath);
    auto size = fs::file_size(tarPath);

    std::cout << "\n";
    std::cout << "包信息（用于 index.json）：\n";
    std::cout << "  name:    " << name << "\n";
    std::cout << "  version: " << version << "\n";
    std::cout << "  sha256:  " << sha << "\n";
    std::cout << "  size:    " << size << "\n";
    std::cout << "\n";

    return 0;
}

} // namespace hpm::cli