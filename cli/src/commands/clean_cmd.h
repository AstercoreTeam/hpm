// cli/src/commands/clean_cmd.h
#pragma once

#include "../command.h"

namespace hpm::cli {

class CleanCmd : public Command {
public:
    const char* name() const override { return "clean"; }
    const char* description() const override { return "删除 build/"; }

    void setup(CLI::App& app) override;
    int run(CmdContext& ctx) override;
};

} // namespace hpm::cli