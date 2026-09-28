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