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

// core/src/util/fs_utils.cpp
#include <hpm/util/fs_utils.h>

#include <fstream>
#include <sstream>

namespace hpm::util {

Status writeFileAtomic(const std::filesystem::path& path,
                       const std::string& content) {
    namespace fs = std::filesystem;

    std::error_code ec;
    auto parent = path.parent_path();
    if (!parent.empty()) {
        fs::create_directories(parent, ec);
        if (ec) {
            return Error(ErrorCode::IoError,
                         "创建目录失败: " + parent.string() + " - " + ec.message());
        }
    }

    auto tmp = path;
    tmp += ".tmp";

    {
        std::ofstream f(tmp, std::ios::binary);
        if (!f) {
            return Error(ErrorCode::IoError, "无法写入: " + tmp.string());
        }
        f.write(content.data(), static_cast<std::streamsize>(content.size()));
    }

    fs::rename(tmp, path, ec);
    if (ec) {
        fs::remove(tmp);
        return Error(ErrorCode::IoError,
                     "重命名失败: " + path.string() + " - " + ec.message());
    }
    return {};
}

Result<std::string> readFile(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return Error(ErrorCode::IoError, "无法打开: " + path.string());
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool exists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

} // namespace hpm::util