// cli/src/command.h
#pragma once

#include <CLI/CLI.hpp>
#include <hpm/result.h>

namespace hpm::cli {

class CmdContext;

// 命令基类
class Command {
public:
    virtual ~Command() = default;

    // 命令名（注册用）
    virtual const char* name() const = 0;

    // 描述
    virtual const char* description() const = 0;

    // 注册命令行选项
    virtual void setup(CLI::App& app) = 0;

    // 执行
    virtual int run(CmdContext& ctx) = 0;
};

} // namespace hpm::cli