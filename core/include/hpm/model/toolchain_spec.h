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

// core/include/hpm/model/toolchain_spec.h
#pragma once

#include <hpm/model/version_range.h>

#include <string>

namespace hpm {

// 工具链描述
// 芯片驱动可以显式声明 cc / objcopy / size / prefix
// 如果只给 name（如 "arm-none-eabi-gcc"），工具会推导 prefix（旧行为，保留兼容）
struct ToolchainSpec {
    std::string  name;       // "arm-none-eabi-gcc"（兼容字段）
    std::string  prefix;     // "arm-none-eabi-"（显式前缀，优先）
    std::string  cc;         // "arm-none-eabi-gcc"（显式命令，最优先）
    std::string  objcopy;    // "arm-none-eabi-objcopy"
    std::string  size;       // "arm-none-eabi-size"
    VersionRange version;
    std::string  url;

    // 解析出实际使用的命令名
    std::string resolveCc() const {
        if (!cc.empty()) return cc;
        if (!prefix.empty()) return prefix + "gcc";
        return name.empty() ? "gcc" : name;
    }

    std::string resolveObjcopy() const {
        if (!objcopy.empty()) return objcopy;
        if (!prefix.empty()) return prefix + "objcopy";
        // 从 name 推导
        if (!name.empty()) {
            auto pos = name.find("gcc");
            if (pos != std::string::npos && pos > 0) {
                return name.substr(0, pos) + "objcopy";
            }
        }
        return "objcopy";
    }

    std::string resolveSize() const {
        if (!size.empty()) return size;
        if (!prefix.empty()) return prefix + "size";
        if (!name.empty()) {
            auto pos = name.find("gcc");
            if (pos != std::string::npos && pos > 0) {
                return name.substr(0, pos) + "size";
            }
        }
        return "size";
    }
};

} // namespace hpm