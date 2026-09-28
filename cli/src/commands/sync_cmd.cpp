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
#include <hpm/resolver/resolver.h>

#include <filesystem>
#include <optional>

namespace hpm::cli {

namespace {

namespace fs = std::filesystem;

// 从本地包目录加载芯片驱动
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

        auto chipJson = entry.path() / "chip.json";
        if (fs::exists(chipJson)) {
            return manifest::loadChipFile(chipJson);
        }
    }
    return Error(ErrorCode::NotFound,
                 "未找到芯片驱动: " + mcu + "（在 " + packagesDir.string() + " 下）");
}

// 扫 packages 目录里所有 manifest.json，全部加载
DriverLibrary loadAllDrivers(const fs::path& packagesDir) {
    DriverLibrary lib;
    if (!fs::exists(packagesDir)) return lib;

    for (const auto& entry : fs::directory_iterator(packagesDir)) {
        if (!entry.is_directory()) continue;
        auto mf = entry.path() / "manifest.json";
        if (!fs::exists(mf)) continue;

        auto dm = manifest::loadDriverManifestFile(mf);
        if (dm.ok()) {
            lib.add(std::move(*dm));
        }
    }
    return lib;
}

} // namespace

void SyncCmd::setup(CLI::App& app) {
    app.add_flag("--dry-run", dryRun_, "只解析，不写文件");
}

int SyncCmd::run(CmdContext& ctx) {
    namespace fs = std::filesystem;

    // 1. 读 project.json
    Output::step("读取 project.json");
    auto proj = manifest::loadProjectFile(ctx.projectFile());
    if (!proj) {
        Output::error(proj.error().message());
        return 1;
    }
    Output::detail("MCU: " + proj->mcu);
    Output::detail("设备数: " + std::to_string(proj->devices.size()));

    // 2. 读已有的 driver.json（可空）
    std::optional<DriverState> existing;
    if (fs::exists(ctx.driverStateFile())) {
        auto st = manifest::loadDriverStateFile(ctx.driverStateFile());
        if (st.ok()) {
            existing = std::move(*st);
            Output::detail("已加载 driver.json（用于复用 ID 和版本）");
        }
    }

    // 3. 加载芯片驱动
    Output::step("加载芯片驱动: " + proj->mcu);
    auto chip = loadChipFromCache(ctx.packagesDir(), proj->mcu);
    if (!chip) {
        Output::error(chip.error().message());
        return 1;
    }

    // 4. 加载驱动库
    Output::step("加载驱动库");
    auto lib = loadAllDrivers(ctx.packagesDir());
    Output::detail("已加载 " + std::to_string(lib.size()) + " 个驱动");

    // 5. 依赖解析
    Output::step("解析依赖");
    Resolver resolver(lib, *proj, existing ? &*existing : nullptr);
    auto state = resolver.resolve();
    if (!state) {
        Output::error(state.error().message());
        return 1;
    }
    Output::detail("用户设备: " + std::to_string(state->userDevices.size()));
    Output::detail("中间设备: " + std::to_string(state->intermediateDevices.size()));

    // 6. 能力检查（在展开后的状态上检查）
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

    // 7. 写 driver.json
    Output::step("写入 driver.json");
    auto stateJson = manifest::saveDriverState(*state);
    auto st = json::writeFile(stateJson, ctx.driverStateFile());
    if (!st.ok()) {
        Output::error(st.error().message());
        return 1;
    }
    Output::success("driver.json 已更新");

    // 8. 生成 driver/
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