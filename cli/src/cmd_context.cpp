// cli/src/cmd_context.cpp
#include "cmd_context.h"

#include <cstdlib>

namespace hpm::cli {

std::filesystem::path CmdContext::defaultCacheDir() {
    // 优先 $HPM_HOME
    if (const char* env = std::getenv("HPM_HOME"); env && *env) {
        return std::filesystem::path(env);
    }
    // 再试 $HOME/.hpm
    if (const char* home = std::getenv("HOME"); home && *home) {
        return std::filesystem::path(home) / ".hpm";
    }
    // Windows: %USERPROFILE%
    if (const char* up = std::getenv("USERPROFILE"); up && *up) {
        return std::filesystem::path(up) / ".hpm";
    }
    // 兜底
    return std::filesystem::current_path() / ".hpm";
}

} // namespace hpm::cli