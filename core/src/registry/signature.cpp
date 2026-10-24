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

// core/src/registry/signature.cpp
#include <hpm/registry/signature.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace hpm::registry {

namespace {

std::vector<unsigned char> readFile(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(f),
        std::istreambuf_iterator<char>());
}

std::string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    return oss.str();
}

} // namespace

Result<bool> verifySignature(const std::filesystem::path& pubKeyPem,
                              const std::filesystem::path& sigFile,
                              const std::filesystem::path& dataFile) {
    // 读公钥
    FILE* pubFile = fopen(pubKeyPem.string().c_str(), "r");
    if (!pubFile) {
        return Error(ErrorCode::IoError,
                     "无法打开公钥: " + pubKeyPem.string());
    }
    EVP_PKEY* pkey = PEM_read_PUBKEY(pubFile, nullptr, nullptr, nullptr);
    fclose(pubFile);

    if (!pkey) {
        return Error(ErrorCode::InvalidField,
                     "公钥格式错误: " + pubKeyPem.string());
    }

    // 读签名
    auto sig = readFile(sigFile);
    if (sig.empty()) {
        EVP_PKEY_free(pkey);
        return Error(ErrorCode::IoError,
                     "无法读签名: " + sigFile.string());
    }

    // 读数据
    auto data = readFile(dataFile);
    if (data.empty()) {
        EVP_PKEY_free(pkey);
        return Error(ErrorCode::IoError,
                     "无法读数据: " + dataFile.string());
    }

    // 根据密钥类型选 digest
    int keyType = EVP_PKEY_base_id(pkey);
    const EVP_MD* md = nullptr;

    if (keyType == EVP_PKEY_RSA) {
        md = EVP_sha256();
    } else if (keyType == EVP_PKEY_EC) {
        md = EVP_sha256();
    } else if (keyType == EVP_PKEY_ED25519) {
        md = nullptr;   // Ed25519 不做哈希
    } else if (keyType == EVP_PKEY_ED448) {
        md = nullptr;   // Ed448 也不做哈希
    } else {
        EVP_PKEY_free(pkey);
        return Error(ErrorCode::InvalidField,
                     "不支持的密钥类型: " + std::to_string(keyType));
    }

    // 验证
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    bool ok = false;
    if (ctx) {
        if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, pkey) == 1) {
            if (md) {
                // RSA / ECDSA：先 update 再 final
                if (EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) == 1) {
                    int rc = EVP_DigestVerifyFinal(ctx, sig.data(), sig.size());
                    ok = (rc == 1);
                }
            } else {
                // Ed25519 / Ed448：一次性 verify
                int rc = EVP_DigestVerify(ctx, sig.data(), sig.size(),
                                           data.data(), data.size());
                ok = (rc == 1);
            }
        }
        EVP_MD_CTX_free(ctx);
    }

    EVP_PKEY_free(pkey);
    return ok;
}

bool isValidPublicKey(const std::filesystem::path& pubKeyPem) {
    FILE* f = fopen(pubKeyPem.string().c_str(), "r");
    if (!f) return false;
    EVP_PKEY* pkey = PEM_read_PUBKEY(f, nullptr, nullptr, nullptr);
    fclose(f);
    if (!pkey) return false;
    EVP_PKEY_free(pkey);
    return true;
}

std::string sha256File(const std::filesystem::path& file) {
    std::ifstream f(file, std::ios::binary);
    if (!f) return "";

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return "";

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    char buf[4096];
    while (f.read(buf, sizeof(buf)) || f.gcount() > 0) {
        EVP_DigestUpdate(ctx, buf, static_cast<size_t>(f.gcount()));
    }

    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    return toHex(hash, hashLen);
}

std::string publicKeyFingerprint(const std::filesystem::path& pubKeyPem) {
    auto data = readFile(pubKeyPem);
    if (data.empty()) return "";

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return "";
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    auto hex = toHex(hash, hashLen);

    std::ostringstream oss;
    for (size_t i = 0; i < hex.size(); i += 2) {
        if (i > 0) oss << ":";
        oss << hex.substr(i, 2);
    }
    return oss.str();
}

} // namespace hpm::registry