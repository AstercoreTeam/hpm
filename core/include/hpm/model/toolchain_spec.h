// core/include/hpm/model/toolchain_spec.h
#pragma once

#include <hpm/model/version_range.h>

#include <string>

namespace hpm {

// 工具链描述
// 例：{ name: "arm-none-eabi-gcc", version: "12.2.0", url: "..." }
struct ToolchainSpec {
    std::string        name;        // "arm-none-eabi-gcc"
    VersionRange       version;     // "12.2.0" 或 "^12.0.0"
    std::string        url;         // 可选：下载地址模板
};

} // namespace hpm