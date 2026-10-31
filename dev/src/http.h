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

#pragma once

#include <string>

namespace hpm::cli {

struct HttpResult {
    int status = 0;      // 0 表示连接层失败
    std::string body;
};

// 发 POST JSON；authHeader 形如 "Authorization: Bearer xxx"，为空则不加
HttpResult httpPostJson(const std::string& url,
                        const std::string& body,
                        const std::string& authHeader = "");

} // namespace hpm::cli