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

// core/include/hpm/generator/template_engine.h
#pragma once

#include <map>
#include <string>

namespace hpm {

// 简单模板替换：把 "{{key}}" 替换为 value
// 未找到的 key 保留原样（便于排查）
std::string renderTemplate(
    const std::string& tmpl,
    const std::map<std::string, std::string>& vars);

} // namespace hpm