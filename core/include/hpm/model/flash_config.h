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