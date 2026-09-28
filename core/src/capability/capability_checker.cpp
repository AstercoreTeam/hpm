// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// core/src/capability/capability_checker.cpp
#include <hpm/capability/capability_checker.h>

#include <hpm/capability/method_resolver.h>
#include <hpm/capability/pin_allocator.h>
#include <hpm/capability/pin_matcher.h>

#include <set>

namespace hpm {

CheckReport CapabilityChecker::check(const Project& project) const {
    CheckReport report;

    // 0. 名字唯一性
    checkNames(project, report);

    // 1. 引脚分配
    PinAllocator allocator(chip_);

    for (const auto& device : project.devices) {
        checkDevice(project, device, allocator, report);
    }

    return report;
}

void CapabilityChecker::checkNames(const Project& project,
                                   CheckReport& report) const {
    std::set<DeviceName> seen;
    for (const auto& d : project.devices) {
        if (seen.count(d.name)) {
            report.addError(ErrorCode::NameConflict,
                            "设备名重复: " + d.name,
                            "devices." + d.name);
        }
        seen.insert(d.name);
    }
}

void CapabilityChecker::checkDevice(const Project& project,
                                    const Device& device,
                                    PinAllocator& allocator,
                                    CheckReport& report) const {
    const DriverManifest* dm = lib_.find(device.driver);
    if (!dm) {
        report.addError(ErrorCode::NotFound,
                        "找不到驱动: " + device.driver +
                            "（设备 " + device.name + "）",
                        "devices." + device.name);
        return;
    }

    MethodResolver methodResolver(lib_);
    PinMatcher     pinMatcher(chip_);

    // 遍历驱动的每个 requires
    for (const auto& req : dm->requirements) {
        auto it = device.bindings.find(req.key);

        if (it == device.bindings.end()) {
            if (!req.optional) {
                report.addError(ErrorCode::MissingField,
                                "设备 " + device.name + " 缺少必填资源: " + req.key +
                                    "（驱动 " + device.driver + " 需要）",
                                "devices." + device.name);
            }
            continue;
        }

        const std::string& value = it->second;

        // value 可能是引脚名（PA4）或设备名（spi1）
        // 先看它是不是本项目里的设备名
        const Device* targetDev = project.findDevice(value);

        if (targetDev) {
            // 引用另一个设备：检查方法能力
            methodResolver.check(device.name, req.key, req, *targetDev, report);
        } else {
            // 当作引脚名处理
            pinMatcher.check(value, req, device.name, report);
            allocator.allocate(value, device.name, req.key, report);
        }
    }
}

} // namespace hpm