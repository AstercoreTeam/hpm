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

#include <hpm/capability/pin_allocator.h>
#include <hpm/capability/pin_matcher.h>

#include <set>

namespace hpm {

// ─────────────────────────────────────────────
// 便捷重载：Project → 临时 DriverState
// ─────────────────────────────────────────────

CheckReport CapabilityChecker::check(const Project& project) const {
    DriverState state;
    state.userDevices = project.devices;
    return check(state);
}

// ─────────────────────────────────────────────
// 主入口
// ─────────────────────────────────────────────

CheckReport CapabilityChecker::check(const DriverState& state) const {
    CheckReport report;

    checkNames(state, report);

    PinAllocator allocator(chip_);

    for (const auto& d : state.userDevices) {
        checkDevice(state, d, allocator, report);
    }
    for (const auto& d : state.intermediateDevices) {
        checkIntermediate(state, d, allocator, report);
    }

    return report;
}

// ─────────────────────────────────────────────
// 名字唯一性
// ─────────────────────────────────────────────

void CapabilityChecker::checkNames(const DriverState& state,
                                   CheckReport& report) const {
    std::set<std::string> seen;

    auto checkName = [&](const std::string& name, const std::string& location) {
        if (seen.count(name)) {
            report.addError(ErrorCode::NameConflict,
                            "设备名重复: " + name, location);
        }
        seen.insert(name);
    };

    for (const auto& d : state.userDevices) {
        checkName(d.name, "devices." + d.name);
    }
    for (const auto& d : state.intermediateDevices) {
        checkName(d.name, "devices." + d.name);
    }
}

// ─────────────────────────────────────────────
// 用户设备
// ─────────────────────────────────────────────

void CapabilityChecker::checkDevice(const DriverState& state,
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

    PinMatcher pinMatcher(chip_);

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
        const std::string location = "devices." + device.name + "." + req.key;

        // 1) 引用了另一个用户设备
        if (const Device* target = state.findUserDevice(value)) {
            const DriverManifest* tdm = lib_.find(target->driver);
            if (!tdm) continue;
            for (const auto& m : req.methods) {
                if (!tdm->providesMethod(m)) {
                    report.addError(ErrorCode::MethodNotFound,
                                    "驱动 " + target->driver +
                                        "（设备 " + target->name +
                                        "）未提供方法 " + m,
                                    location);
                }
            }
            continue;
        }

        // 2) 引用了中间设备
        if (const IntermediateDevice* target = state.findIntermediate(value)) {
            const DriverManifest* tdm = lib_.find(target->driver);
            if (!tdm) continue;
            for (const auto& m : req.methods) {
                if (!tdm->providesMethod(m)) {
                    report.addError(ErrorCode::MethodNotFound,
                                    "驱动 " + target->driver +
                                        "（中间设备 " + target->name +
                                        "）未提供方法 " + m,
                                    location);
                }
            }
            continue;
        }

        // 3) 否则当作引脚
        pinMatcher.check(value, req, device.name, report);
        allocator.allocate(value, device.name, req.key, report);
    }
}

// ─────────────────────────────────────────────
// 中间设备
// ─────────────────────────────────────────────

void CapabilityChecker::checkIntermediate(const DriverState& state,
                                          const IntermediateDevice& device,
                                          PinAllocator& allocator,
                                          CheckReport& report) const {
    const DriverManifest* dm = lib_.find(device.driver);
    if (!dm) {
        report.addError(ErrorCode::NotFound,
                        "找不到驱动: " + device.driver +
                            "（中间设备 " + device.name + "）",
                        "devices." + device.name);
        return;
    }

    PinMatcher pinMatcher(chip_);

    for (const auto& req : dm->requirements) {
        auto it = device.bindings.find(req.key);

        if (it == device.bindings.end()) {
            if (!req.optional) {
                report.addError(ErrorCode::MissingField,
                                "中间设备 " + device.name +
                                    " 缺少必填资源: " + req.key,
                                "devices." + device.name);
            }
            continue;
        }

        const std::string& value = it->second;
        const std::string location = "devices." + device.name + "." + req.key;

        // 中间设备通常直接绑引脚，也可能引用别的中间设备
        if (const IntermediateDevice* target = state.findIntermediate(value)) {
            const DriverManifest* tdm = lib_.find(target->driver);
            if (!tdm) continue;
            for (const auto& m : req.methods) {
                if (!tdm->providesMethod(m)) {
                    report.addError(ErrorCode::MethodNotFound,
                                    "驱动 " + target->driver +
                                        "（中间设备 " + target->name +
                                        "）未提供方法 " + m,
                                    location);
                }
            }
            continue;
        }

        // 引脚
        pinMatcher.check(value, req, device.name, report);
        allocator.allocate(value, device.name, req.key, report);
    }
}

} // namespace hpm