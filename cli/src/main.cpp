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

// cli/src/main.cpp
#include "cmd_context.h"
#include "command_registry.h"
#include "output.h"

#include <CLI/CLI.hpp>
#include <hpm/hpm.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"hpm - Hardware Package Manager"};
    app.set_version_flag("--version", hpm::version());
    app.require_subcommand(0, 1);   // 允许不带子命令

    // 全局选项
    std::string projectDir;
    std::string cacheDir;
    bool verbose = false;
    app.add_option("-C,--project-dir", projectDir, "项目目录（默认当前目录）");
    app.add_option("--cache-dir", cacheDir, "缓存目录（默认 ~/.hpm）");
    app.add_flag("-v,--verbose", verbose, "详细输出");

    hpm::cli::CommandRegistry registry;
    registry.registerAll(app);

    CLI11_PARSE(app, argc, argv);

    hpm::cli::CmdContext ctx;
    if (!projectDir.empty()) ctx.setProjectDir(projectDir);
    if (!cacheDir.empty()) ctx.setCacheDir(cacheDir);
    ctx.setVerbose(verbose);

    hpm::cli::Output::setColor(true);

    // 无子命令：打印帮助
    if (app.get_subcommands().empty()) {
        std::cout << app.help() << std::endl;
        return 0;
    }

    // 找到被调用的子命令
    for (auto* sub : app.get_subcommands()) {
        if (sub->count() > 0) {
            int rc = registry.dispatch(sub->get_name(), ctx);
            return rc < 0 ? 1 : rc;
        }
    }

    std::cout << app.help() << std::endl;
    return 0;
}