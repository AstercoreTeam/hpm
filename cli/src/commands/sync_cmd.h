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