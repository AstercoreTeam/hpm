// core/src/util/process.cpp
#include <hpm/util/process.h>

#include <array>
#include <cstdio>
#include <sstream>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace hpm::util {

namespace {

// 简单的 shell 转义
std::string shellEscape(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    out += "'";
    return out;
}

} // namespace

Status runProcess(const std::string& program,
                  const std::vector<std::string>& args,
                  bool verbose) {
    std::ostringstream cmd;
    cmd << shellEscape(program);
    for (const auto& a : args) {
        cmd << " " << shellEscape(a);
    }
    if (verbose) {
        cmd << " 2>&1";     // 把 stderr 合并到 stdout
    }

    FILE* pipe = POPEN(cmd.str().c_str(), "r");
    if (!pipe) {
        return Error(ErrorCode::IoError, "无法启动进程: " + program);
    }

    std::string output;
    std::array<char, 4096> buf{};
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
        output += buf.data();
        if (verbose) {
            std::fputs(buf.data(), stdout);
        }
    }

    int rc = PCLOSE(pipe);
    if (rc != 0) {
        if (!verbose) {
            return Error(ErrorCode::BuildFailed,
                         program + " 退出码 " + std::to_string(rc) +
                             "\n输出:\n" + output);
        }
        return Error(ErrorCode::BuildFailed,
                     program + " 退出码 " + std::to_string(rc));
    }
    return {};
}

} // namespace hpm::util