// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// core/src/manifest/field_reader.cpp
#include <hpm/manifest/field_reader.h>

namespace hpm::manifest {

std::string FieldReader::errorPrefix(const std::string& key) const {
    if (context_.empty()) return key;
    return context_ + "." + key;
}

Result<std::string> FieldReader::requireString(const std::string& key) const {
    auto v = obj_.get(key);
    if (!v) {
        return Error(ErrorCode::MissingField,
                     "缺少字段: " + errorPrefix(key));
    }
    auto s = v->asString();
    if (!s) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为字符串）: " + errorPrefix(key));
    }
    return *s;
}

Result<int64_t> FieldReader::requireInt(const std::string& key) const {
    auto v = obj_.get(key);
    if (!v) {
        return Error(ErrorCode::MissingField,
                     "缺少字段: " + errorPrefix(key));
    }
    auto i = v->asInt();
    if (!i) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为整数）: " + errorPrefix(key));
    }
    return *i;
}

Result<bool> FieldReader::requireBool(const std::string& key) const {
    auto v = obj_.get(key);
    if (!v) {
        return Error(ErrorCode::MissingField,
                     "缺少字段: " + errorPrefix(key));
    }
    auto b = v->asBool();
    if (!b) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为布尔值）: " + errorPrefix(key));
    }
    return *b;
}

Result<json::Value> FieldReader::requireObject(const std::string& key) const {
    auto v = obj_.get(key);
    if (!v) {
        return Error(ErrorCode::MissingField,
                     "缺少字段: " + errorPrefix(key));
    }
    if (!v->isObject()) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为对象）: " + errorPrefix(key));
    }
    return *v;
}

Result<json::Value> FieldReader::requireArray(const std::string& key) const {
    auto v = obj_.get(key);
    if (!v) {
        return Error(ErrorCode::MissingField,
                     "缺少字段: " + errorPrefix(key));
    }
    if (!v->isArray()) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为数组）: " + errorPrefix(key));
    }
    return *v;
}

std::string FieldReader::optString(const std::string& key,
                                   const std::string& def) const {
    return obj_.getString(key, def);
}

int64_t FieldReader::optInt(const std::string& key, int64_t def) const {
    return obj_.getInt(key, def);
}

bool FieldReader::optBool(const std::string& key, bool def) const {
    return obj_.getBool(key, def);
}

Result<json::Value> FieldReader::optObject(const std::string& key) const {
    if (!obj_.has(key)) return json::Value();
    auto v = obj_.get(key);
    if (!v) return v.error();
    if (!v->isObject()) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为对象）: " + errorPrefix(key));
    }
    return *v;
}

Result<std::vector<json::Value>> FieldReader::optArray(const std::string& key) const {
    std::vector<json::Value> out;
    if (!obj_.has(key)) return out;
    auto v = obj_.get(key);
    if (!v) return v.error();
    if (!v->isArray()) {
        return Error(ErrorCode::InvalidField,
                     "字段类型错误（应为数组）: " + errorPrefix(key));
    }
    auto n = v->size();
    if (!n) return n.error();
    out.reserve(*n);
    for (size_t i = 0; i < *n; ++i) {
        auto item = v->at(i);
        if (!item) return item.error();
        out.push_back(*item);
    }
    return out;
}

} // namespace hpm::manifest