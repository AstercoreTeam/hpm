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

// cli/src/commands/init_cmd.cpp
#include "init_cmd.h"

#include "../cmd_context.h"
#include "../output.h"

#include <hpm/json/json_io.h>
#include <hpm/util/fs_utils.h>

namespace hpm::cli {

void InitCmd::setup(CLI::App& app) {
    app.add_option("--mcu", mcu_, "指定 MCU 型号");
    app.add_flag("--force", force_, "覆盖已有 project.json");
}

int InitCmd::run(CmdContext& ctx) {
    namespace fs = std::filesystem;

    auto projFile = ctx.projectFile();
    if (fs::exists(projFile) && !force_) {
        Output::error("project.json 已存在（用 --force 覆盖）");
        return 1;
    }

    // 构造模板
    auto root = json::Value::object();
    root.set("mcu", json::Value(mcu_.empty() ? "stm32f103c8t6" : mcu_));

    auto devices = json::Value::object();
    root.set("devices", devices);

    auto st = json::writeFile(root, projFile);
    if (!st.ok()) {
        Output::error(st.error().message());
        return 1;
    }
    Output::success("已创建 " + projFile.string());

    // .gitignore
    auto gitignore = ctx.projectDir() / ".gitignore";
    if (!fs::exists(gitignore)) {
        std::string content =
            "build/\n"
            "driver/\n"
            ".hpm/\n"
            "*.o\n"
            "*.elf\n"
            "*.bin\n"
            "*.hex\n";
        auto s2 = util::writeFileAtomic(gitignore, content);
        if (s2.ok()) {
            Output::success("已创建 .gitignore");
        }
    }

    return 0;
}

} // namespace hpm::cli