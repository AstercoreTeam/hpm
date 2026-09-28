// core/include/hpm/model/device.h
#pragma once

#include <hpm/model/version.h>
#include <hpm/types.h>

#include <map>
#include <string>

namespace hpm {

// 一个设备实例：用户在 project.json 里声明的
// 例：
//   "door_reader": {
//       "driver": "rc522",
//       "bus": "spi1",
//       "cs": "PA4",
//       "rst": "PA3"
//   }
struct Device {
    DeviceName name;                       // "door_reader"（用户起的名）
    DriverName driver;                     // "rc522"
    Version    version;                    // 可选，驱动版本

    // 资源 key → 值（引脚名 / 总线名 / 其他）
    // "bus" → "spi1"，"cs" → "PA4"
    std::map<ResourceKey, std::string> bindings;

    // 是否用户直接声明的（true）还是工具生成的中间设备（false）
    bool userDeclared = true;
};

} // namespace hpm