// cli/src/output.h
#pragma once

#include <string>

namespace hpm::cli {

// 输出辅助：带颜色、前缀
class Output {
public:
    static void info(const std::string& msg);
    static void success(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    static void step(const std::string& msg);   // 阶段标题
    static void detail(const std::string& msg); // 缩进细节

    // 是否用彩色（非 tty 时关闭）
    static void setColor(bool on);
};

} // namespace hpm::cli