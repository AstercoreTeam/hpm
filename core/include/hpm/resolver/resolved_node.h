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

// core/include/hpm/resolver/resolved_node.h
#pragma once

#include <hpm/model/version.h>
#include <hpm/types.h>

#include <map>
#include <memory>
#include <string>

namespace hpm {

// 解析阶段内部的树节点
// 表示「一个驱动实例」以及它的子驱动（中间设备）
struct ResolvedNode {
    DriverName  driver;
    Version     version;
    std::string instanceName;    // 顶层用户设备：设备名；中间设备：生成名

    // 直接绑定到这个节点的引脚（key -> pin name）
    std::map<ResourceKey, std::string> pins;
    std::map<std::string, std::string> config;

    // 子节点：key -> 子节点
    std::map<ResourceKey, std::unique_ptr<ResolvedNode>> children;

    // 是否是顶层用户设备
    bool isTopLevel = false;
};

} // namespace hpm