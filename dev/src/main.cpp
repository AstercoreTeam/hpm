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
// dev/src/main.cpp
#include "dev_context.h"
#include "commands/init_cmd.h"
#include "commands/publish_cmd.h"
#include "commands/validate_cmd.h"

#include <CLI/CLI.hpp>
#include <hpm/hpm.h>

#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"hpm-dev - HPM 驱动开发工具"};
    app.set_version_flag("--version", hpm::version());
    app.require_subcommand(0, 1);

    // init
    hpm::cli::InitCmd initCmd;
    auto* initSub = app.add_subcommand("init", "生成驱动包骨架");
    initCmd.setup(*initSub);

    // validate
    std::string path;
    auto* validateCmd = app.add_subcommand("validate", "校验驱动包格式");
    validateCmd->add_option("path", path, "驱动包目录")->required();

    // publish
    hpm::cli::PublishCmd publishCmd;
    auto* publishSub = app.add_subcommand("publish", "打包 + 签名");
    publishCmd.setup(*publishSub);

    CLI11_PARSE(app, argc, argv);

    hpm::cli::DevContext ctx;
    ctx.setPath(path);

    if (*initSub) {
        return initCmd.run(ctx);
    }
    if (*validateCmd) {
        return hpm::cli::runValidate(ctx);
    }
    if (*publishSub) {
        return publishCmd.run(ctx);
    }

    std::cout << app.help() << std::endl;
    return 0;
}