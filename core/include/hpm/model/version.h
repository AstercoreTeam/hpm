// core/include/hpm/model/version.h
#pragma once

#include <hpm/result.h>
#include <string>
#include <compare>
#include <cstdint>
#include <optional>

namespace hpm {

// 语义化版本：major.minor.patch[-prerelease][+build]
// 例：1.2.3、1.2.3-alpha.1、1.2.3+build.5
class Version {
public:
    Version() = default;

    Version(uint32_t major, uint32_t minor, uint32_t patch,
            std::string prerelease = {},
            std::string build      = {})
        : major_(major), minor_(minor), patch_(patch),
          prerelease_(std::move(prerelease)),
          build_(std::move(build)) {}

    // 解析 "1.2.3"、"1.2.3-alpha"、"1.2.3+build"
    static Result<Version> parse(const std::string& text);

    uint32_t major() const noexcept { return major_; }
    uint32_t minor() const noexcept { return minor_; }
    uint32_t patch() const noexcept { return patch_; }
    const std::string& prerelease() const noexcept { return prerelease_; }
    const std::string& build()      const noexcept { return build_; }

    // 是否为预发布版本
    bool isPrerelease() const noexcept { return !prerelease_.empty(); }

    // 序列化
    std::string toString() const;

    // 比较（semver 规则：预发布 < 正式版）
    std::strong_ordering operator<=>(const Version& other) const;
    bool operator==(const Version& other) const;

private:
    uint32_t    major_ = 0;
    uint32_t    minor_ = 0;
    uint32_t    patch_ = 0;
    std::string prerelease_;
    std::string build_;
};

} // namespace hpm