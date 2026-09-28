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

    // 子节点：key -> 子节点
    std::map<ResourceKey, std::unique_ptr<ResolvedNode>> children;

    // 是否是顶层用户设备
    bool isTopLevel = false;
};

} // namespace hpm