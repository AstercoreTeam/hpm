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

// core/include/hpm/model/intermediate_device.h
#pragma once

#include <hpm/model/version.h>
#include <hpm/types.h>

#include <map>
#include <string>

namespace hpm {

// 中间虚拟设备：工具根据驱动依赖自动创建
// 例：rc522 要 spi，用户只写了 "bus": "spi1"，
//     工具创建一个 __spi_a3f2 设备承载 spi1 的引脚
//
// 对用户默认不可见，可通过 hpm dev --export 暴露
struct IntermediateDevice {
    DeviceName id;                     // "__spi_a3f2"（工具生成的随机 ID）
    DeviceName name;                   // 用户可改名，默认等于 id
    DriverName driver;                 // "spi"
    Version    version;

    // 资源绑定（同 Device）
    std::map<ResourceKey, std::string> bindings;
};

} // namespace hpm