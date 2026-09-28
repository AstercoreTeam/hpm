// cli/src/command_registry.cpp
#include "command_registry.h"

#include "commands/build_cmd.h"
#include "commands/clean_cmd.h"
#include "commands/dev_export_cmd.h"
#include "commands/flash_cmd.h"
#include "commands/init_cmd.h"
#include "commands/sync_cmd.h"

namespace hpm::cli {

CommandRegistry::CommandRegistry() {
    commands_.push_back(std::make_unique<InitCmd>());
    commands_.push_back(std::make_unique<SyncCmd>());
    commands_.push_back(std::make_unique<BuildCmd>());
    commands_.push_back(std::make_unique<FlashCmd>());
    commands_.push_back(std::make_unique<CleanCmd>());
    commands_.push_back(std::make_unique<DevExportCmd>());
}

void CommandRegistry::registerAll(CLI::App& app) {
    for (auto& cmd : commands_) {
        auto* sub = app.add_subcommand(cmd->name(), cmd->description());
        cmd->setup(*sub);
    }
}

int CommandRegistry::dispatch(const std::string& name, CmdContext& ctx) {
    for (auto& cmd : commands_) {
        if (name == cmd->name()) {
            return cmd->run(ctx);
        }
    }
    return -1;
}

std::vector<std::string> CommandRegistry::names() const {
    std::vector<std::string> out;
    for (const auto& c : commands_) out.push_back(c->name());
    return out;
}

} // namespace hpm::cli