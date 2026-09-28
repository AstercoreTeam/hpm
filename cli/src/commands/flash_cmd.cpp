// cli/src/commands/flash_cmd.cpp
#include "flash_cmd.h"

#include "../cmd_context.h"
#include "../output.h"

#include <hpm/manifest/chip_loader.h>
#include <hpm/manifest/project_loader.h>
#include <hpm/util/process.h>

#include <filesystem>

namespace hpm::cli {

namespace {

namespace fs = std::filesystem;

// 在 packagesDir 下找芯片驱动的 chip.json
Result<Chip> loadChip(const fs::path& packagesDir, const std::string& mcu) {
    if (!fs::exists(packagesDir)) {
        return Error(ErrorCode::NotFound, "包目录不存在");
    }
    for (const auto& entry : fs::directory_iterator(packagesDir)) {
        if (!entry.is_directory()) continue;
        auto name = entry.path().filename().string();
        if (name.rfind(mcu + "-", 0) != 0) continue;
        auto chipJson = entry.path() / "chip.json";
        if (fs::exists(chipJson)) return manifest::loadChipFile(chipJson);
    }
    return Error(ErrorCode::NotFound, "未找到芯片驱动: " + mcu);
}

// 替换 {bin}/{elf}/{hex} 占位符
std::string subst(const std::string& tmpl, const fs::path& buildDir) {
    std::string s = tmpl;
    auto replaceAll = [&](const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.size(), to);
            pos += to.size();
        }
    };
    replaceAll("{bin}", (buildDir / "firmware.bin").string());
    replaceAll("{elf}", (buildDir / "firmware.elf").string());
    replaceAll("{hex}", (buildDir / "firmware.hex").string());
    return s;
}

} // namespace

void FlashCmd::setup(CLI::App& app) {
    (void)app;
}

int FlashCmd::run(CmdContext& ctx) {
    namespace fs = std::filesystem;

    auto proj = manifest::loadProjectFile(ctx.projectFile());
    if (!proj) {
        Output::error(proj.error().message());
        return 1;
    }

    auto chip = loadChip(ctx.packagesDir(), proj->mcu);
    if (!chip) {
        Output::error(chip.error().message());
        return 1;
    }

    if (chip->flash.tool.empty()) {
        Output::error("芯片驱动未声明烧录工具");
        return 1;
    }

    Output::step("烧录固件");
    Output::detail("工具: " + chip->flash.tool);

    std::vector<std::string> args;
    for (const auto& a : chip->flash.args) {
        args.push_back(subst(a, ctx.buildDir()));
    }

    auto st = util::runProcess(chip->flash.tool, args, ctx.verbose());
    if (!st.ok()) {
        Output::error("烧录失败: " + st.error().message());
        return 1;
    }

    Output::success("烧录完成");
    return 0;
}

} // namespace hpm::cli