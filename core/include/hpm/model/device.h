/*
 * Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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