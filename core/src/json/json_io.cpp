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

// core/src/json/json_io.cpp
#include <hpm/json/json_io.h>
#include "json_impl.h"
#include <nlohmann/json.hpp>

#include <toml++/toml.hpp> 

#include <filesystem>
#include <fstream>
#include <sstream>

namespace hpm::json {

// ---------- 内部辅助：TOML 文本 -> JSON 文本 ----------
namespace {

Result<std::string> tomlToJsonText(const std::string& tomlText) {
    try {
        auto tbl = toml::parse(tomlText);
        std::ostringstream ss;
        ss << toml::json_formatter{tbl};
        return ss.str();
    } catch (const toml::parse_error& e) {
        return Error(ErrorCode::ManifestParseError,
                     std::string("TOML 解析失败: ") + e.what());
    }
}

} // namespace

Result<Value> parse(const std::string& text) {
    try {
        auto j = nlohmann::json::parse(text);
        Value v;
        v.impl().data = std::move(j);
        return v;
    } catch (const nlohmann::json::parse_error& e) {
        return Error(ErrorCode::ManifestParseError,
                     std::string("JSON 解析失败: ") + e.what());
    }
}

// 读文件：同目录存在同名 .toml 时优先读 TOML 并转成 JSON，否则读原 JSON 文件
Result<Value> parseFile(const std::filesystem::path& path) {
    auto tomlPath = path;
    tomlPath.replace_extension(".toml");

    std::error_code ec;
    bool hasToml = std::filesystem::exists(tomlPath, ec) && !ec;

    std::filesystem::path actual = hasToml ? tomlPath : path;

    std::ifstream f(actual, std::ios::binary);
    if (!f) {
        return Error(ErrorCode::IoError,
                     "无法打开文件: " + actual.string());
    }
    std::ostringstream ss;
    ss << f.rdbuf();

    if (hasToml) {
        auto jsonText = tomlToJsonText(ss.str());
        if (!jsonText.ok()) return jsonText.error();
        auto parsed = parse(jsonText.value());
        if (!parsed.ok()) return parsed;

        // 回写同名 .json，保持 json 与 toml 同步
        auto writeSt = writeFile(*parsed, path, 2);
        if (!writeSt.ok()) {
            // 回写失败不阻断解析，只提示（可选：返回 error）
            // 这里选择不阻断，避免只读目录/权限问题导致 sync 失败
        }
        return parsed;
    }

    return parse(ss.str());
}

std::string dump(const Value& v, int indent) {
    return v.impl().data.dump(indent);
}

Result<void> writeFile(const Value& v,
                       const std::filesystem::path& path,
                       int indent) {
    std::error_code ec;
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return Error(ErrorCode::IoError,
                         "创建目录失败: " + parent.string() + " - " + ec.message());
        }
    }

    // 原子写：先写临时文件，再重命名
    auto tmp = path;
    tmp += ".tmp";

    {
        std::ofstream f(tmp, std::ios::binary);
        if (!f) {
            return Error(ErrorCode::IoError,
                         "无法写入文件: " + tmp.string());
        }
        f << v.impl().data.dump(indent);
        f << '\n';
    }

    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp);
        return Error(ErrorCode::IoError,
                     "重命名失败: " + path.string() + " - " + ec.message());
    }
    return {};
}

} // namespace hpm::json