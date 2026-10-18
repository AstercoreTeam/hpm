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

struct Device {
    DeviceName name;
    DriverName driver;
    Version    version;

    std::map<ResourceKey, std::string> bindings;

    // 用户配置：key -> 值（如 usb_vid、usb_pid）
    std::map<std::string, std::string> config;

    bool userDeclared = true;
};

} // namespace hpm