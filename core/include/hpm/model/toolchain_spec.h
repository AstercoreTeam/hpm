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
// 例：{ name: "arm-none-eabi-gcc", version: "12.2.0", url: "..." }
struct ToolchainSpec {
    std::string        name;        // "arm-none-eabi-gcc"
    VersionRange       version;     // "12.2.0" 或 "^12.0.0"
    std::string        url;         // 可选：下载地址模板
};

} // namespace hpm