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

// core/include/hpm/types.h
#pragma once

#include <string>
#include <vector>

namespace hpm {

// 设备名：用户起的，如 "door_reader"
using DeviceName = std::string;

// 驱动名：如 "rc522"、"spi"
using DriverName = std::string;

// 引脚名：如 "PA4"
using PinName = std::string;

// 资源 key：驱动 requires 里的 key，如 "bus"、"cs"
using ResourceKey = std::string;

// 资源类型：如 "spi"、"gpio_out"
using ResourceType = std::string;

// 方法名：如 "transfer"、"read_card"
using MethodName = std::string;

} // namespace hpm