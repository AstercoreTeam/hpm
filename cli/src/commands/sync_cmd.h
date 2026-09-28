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

// cli/src/commands/sync_cmd.h
#pragma once

#include "../command.h"

namespace hpm::cli {

class SyncCmd : public Command {
public:
    const char* name() const override { return "sync"; }
    const char* description() const override { return "拉取驱动、解析依赖、生成 driver/"; }

    void setup(CLI::App& app) override;
    int run(CmdContext& ctx) override;

private:
    bool dryRun_ = false;
};

} // namespace hpm::cli