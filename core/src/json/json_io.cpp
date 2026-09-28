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

#include <fstream>
#include <sstream>

namespace hpm::json {

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

Result<Value> parseFile(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return Error(ErrorCode::IoError,
                     "无法打开文件: " + path.string());
    }
    std::ostringstream ss;
    ss << f.rdbuf();
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