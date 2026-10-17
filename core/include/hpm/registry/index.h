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

// core/include/hpm/registry/index.h
#pragma once

#include <hpm/model/version.h>
#include <hpm/result.h>
#include <hpm/types.h>

#include <map>
#include <string>
#include <vector>

namespace hpm::registry {

// 开发者信息
struct DeveloperInfo {
    std::string pub;       // 公钥 URL
    std::string pubSig;    // 公钥签名 URL
};

// 索引里的一个包版本
struct PackageVersion {
    std::string kind;       // "driver" / "chip"
    std::string developer;  // 开发者名
    std::string url;        // tarball URL
    std::string sigUrl;     // 包签名 URL
    std::string sha256;
    uint64_t    size = 0;
    std::string license;

    // requires 简版（key -> type），用于依赖递归
    std::map<std::string, std::string> requirements;
};

// 一个包的所有版本
struct PackageInfo {
    std::map<std::string, PackageVersion> versions;   // "1.0.0" -> info
};

// 索引
struct Index {
    int schema = 1;
    std::string officialPub;   // 官方公钥 URL（仅参考，实际用硬编码地址）
    std::map<std::string, DeveloperInfo> developers;
    std::map<DriverName, PackageInfo> packages;

    // 查找某个包的某个版本
    const PackageVersion* find(const std::string& name,
                                const std::string& version) const;

    // 查找某个包的最新版本
    const PackageVersion* findLatest(const std::string& name) const;

    // 找最新版本号
    std::string findLatestVersion(const std::string& name) const;
};

// 解析 index.json
Result<Index> parseIndex(const std::string& jsonText);

} // namespace hpm::registry