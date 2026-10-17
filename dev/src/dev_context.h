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

// dev/src/dev_context.h
#pragma once

#include <filesystem>

namespace hpm::cli {

class DevContext {
public:
    const std::filesystem::path& path() const { return path_; }
    void setPath(std::filesystem::path p) { path_ = std::move(p); }

private:
    std::filesystem::path path_;
};

} // namespace hpm::cli