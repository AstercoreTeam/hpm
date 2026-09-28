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

// core/include/hpm/model/provided_method.h
#pragma once

#include <hpm/types.h>

#include <string>

namespace hpm {

// 驱动 provides 里的一个方法
// 签名暂时用字符串（后续可解析成参数列表做严格检查）
struct ProvidedMethod {
    MethodName name;      // "transfer" / "read_card"
    std::string signature; // 可选："int(uint8_t*, uint8_t*, size_t)"
};

} // namespace hpm