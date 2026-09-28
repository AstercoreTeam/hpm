// core/include/hpm/model/driver_manifest.h
#pragma once

#include <hpm/model/provided_method.h>
#include <hpm/model/requirement.h>
#include <hpm/model/version.h>
#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// 驱动清单：描述一个驱动包「需要什么」和「提供什么」
// 注意：不能用 requires 做成员名（C++20 关键字），用 requirements
struct DriverManifest {
    DriverName name;                          // "rc522"
    Version version;                          // 1.2.3
    std::string description;

    std::vector<Requirement>    requirements; // 需求列表
    std::vector<ProvidedMethod> provides;     // 提供的方法

    // 便捷查询：是否提供某个方法
    bool providesMethod(const MethodName& m) const {
        for (const auto& p : provides) {
            if (p.name == m) return true;
        }
        return false;
    }

    // 便捷查询：找某个 key 的需求
    const Requirement* findRequirement(const ResourceKey& key) const {
        for (const auto& r : requirements) {
            if (r.key == key) return &r;
        }
        return nullptr;
    }
};

} // namespace hpm