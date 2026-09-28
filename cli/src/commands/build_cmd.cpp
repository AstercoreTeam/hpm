// cli/src/commands/build_cmd.cpp
#include "build_cmd.h"

#include "../cmd_context.h"
#include "../output.h"

#include <hpm/util/process.h>

#include <filesystem>

namespace hpm::cli {

void BuildCmd::setup(CLI::App& app) {
    (void)app;
}

int BuildCmd::run(CmdContext& ctx) {
    namespace fs = std::filesystem;

    auto driverDir = ctx.driverDir();
    if (!fs::exists(driverDir)) {
        Output::error("driver/ 不存在，先运行 hpm sync");
        return 1;
    }

    Output::step("编译");
    auto st = util::runProcess("make",
                               {"-C", driverDir.string()},
                               ctx.verbose());
    if (!st.ok()) {
        Output::error("编译失败: " + st.error().message());
        return 1;
    }

    Output::success("编译完成");
    Output::detail("产物: " + (ctx.buildDir() / "firmware.elf").string());
    return 0;
}

} // namespace hpm::cli