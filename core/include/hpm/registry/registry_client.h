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

// core/include/hpm/registry/registry_client.h
#pragma once

#include <hpm/registry/index.h>
#include <hpm/result.h>

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace hpm::registry {

// 镜像配置
struct RegistryConfig {
    std::vector<std::string> registries;
    std::filesystem::path    cacheDir;
    bool                     offline = false;
    bool                     skipSignature = false;

    static RegistryConfig load();
};

using DownloadProgress = std::function<void(uint64_t downloaded, uint64_t total)>;

class RegistryClient {
public:
    explicit RegistryClient(const RegistryConfig& config);
    ~RegistryClient();

    // 拉取索引（多镜像 fallback + 签名验证）
    Result<Index> fetchIndex(DownloadProgress progress = nullptr);

    // 确保包在本地（缓存或下载 + 签名验证）
    Result<std::filesystem::path> ensurePackage(
        const std::string& name,
        const std::string& version,
        DownloadProgress progress = nullptr);

    std::filesystem::path packagesDir() const;
    std::filesystem::path tarballsDir() const;

private:
    // HTTP
    Result<std::string> httpGet(const std::string& url,DownloadProgress progress = nullptr);
    Result<void> httpDownload(const std::string& url,
                              const std::filesystem::path& dest,
                              DownloadProgress progress);

    // 签名验证
    Result<std::filesystem::path> fetchAndVerifyDeveloperKey(
        const std::string& devName);
    Status verifyPackageSignature(const std::filesystem::path& tarball,
                                   const std::string& sigUrl,
                                   const std::string& devName);

    // 解压
    Status extractTarball(const std::filesystem::path& tarball,
                          const std::filesystem::path& destDir);

    RegistryConfig config_;
    Index          cachedIndex_;
    bool           indexLoaded_ = false;

    std::filesystem::path officialPubKey_;
    std::map<std::string, std::filesystem::path> developerKeys_;
};

} // namespace hpm::registry