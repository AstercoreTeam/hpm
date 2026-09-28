// core/include/hpm/model/flash_config.h
#pragma once

#include <string>
#include <vector>

namespace hpm {

// 烧录配置
struct FlashConfig {
    std::string tool;                     // "st-flash" / "openocd" / "esptool"
    std::vector<std::string> args;        // 参数模板，支持 {bin} {elf} {hex} 占位符
    std::string baseAddress;              // "0x8000000"
};

} // namespace hpm