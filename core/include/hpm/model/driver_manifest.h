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

// core/include/hpm/model/driver_manifest.h
#pragma once

#include <hpm/model/provided_method.h>
#include <hpm/model/requirement.h>
#include <hpm/model/version.h>
#include <hpm/types.h>

#include <map>
#include <string>
#include <vector>

namespace hpm {

// 驱动配置字段
struct ConfigField {
    std::string type;         // "int" / "string"
    std::string defaultValue;
    int         length = 0;   // 字符串长度（type == "string" 时）
};

// 驱动清单
struct DriverManifest {
    DriverName name;
    Version version;
    std::string description;
    std::string license;

    // 配置 schema
    std::vector<std::string> toolchain;
    std::map<std::string, ConfigField> config;

    std::vector<Requirement>    requirements;
    std::vector<ProvidedMethod> provides;

    bool providesMethod(const MethodName& m) const {
        for (const auto& p : provides) {
            if (p.name == m) return true;
        }
        return false;
    }

    const Requirement* findRequirement(const ResourceKey& key) const {
        for (const auto& r : requirements) {
            if (r.key == key) return &r;
        }
        return nullptr;
    }
};

} // namespace hpm