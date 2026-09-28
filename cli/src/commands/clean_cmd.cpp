// cli/src/commands/clean_cmd.cpp
#include "clean_cmd.h"

#include "../cmd_context.h"
#include "../output.h"

#include <filesystem>

namespace hpm::cli {

void CleanCmd::setup(CLI::App& app) { (void)app; }

int CleanCmd::run(CmdContext& ctx) {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto n = fs::remove_all(ctx.buildDir(), ec);
    if (ec) {
        Output::error("删除失败: " + ec.message());
        return 1;
    }
    Output::success("已删除 build/（" + std::to_string(n) + " 项）");
    return 0;
}

} // namespace hpm::cli