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

// cli/src/commands/sync_cmd.cpp
#include "sync_cmd.h"

#include "../cmd_context.h"
#include "../output.h"

#include <hpm/capability/capability_checker.h>
#include <hpm/capability/driver_library.h>
#include <hpm/generator/generator.h>
#include <hpm/json/json_io.h>
#include <hpm/manifest/chip_loader.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/manifest/project_loader.h>
#include <hpm/manifest/state_loader.h>
#include <hpm/registry/registry_client.h>
#include <hpm/resolver/resolver.h>
#include <hpm/util/progress_bar.h>
#include "hpm/config.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <queue>
#include <set>

namespace hpm::cli {

namespace {

namespace fs = std::filesystem;

Result<Chip> loadChipFromCache(const fs::path& packagesDir,
                                const std::string& mcu) {
    if (!fs::exists(packagesDir)) {
        return Error(ErrorCode::NotFound,
                     "包目录不存在: " + packagesDir.string());
    }
    for (const auto& entry : fs::directory_iterator(packagesDir)) {
        if (!entry.is_directory()) continue;
        auto name = entry.path().filename().string();
        if (name.rfind(mcu + "-", 0) != 0) continue;
        auto chipJson = entry.path() / HPM_FILE_CHIP;
        if (fs::exists(chipJson)) {
            return manifest::loadChipFile(chipJson);
        }
    }
    return Error(ErrorCode::NotFound,
                 "未找到芯片驱动: " + mcu + "（在 " + packagesDir.string() + " 下）");
}

DriverLibrary loadAllDrivers(const fs::path& packagesDir) {
    DriverLibrary lib;
    if (!fs::exists(packagesDir)) return lib;

    for (const auto& entry : fs::directory_iterator(packagesDir)) {
        if (!entry.is_directory()) continue;
        auto mf = entry.path() / HPM_FILE_MANIFEST;
        if (!fs::exists(mf)) continue;
        auto dm = manifest::loadDriverManifestFile(mf);
        if (dm.ok()) {
            lib.add(std::move(*dm));
        }
    }
    return lib;
}

struct NeededPackage {
    std::string name;
    std::string version;
};

Result<std::vector<NeededPackage>> expandDependencies(
    const registry::Index& idx,
    const std::string& mcu,
    const Project& proj) {

    std::vector<NeededPackage> needed;
    std::set<std::string> visited;
    std::queue<std::string> queue;

    queue.push(mcu);

    for (const auto& dev : proj.devices) {
        if (dev.version.toString() != "0.0.0") {
            queue.push(dev.driver + "@" + dev.version.toString());
        } else {
            queue.push(dev.driver);
        }
    }

    while (!queue.empty()) {
        auto item = queue.front();
        queue.pop();

        if (visited.count(item)) continue;
        visited.insert(item);

        std::string name = item;
        std::string ver;
        auto at = item.find('@');
        if (at != std::string::npos) {
            name = item.substr(0, at);
            ver = item.substr(at + 1);
        }
        if (ver.empty()) {
            ver = idx.findLatestVersion(name);
        }
        if (ver.empty()) {
            return Error(ErrorCode::NotFound,
                         "索引里找不到包: " + name);
        }

        auto* pv = idx.find(name, ver);
        if (!pv) {
            return Error(ErrorCode::NotFound,
                         "索引里找不到包: " + name + "@" + ver);
        }

        needed.push_back({name, ver});

        for (const auto& [key, type] : pv->requirements) {
            if (idx.packages.count(type)) {
                queue.push(type);
            }
        }
    }

    return needed;
}

} // namespace

void SyncCmd::setup(CLI::App& app) {
    app.add_flag("--dry-run", dryRun_, "只解析，不写文件");
    app.add_flag("--offline", offline_, "离线模式，只用本地缓存");
    app.add_flag("--skip-signature", skipSignature_, "跳过签名验证（仅开发）");
}

int SyncCmd::run(CmdContext& ctx) {
    // 1. 读 project.json
    Output::step("读取 project.json");
    auto proj = manifest::loadProjectFile(ctx.projectFile());
    if (!proj) {
        Output::error(proj.error().message());
        return 1;
    }
    Output::detail("MCU: " + proj->mcu);
    Output::detail("设备数: " + std::to_string(proj->devices.size()));

    // 2. 读已有 driver.json
    std::optional<DriverState> existing;
    if (fs::exists(ctx.driverStateFile())) {
        auto st = manifest::loadDriverStateFile(ctx.driverStateFile());
        if (st.ok()) {
            existing = std::move(*st);
            Output::detail("已加载 driver.json（用于复用 ID 和版本）");
        }
    }

    // 3. 确保依赖包在本地
    Output::step("确保依赖包在本地");

    registry::RegistryConfig cfg = registry::RegistryConfig::load();
    if (offline_) cfg.offline = true;
    if (skipSignature_) cfg.skipSignature = true;
    cfg.cacheDir = ctx.cacheDir();

    registry::RegistryClient client(cfg);

    // 3a. 拉索引
    std::optional<registry::Index> idx;

    {
        auto idxBar = util::ProgressBar(HPM_FILE_INDEX, 0);
        auto idxResult = client.fetchIndex([&](uint64_t downloaded, uint64_t total) {
            idxBar.update(downloaded, total);
        });
        if (!idxResult) {
            idxBar.fail();
            Output::error(idxResult.error().message());
            return 1;
        }
        idxBar.done();
        idx = std::move(*idxResult);
        Output::detail("索引已加载（" + std::to_string(idx->packages.size()) + " 个包）");
    }

    // 3b. 展开依赖
    auto needed = expandDependencies(*idx, proj->mcu, *proj);
    if (!needed) {
        Output::error(needed.error().message());
        return 1;
    }
    Output::detail("需要 " + std::to_string(needed->size()) + " 个包");

    // 3c. 下载
    for (const auto& pkg : *needed) {
        auto bar = util::ProgressBar(pkg.name + "@" + pkg.version, 0);
        auto pkgPath = client.ensurePackage(pkg.name, pkg.version,
            [&](uint64_t downloaded, uint64_t total) {
                bar.update(downloaded, total);
            });
        if (!pkgPath) {
            bar.fail();
            Output::error(pkgPath.error().message());
            return 1;
        }
        bar.done();
    }
    Output::success("所有依赖包已就绪");

    // 4. 加载芯片驱动
    Output::step("加载芯片驱动: " + proj->mcu);
    auto chip = loadChipFromCache(ctx.packagesDir(), proj->mcu);
    if (!chip) {
        Output::error(chip.error().message());
        return 1;
    }

    // 5. 加载驱动库
    Output::step("加载驱动库");
    auto lib = loadAllDrivers(ctx.packagesDir());
    Output::detail("已加载 " + std::to_string(lib.size()) + " 个驱动");

    // 6. 依赖解析
    Output::step("解析依赖");
    Resolver resolver(lib, *proj, existing ? &*existing : nullptr);
    auto state = resolver.resolve();
    if (!state) {
        Output::error(state.error().message());
        return 1;
    }
    Output::detail("用户设备: " + std::to_string(state->userDevices.size()));
    Output::detail("中间设备: " + std::to_string(state->intermediateDevices.size()));

    // 7. 能力检查
    Output::step("能力检查");
    CapabilityChecker checker(lib, *chip);
    auto report = checker.check(*state);
    if (!report.ok()) {
        Output::error(report.toError().message());
        return 1;
    }
    for (const auto& w : report.warnings()) {
        Output::warn(w.message);
    }
    Output::success("能力检查通过");

    if (dryRun_) {
        Output::info("--dry-run 模式，不写文件");
        return 0;
    }

    // 8. 写 driver.json
    Output::step("写入 driver.json");
    auto stateJson = manifest::saveDriverState(*state);
    auto st = json::writeFile(stateJson, ctx.driverStateFile());
    if (!st.ok()) {
        Output::error(st.error().message());
        return 1;
    }
    Output::success("driver.json 已更新");

    // 9. 生成 driver/
    Output::step("生成 driver/");
    Generator gen(*state, lib, *proj, *chip, ctx.packagesDir(), ctx.driverDir());
    auto genSt = gen.generate();
    if (!genSt.ok()) {
        Output::error(genSt.error().message());
        return 1;
    }
    Output::success("driver/ 已生成");

    Output::info("下一步：hpm build");
    return 0;
}

} // namespace hpm::cli