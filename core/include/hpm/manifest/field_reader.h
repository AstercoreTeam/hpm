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

// core/include/hpm/manifest/field_reader.h
#pragma once

#include <hpm/json/json_value.h>
#include <hpm/result.h>

#include <cstdint>
#include <string>
#include <vector>

namespace hpm::manifest {

// 字段读取辅助：集中在 JSON 对象上按 key 取值并校验类型
// 缺字段 / 类型不符时返回带字段名的错误，方便定位
class FieldReader {
public:
    explicit FieldReader(const json::Value& obj, std::string context = {})
        : obj_(obj), context_(std::move(context)) {}

    // 必填字段
    Result<std::string> requireString(const std::string& key) const;
    Result<int64_t>     requireInt(const std::string& key) const;
    Result<bool>        requireBool(const std::string& key) const;
    Result<json::Value> requireObject(const std::string& key) const;
    Result<json::Value> requireArray(const std::string& key) const;

    // 可选字段（缺失返回默认值）
    std::string optString(const std::string& key,
                          const std::string& def = "") const;
    int64_t     optInt(const std::string& key, int64_t def = 0) const;
    bool        optBool(const std::string& key, bool def = false) const;

    // 可选对象：缺失返回 null
    Result<json::Value> optObject(const std::string& key) const;
    // 可选数组：缺失返回空数组
    Result<std::vector<json::Value>> optArray(const std::string& key) const;

    // 存在性
    bool has(const std::string& key) const { return obj_.has(key); }

    // 遍历对象：返回所有 key
    Result<std::vector<std::string>> keys() const { return obj_.keys(); }

    // 底层对象
    const json::Value& raw() const { return obj_; }

private:
    std::string errorPrefix(const std::string& key) const;

    const json::Value& obj_;
    std::string context_;
};

} // namespace hpm::manifest