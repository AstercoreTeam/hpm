// dev/src/commands/publish_cmd.h
#pragma once

#include "../dev_context.h"

#include <CLI/CLI.hpp>

#include <string>

namespace hpm::cli {

class PublishCmd {
public:
    void setup(CLI::App& app);
    int run(DevContext& ctx);

private:
    std::string pkgDir_;
    std::string keyPath_;
    std::string outDir_;
    std::string algorithm_ = "auto";
};

} // namespace hpm::cli