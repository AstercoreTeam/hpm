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

// core/src/resolver/resolver.cpp
#include <hpm/resolver/resolver.h>

#include <hpm/resolver/id_generator.h>

#include <algorithm>

namespace hpm {

namespace {

bool samePins(const std::map<ResourceKey, std::string>& a,
              const std::map<ResourceKey, std::string>& b) {
    if (a.size() != b.size()) return false;
    for (const auto& [k, v] : a) {
        auto it = b.find(k);
        if (it == b.end() || it->second != v) return false;
    }
    return true;
}

} // namespace

// ─────────────────────────────────────────────
// 公共入口
// ─────────────────────────────────────────────

Result<DriverState> Resolver::resolve() {
    DriverState out;

    for (const auto& dev : project_.devices) {
        // 阶段 1：展开
        auto node = expandUserDevice(dev);
        if (!node) return node.error();

        // 阶段 2：转成 DriverState
        auto st = buildState(**node, out);
        if (!st.ok()) return st.error();
    }

    return out;
}

// ─────────────────────────────────────────────
// 阶段 1：展开成 ResolvedNode 树
// ─────────────────────────────────────────────

Result<std::unique_ptr<ResolvedNode>>
Resolver::expandUserDevice(const Device& dev) {
    const DriverManifest* dm = lib_.find(dev.driver);
    if (!dm) {
        return Error(ErrorCode::NotFound,
                     "找不到驱动: " + dev.driver + "（设备 " + dev.name + "）");
    }

    auto node = std::make_unique<ResolvedNode>();
    node->driver = dev.driver;
    node->version = lockVersion(dev.driver);
    node->instanceName = dev.name;
    node->isTopLevel = true;
    node->config = dev.config;

    std::vector<std::string> visited = {dev.driver};
    std::set<std::string>    consumed;   // 每个用户设备独立

    auto st = expandRequirements(*dm, dev, dev.name, visited, consumed, *node);
    if (!st.ok()) return st.error();

    // 顶层设备未被消费的字段直接作为引脚保留
    for (const auto& [k, v] : dev.bindings) {
        if (consumed.count(k)) continue;
        node->pins[k] = v;
    }

    return node;
}

Status Resolver::expandRequirements(const DriverManifest& dm,
                                    const Device& topDevice,
                                    const std::string& topName,
                                    std::vector<std::string>& visited,
                                    std::set<std::string>& consumed,
                                    ResolvedNode& node) {
    for (const auto& req : dm.requirements) {
        const DriverManifest* childDm = lib_.find(req.type);

        if (childDm) {
            // ── 子驱动：创建子节点 ──
            if (std::find(visited.begin(), visited.end(), req.type) != visited.end()) {
                return Error(ErrorCode::CyclicDependency,
                             "驱动循环依赖: " + topName + " → " + req.type);
            }

            visited.push_back(req.type);
            auto child = expandChild(*childDm, topDevice, topName,
                                     req.key, visited, consumed);
            visited.pop_back();

            if (!child) return child.error();
            node.children[req.key] = std::move(*child);

        } else {
            // ── 直接引脚 ──
            auto it = topDevice.bindings.find(req.key);
            if (it == topDevice.bindings.end()) {
                if (req.optional) continue;
                return Error(ErrorCode::MissingField,
                             "设备 " + topName + " 缺少必填字段 " + req.key +
                                 "（驱动 " + dm.name + " 需要）");
            }
            if (consumed.count(req.key)) {
                return Error(ErrorCode::NameConflict,
                             "设备 " + topName + " 的字段 " + req.key +
                                 " 被多个驱动同时需要");
            }
            consumed.insert(req.key);
            node.pins[req.key] = it->second;
        }
    }
    return {};
}

Result<std::unique_ptr<ResolvedNode>>
Resolver::expandChild(const DriverManifest& childDm,
                      const Device& topDevice,
                      const std::string& topName,
                      const std::string& parentKey,
                      std::vector<std::string>& visited,
                      std::set<std::string>& consumed) {
    auto child = std::make_unique<ResolvedNode>();
    child->driver = childDm.name;
    child->version = lockVersion(childDm.name);
    child->isTopLevel = false;

    // 子驱动的 requires 从 topDevice 里按 key 取
    for (const auto& subReq : childDm.requirements) {
        const DriverManifest* subChildDm = lib_.find(subReq.type);

        if (subChildDm) {
            // 子驱动的子驱动：递归
            if (std::find(visited.begin(), visited.end(), subReq.type) != visited.end()) {
                return Error(ErrorCode::CyclicDependency,
                             "驱动循环依赖: " + topName + " → " + subReq.type);
            }
            visited.push_back(subReq.type);
            auto sub = expandChild(*subChildDm, topDevice, topName,
                                   subReq.key, visited, consumed);
            visited.pop_back();
            if (!sub) return sub.error();
            child->children[subReq.key] = std::move(*sub);
        } else {
            // 引脚
            auto it = topDevice.bindings.find(subReq.key);
            if (it == topDevice.bindings.end()) {
                if (subReq.optional) continue;
                return Error(ErrorCode::MissingField,
                             "设备 " + topName + " 缺少引脚 " + subReq.key +
                                 "（被 " + childDm.name + " 需要，由 " +
                                 parentKey + " 引入）");
            }
            if (consumed.count(subReq.key)) {
                return Error(ErrorCode::NameConflict,
                             "设备 " + topName + " 的字段 " + subReq.key +
                                 " 被多个驱动同时需要");
            }
            consumed.insert(subReq.key);
            child->pins[subReq.key] = it->second;
        }
    }

    // ── 复用或新建 ──

    // 1. 本次解析中已创建的
    if (const std::string* name = findIntermediateName(childDm.name, child->pins)) {
        child->instanceName = *name;
        return child;
    }

    // 2. existing_（driver.json）里已有的
    const IntermediateDevice* reuse =
        findExistingIntermediate(childDm.name, child->pins);
    if (reuse) {
        child->instanceName = reuse->name;
        rememberIntermediate(childDm.name, child->pins, reuse->name);
        return child;
    }

    // 3. 全新生成
    child->instanceName = generateIntermediateId(childDm.name);
    rememberIntermediate(childDm.name, child->pins, child->instanceName);

    return child;
}

// ─────────────────────────────────────────────
// 阶段 2：把树转成 DriverState
// ─────────────────────────────────────────────

Status Resolver::buildState(const ResolvedNode& root, DriverState& out) {
    // 根节点 = 用户设备
    Device userDev;
    userDev.name = root.instanceName;
    userDev.driver = root.driver;
    userDev.version = root.version;
    userDev.userDeclared = true;
    userDev.config = root.config;

    // 直接引脚
    for (const auto& [k, v] : root.pins) {
        userDev.bindings[k] = v;
    }

    // 子节点的引用（bus -> 子节点名）
    for (const auto& [key, childPtr] : root.children) {
        userDev.bindings[key] = childPtr->instanceName;
    }

    out.userDevices.push_back(std::move(userDev));

    // 递归写中间设备
    for (const auto& [key, childPtr] : root.children) {
        auto st = buildIntermediate(*childPtr, out);
        if (!st.ok()) return st.error();
    }

    return {};
}

Status Resolver::buildIntermediate(const ResolvedNode& node,
                                   DriverState& out) {
    IntermediateDevice idev;
    idev.id = node.instanceName;
    idev.name = node.instanceName;
    idev.driver = node.driver;
    idev.version = node.version;
    idev.config = node.config;

    // 引脚
    for (const auto& [k, v] : node.pins) {
        idev.bindings[k] = v;
    }

    // 子引用
    for (const auto& [key, childPtr] : node.children) {
        idev.bindings[key] = childPtr->instanceName;
    }

    // 去重：如果已有同名中间设备，跳过
    bool exists = false;
    for (const auto& e : out.intermediateDevices) {
        if (e.name == idev.name) { exists = true; break; }
    }
    if (!exists) {
        out.intermediateDevices.push_back(std::move(idev));
    }

    // 递归子设备
    for (const auto& [key, childPtr] : node.children) {
        auto st = buildIntermediate(*childPtr, out);
        if (!st.ok()) return st.error();
    }

    return {};
}

// ─────────────────────────────────────────────
// 工具
// ─────────────────────────────────────────────

const std::string* Resolver::findIntermediateName(
    const std::string& driver,
    const std::map<ResourceKey, std::string>& pins) {
    IntermediateKey key{driver, pins};
    auto it = created_.find(key);
    return it == created_.end() ? nullptr : &it->second;
}

void Resolver::rememberIntermediate(
    const std::string& driver,
    const std::map<ResourceKey, std::string>& pins,
    const std::string& name) {
    created_[IntermediateKey{driver, pins}] = name;
}

const IntermediateDevice* Resolver::findExistingIntermediate(
    const std::string& driver,
    const std::map<ResourceKey, std::string>& pins) const {
    if (!existing_) return nullptr;
    for (const auto& idev : existing_->intermediateDevices) {
        if (idev.driver != driver) continue;
        if (samePins(idev.bindings, pins)) return &idev;
    }
    return nullptr;
}

Version Resolver::lockVersion(const std::string& driver) const {
    if (existing_) {
        for (const auto& d : existing_->userDevices) {
            if (d.driver == driver) return d.version;
        }
        for (const auto& d : existing_->intermediateDevices) {
            if (d.driver == driver) return d.version;
        }
    }
    if (auto* dm = lib_.find(driver)) {
        return dm->version;
    }
    return Version{};
}

} // namespace hpm