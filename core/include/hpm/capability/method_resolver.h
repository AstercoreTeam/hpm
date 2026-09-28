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

// core/include/hpm/capability/method_resolver.h
#pragma once

#include <hpm/capability/check_report.h>
#include <hpm/capability/driver_library.h>
#include <hpm/model/project.h>
#include <hpm/types.h>

namespace hpm {

// 检查下层驱动是否提供了上层需要的方法
class MethodResolver {
public:
    MethodResolver(const DriverLibrary& lib) : lib_(lib) {}

    // 检查 targetDeviceName 的 driver 是否提供了 req.methods 里的所有方法
    // targetDeviceName 是下层设备的设备名，用来查它的驱动清单
    bool check(const DeviceName& device,
               const ResourceKey& key,
               const Requirement& req,
               const Device& target,
               CheckReport& report) const;

private:
    const DriverLibrary& lib_;
};

} // namespace hpm