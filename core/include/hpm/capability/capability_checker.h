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

// core/include/hpm/capability/capability_checker.h
#pragma once

#include <hpm/capability/check_report.h>
#include <hpm/capability/driver_library.h>
#include <hpm/model/chip.h>
#include <hpm/model/driver_state.h>
#include <hpm/model/project.h>

namespace hpm {

// 能力检查总调度
// 主入口接受 DriverState（已展开的中间设备）
// 另提供一个从 Project 构造临时 state 的重载（兼容旧调用）
class CapabilityChecker {
public:
    CapabilityChecker(const DriverLibrary& lib, const Chip& chip)
        : lib_(lib), chip_(chip) {}

    // 主入口：检查整个状态
    CheckReport check(const DriverState& state) const;

    // 兼容：把 Project 的 devices 直接当 userDevices 检查
    // 注意：不会展开中间设备，所以引用中间设备的 requires 会被当作引脚
    // 建议先用 resolver 展开，再调上面的 check(state)
    CheckReport check(const Project& project) const;

private:
    // 检查一个用户设备
    void checkDevice(const DriverState& state,
                     const Device& device,
                     class PinAllocator& allocator,
                     CheckReport& report) const;

    // 检查一个中间设备
    void checkIntermediate(const DriverState& state,
                           const IntermediateDevice& device,
                           class PinAllocator& allocator,
                           CheckReport& report) const;

    // 名字唯一性
    void checkNames(const DriverState& state, CheckReport& report) const;

    const DriverLibrary& lib_;
    const Chip&          chip_;
};

} // namespace hpm