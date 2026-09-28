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

// core/include/hpm/model/project.h
#pragma once

#include <hpm/model/device.h>
#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// 用户项目：project.json 的内存表示
struct Project {
    std::string mcu;                 // "stm32f103c8t6"
    std::vector<Device> devices;     // 用户声明的所有设备

    // 便捷查询：按设备名找设备
    const Device* findDevice(const DeviceName& name) const {
        for (const auto& d : devices) {
            if (d.name == name) return &d;
        }
        return nullptr;
    }

    Device* findDevice(const DeviceName& name) {
        for (auto& d : devices) {
            if (d.name == name) return &d;
        }
        return nullptr;
    }
};

} // namespace hpm