// cli/src/commands/build_cmd.h
#pragma once

#include "../command.h"

namespace hpm::cli {

class BuildCmd : public Command {
public:
    const char* name() const override { return "build"; }
    const char* description() const override { return "编译固件"; }

    void setup(CLI::App& app) override;
    int run(CmdContext& ctx) override;
};

} // namespace hpm::cli