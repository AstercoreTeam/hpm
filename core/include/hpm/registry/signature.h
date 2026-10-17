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

// core/include/hpm/registry/signature.h
#pragma once

#include <hpm/result.h>

#include <filesystem>
#include <string>

namespace hpm::registry {

// 用 OpenSSL 验证签名
// pubKeyPem: PEM 格式的公钥文件路径
// sigFile:   签名文件路径
// dataFile:  被签名的文件路径
Result<bool> verifySignature(const std::filesystem::path& pubKeyPem,
                              const std::filesystem::path& sigFile,
                              const std::filesystem::path& dataFile);

// 验证 PEM 公钥是否有效
bool isValidPublicKey(const std::filesystem::path& pubKeyPem);

// 计算文件 SHA256（十六进制字符串）
std::string sha256File(const std::filesystem::path& file);

// 计算公钥的 SHA256 指纹（XX:XX:XX:... 格式，用于用户验证）
std::string publicKeyFingerprint(const std::filesystem::path& pubKeyPem);

} // namespace hpm::registry