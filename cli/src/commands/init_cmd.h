// cli/src/commands/init_cmd.h
#pragma once

#include "../command.h"

namespace hpm::cli {

class InitCmd : public Command {
public:
    const char* name() const override { return "init"; }
    const char* description() const override { return "初始化项目"; }

    void setup(CLI::App& app) override;
    int run(CmdContext& ctx) override;

private:
    std::string mcu_;
    bool force_ = false;
};

} // namespace hpm::cli