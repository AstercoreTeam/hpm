// core/src/model/version.cpp
#include <hpm/model/version.h>

#include <algorithm>
#include <charconv>
#include <sstream>
#include <vector>

namespace hpm {

namespace {

// 解析点分数字，失败返回 nullopt
std::optional<uint32_t> parseNumber(const std::string& s) {
    if (s.empty()) return std::nullopt;
    uint32_t value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc() || ptr != s.data() + s.size()) return std::nullopt;
    return value;
}

// 按分隔符切分
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

// 比较预发布标识符（semver 规则）
// 规则：数字 < 字母；数字按数值比；字母按字典序；字段少的 < 字段多的
int comparePrerelease(const std::string& a, const std::string& b) {
    if (a.empty() && b.empty()) return 0;
    if (a.empty()) return 1;   // 无预发布 > 有预发布
    if (b.empty()) return -1;

    auto aParts = split(a, '.');
    auto bParts = split(b, '.');

    size_t n = std::min(aParts.size(), bParts.size());
    for (size_t i = 0; i < n; ++i) {
        const auto& x = aParts[i];
        const auto& y = bParts[i];

        bool xNum = !x.empty() && std::all_of(x.begin(), x.end(), ::isdigit);
        bool yNum = !y.empty() && std::all_of(y.begin(), y.end(), ::isdigit);

        if (xNum && yNum) {
            uint32_t xi = 0, yi = 0;
            std::from_chars(x.data(), x.data() + x.size(), xi);
            std::from_chars(y.data(), y.data() + y.size(), yi);
            if (xi != yi) return xi < yi ? -1 : 1;
        } else if (xNum) {
            return -1;  // 数字 < 字母
        } else if (yNum) {
            return 1;
        } else {
            if (x != y) return x < y ? -1 : 1;
        }
    }

    if (aParts.size() != bParts.size()) {
        return aParts.size() < bParts.size() ? -1 : 1;
    }
    return 0;
}

} // namespace

Result<Version> Version::parse(const std::string& text) {
    if (text.empty()) {
        return Error(ErrorCode::InvalidArgument, "版本号为空");
    }

    std::string s = text;

    // 提取 build 元数据（+ 之后）
    std::string build;
    if (auto pos = s.find('+'); pos != std::string::npos) {
        build = s.substr(pos + 1);
        s = s.substr(0, pos);
    }

    // 提取 prerelease（- 之后）
    std::string prerelease;
    if (auto pos = s.find('-'); pos != std::string::npos) {
        prerelease = s.substr(pos + 1);
        s = s.substr(0, pos);
    }

    // 解析 major.minor.patch
    auto parts = split(s, '.');
    if (parts.size() != 3) {
        return Error(ErrorCode::InvalidArgument,
                     "版本号格式错误（应为 major.minor.patch）: " + text);
    }

    auto major = parseNumber(parts[0]);
    auto minor = parseNumber(parts[1]);
    auto patch = parseNumber(parts[2]);

    if (!major || !minor || !patch) {
        return Error(ErrorCode::InvalidArgument,
                     "版本号包含非数字字段: " + text);
    }

    return Version(*major, *minor, *patch,
                   std::move(prerelease), std::move(build));
}

std::string Version::toString() const {
    std::string s = std::to_string(major_) + "." +
                    std::to_string(minor_) + "." +
                    std::to_string(patch_);
    if (!prerelease_.empty()) s += "-" + prerelease_;
    if (!build_.empty())      s += "+" + build_;
    return s;
}

std::strong_ordering Version::operator<=>(const Version& other) const {
    if (major_ != other.major_) return major_ <=> other.major_;
    if (minor_ != other.minor_) return minor_ <=> other.minor_;
    if (patch_ != other.patch_) return patch_ <=> other.patch_;

    int pr = comparePrerelease(prerelease_, other.prerelease_);
    if (pr < 0) return std::strong_ordering::less;
    if (pr > 0) return std::strong_ordering::greater;

    return std::strong_ordering::equal;
    // 注意：build 元数据不参与比较（semver 规则）
}

bool Version::operator==(const Version& other) const {
    return (*this <=> other) == std::strong_ordering::equal;
}

} // namespace hpm