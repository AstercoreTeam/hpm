// dev/src/commands/init_cmd.h
#pragma once

#include "../dev_context.h"

#include <CLI/CLI.hpp>

namespace hpm::cli {

class InitCmd {
public:
    void setup(CLI::App& app);
    int run(DevContext& ctx);

private:
    bool force_ = false;
};

} // namespace hpm::cli