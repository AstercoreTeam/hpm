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

// core/include/hpm/resolver/resolver.h
#pragma once

#include <hpm/capability/driver_library.h>
#include <hpm/model/driver_state.h>
#include <hpm/model/project.h>
#include <hpm/resolver/resolved_node.h>
#include <hpm/result.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace hpm {

// 依赖解析：把 Project 展开成 DriverState
// 两阶段：
//   1. 递归展开成 ResolvedNode 树
//   2. 把树转成 DriverState（user_devices + intermediate_devices）
class Resolver {
public:
    Resolver(const DriverLibrary& lib,
             const Project& project,
             const DriverState* existingState = nullptr)
        : lib_(lib),
          project_(project),
          existing_(existingState) {}

    // 执行解析，返回新的 DriverState
    Result<DriverState> resolve();

private:
    // 本次解析中「已创建的中间设备」的索引键
    // 按 (driver, 引脚集合) 唯一确定一个中间设备
    struct IntermediateKey {
        std::string driver;
        std::map<ResourceKey, std::string> pins;

        bool operator<(const IntermediateKey& o) const {
            if (driver != o.driver) return driver < o.driver;
            return pins < o.pins;
        }
    };

    // ─────────────────────────────────────────
    // 阶段 1：展开成 ResolvedNode 树
    // ─────────────────────────────────────────

    // 展开一个用户设备
    Result<std::unique_ptr<ResolvedNode>>
    expandUserDevice(const Device& dev);

    // 递归展开一个驱动的 requirements，结果写入 node
    Status expandRequirements(const DriverManifest& dm,
                              const Device& topDevice,
                              const std::string& topName,
                              std::vector<std::string>& visitedDrivers,
                              std::set<std::string>& consumedKeys,
                              ResolvedNode& node);

    // 为子驱动创建节点（子驱动的 key 从 topDevice 里取）
    Result<std::unique_ptr<ResolvedNode>>
    expandChild(const DriverManifest& childDm,
                const Device& topDevice,
                const std::string& topName,
                const std::string& parentKey,
                std::vector<std::string>& visitedDrivers,
                std::set<std::string>& consumedKeys);

    // ─────────────────────────────────────────
    // 阶段 2：把树转成 DriverState
    // ─────────────────────────────────────────

    Status buildState(const ResolvedNode& root, DriverState& out);

    Status buildIntermediate(const ResolvedNode& node, DriverState& out);

    // ─────────────────────────────────────────
    // 工具
    // ─────────────────────────────────────────

    // 在「本次解析已创建」中查找匹配的中间设备名
    const std::string* findIntermediateName(
        const std::string& driver,
        const std::map<ResourceKey, std::string>& pins);

    // 记录一个新创建的中间设备
    void rememberIntermediate(
        const std::string& driver,
        const std::map<ResourceKey, std::string>& pins,
        const std::string& name);

    // 在 existing_（driver.json）里按 driver + 引脚集合查找
    const IntermediateDevice* findExistingIntermediate(
        const std::string& driver,
        const std::map<ResourceKey, std::string>& pins) const;

    // 锁定版本：优先沿用已有版本，否则用库里的最新
    Version lockVersion(const std::string& driver) const;

    const DriverLibrary& lib_;
    const Project&       project_;
    const DriverState*   existing_;

    // 本次解析中已创建的中间设备
    std::map<IntermediateKey, std::string> created_;
};

} // namespace hpm