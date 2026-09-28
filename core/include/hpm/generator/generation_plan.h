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

// core/include/hpm/generator/generation_plan.h
#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace hpm {

// 一个要生成的文件
struct PlannedFile {
    std::filesystem::path   relativePath;   // 相对 driver/ 的路径
    std::string             content;        // 文件内容
    bool                    overwrite = true;  // 是否覆盖已有
};

// 生成计划：所有要写的文件
struct GenerationPlan {
    std::vector<PlannedFile> files;

    void addFile(std::filesystem::path rel, std::string content) {
        files.push_back({std::move(rel), std::move(content), true});
    }
};

} // namespace hpm