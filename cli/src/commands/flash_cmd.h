// cli/src/commands/flash_cmd.h
#pragma once

#include "../command.h"

namespace hpm::cli {

class FlashCmd : public Command {
public:
    const char* name() const override { return "flash"; }
    const char* description() const override { return "烧录固件"; }

    void setup(CLI::App& app) override;
    int run(CmdContext& ctx) override;
};

} // namespace hpm::cli