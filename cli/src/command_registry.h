/*
 * Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// cli/src/command_registry.h
#pragma once

#include "command.h"

#include <CLI/CLI.hpp>

#include <memory>
#include <vector>

namespace hpm::cli {

class CmdContext;

// 注册所有命令
class CommandRegistry {
public:
    CommandRegistry();

    // 把所有命令注册到 app
    void registerAll(CLI::App& app);

    // 找到并执行命令
    // 返回 -1 表示未找到
    int dispatch(const std::string& name, CmdContext& ctx);

    // 列出所有命令名
    std::vector<std::string> names() const;

private:
    std::vector<std::unique_ptr<Command>> commands_;
};

} // namespace hpm::cli