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

// cli/src/cmd_context.cpp
#include "cmd_context.h"
#include "hpm/config.h"
#include <cstdlib>

namespace hpm::cli {

std::filesystem::path CmdContext::defaultCacheDir() {
    // 优先 $HPM_HOME
    if (const char* env = std::getenv(HPM_ENV_HOME); env && *env) {
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