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

// core/include/hpm/model/requirement.h
#pragma once

#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// 驱动 requires 里的一条需求
// 例：{ key: "bus",  type: "spi",     methods: ["transfer"], optional: false }
//     { key: "cs",   type: "gpio_out", methods: ["write"],    optional: false }
//     { key: "rst",  type: "gpio_out", methods: ["write"],    optional: true  }
struct Requirement {
    ResourceKey key;                          // "bus" / "cs" / "rst"
    ResourceType type;                        // "spi" / "gpio_out"
    std::vector<MethodName> methods;          // 需要下层驱动提供的方法
    bool optional = false;
};

} // namespace hpm