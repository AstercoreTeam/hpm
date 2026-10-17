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

// core/include/hpm/registry/official_key.h
#pragma once

#include <hpm/result.h>

#include <filesystem>

namespace hpm::registry {

// 官方公钥管理
// - 从官方地址下载（硬编码，不从镜像）
// - 每次 hpm sync 检查更新
// - 指纹变化时提示用户验证
class OfficialKeyManager {
public:
    explicit OfficialKeyManager(std::filesystem::path cacheDir);

    // 确保官方公钥存在且最新，返回公钥文件路径
    Result<std::filesystem::path> ensure();

    // 官方公钥地址（硬编码）
    static const char* officialUrl();

    // 本地路径
    std::filesystem::path localPath() const;
    std::filesystem::path fingerprintPath() const;

private:
    Result<void> download(const std::filesystem::path& dest);
    bool promptFingerprint(const std::string& fingerprint, bool changed);

    std::filesystem::path cacheDir_;
};

} // namespace hpm::registry