// core/include/hpm/util/process.h
#pragma once

#include <hpm/result.h>

#include <string>
#include <vector>

namespace hpm::util {

// 运行外部命令，捕获输出
// - verbose 为 true 时把 stdout/stderr 直接透传到终端
// - 返回非零退出码 → Error
Status runProcess(const std::string& program,
                  const std::vector<std::string>& args,
                  bool verbose = false);

} // namespace hpm::util