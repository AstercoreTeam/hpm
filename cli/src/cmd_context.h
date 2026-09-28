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

// cli/src/cmd_context.h
#pragma once

#include <filesystem>
#include <string>

namespace hpm::cli {

// 命令上下文：项目路径、缓存路径等
class CmdContext {
public:
    CmdContext()
        : projectDir_(std::filesystem::current_path()),
          cacheDir_(defaultCacheDir()) {}

    // 项目目录（含 project.json）
    const std::filesystem::path& projectDir() const { return projectDir_; }
    void setProjectDir(std::filesystem::path p) { projectDir_ = std::move(p); }

    // 全局缓存目录
    const std::filesystem::path& cacheDir() const { return cacheDir_; }
    void setCacheDir(std::filesystem::path p) { cacheDir_ = std::move(p); }

    // 常用路径
    std::filesystem::path projectFile() const { return projectDir_ / "project.json"; }
    std::filesystem::path driverStateFile() const { return projectDir_ / "driver.json"; }
    std::filesystem::path driverDir() const { return projectDir_ / "driver"; }
    std::filesystem::path buildDir() const { return projectDir_ / "build"; }
    std::filesystem::path packagesDir() const { return cacheDir_ / "packages"; }

    // 详细输出
    bool verbose() const { return verbose_; }
    void setVerbose(bool v) { verbose_ = v; }

private:
    static std::filesystem::path defaultCacheDir();

    std::filesystem::path projectDir_;
    std::filesystem::path cacheDir_;
    bool verbose_ = false;
};

} // namespace hpm::cli