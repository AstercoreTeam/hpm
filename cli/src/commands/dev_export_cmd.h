// cli/src/commands/dev_export_cmd.h
#pragma once

#include "../command.h"

namespace hpm::cli {

class DevExportCmd : public Command {
public:
    const char* name() const override { return "dev"; }
    const char* description() const override { return "生成 hpm.h（底层 API）"; }

    void setup(CLI::App& app) override;
    int run(CmdContext& ctx) override;

private:
    bool export_ = false;
};

} // namespace hpm::cli