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

// core/include/hpm/model/driver_state.h
#pragma once

#include <hpm/model/device.h>
#include <hpm/model/intermediate_device.h>
#include <hpm/model/version.h>
#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// driver.json 的内存表示
// 锁定：中间设备 ID、驱动版本、用户设备列表
struct DriverState {
    // 用户设备（来自 project.json，但版本已锁定）
    std::vector<Device> userDevices;

    // 中间设备（工具生成）
    std::vector<IntermediateDevice> intermediateDevices;

    // 便捷查询
    const IntermediateDevice* findIntermediate(const DeviceName& id) const {
        for (const auto& d : intermediateDevices) {
            if (d.id == id || d.name == id) return &d;
        }
        return nullptr;
    }
};

} // namespace hpm