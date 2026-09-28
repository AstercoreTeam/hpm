// core/src/model/version_range.cpp
#include <hpm/model/version_range.h>

#include <sstream>

namespace hpm {

namespace {

// 去除首尾空白
std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t");
    if (b == std::string::npos) return {};
    size_t e = s.find_last_not_of(" \t");
    return s.substr(b, e - b + 1);
}

} // namespace

Result<VersionRange> VersionRange::parse(const std::string& text) {
    VersionRange range;
    range.raw_ = text;

    std::string s = trim(text);
    if (s.empty() || s == "*") {
        // 空或 * 表示任意版本：不加任何 comparator
        return range;
    }

    // 按空白切分成多个条件（AND 关系）
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        Op op = Op::Exact;
        std::string verText = token;

        if (token.size() >= 2 && token[0] == '>' && token[1] == '=') {
            op = Op::Ge; verText = token.substr(2);
        } else if (token.size() >= 2 && token[0] == '<' && token[1] == '=') {
            op = Op::Le; verText = token.substr(2);
        } else if (token[0] == '>') {
            op = Op::Gt; verText = token.substr(1);
        } else if (token[0] == '<') {
            op = Op::Lt; verText = token.substr(1);
        } else if (token[0] == '=') {
            op = Op::Exact; verText = token.substr(1);
        } else if (token[0] == '^') {
            // ^1.2.0  →  >=1.2.0 <2.0.0
            auto base = Version::parse(token.substr(1));
            if (!base) return base.error();
            range.comparators_.push_back({Op::Ge, *base});
            Version upper{base->major() + 1, 0, 0};
            range.comparators_.push_back({Op::Lt, upper});
            continue;
        } else if (token[0] == '~') {
            // ~1.2.0  →  >=1.2.0 <1.3.0
            auto base = Version::parse(token.substr(1));
            if (!base) return base.error();
            range.comparators_.push_back({Op::Ge, *base});
            Version upper{base->major(), base->minor() + 1, 0};
            range.comparators_.push_back({Op::Lt, upper});
            continue;
        }

        verText = trim(verText);
        auto v = Version::parse(verText);
        if (!v) return v.error();

        // 精确版本：若不带预发布，也允许同版本号的正式版
        range.comparators_.push_back({op, *v});
    }

    return range;
}

bool VersionRange::matches(const Version& v) const {
    // 空约束 = 任意版本
    if (comparators_.empty()) return true;

    for (const auto& c : comparators_) {
        switch (c.op) {
            case Op::Exact:
                if (!(v == c.version)) return false;
                break;
            case Op::Gt:
                if (!(v > c.version)) return false;
                break;
            case Op::Ge:
                if (!(v >= c.version)) return false;
                break;
            case Op::Lt:
                if (!(v < c.version)) return false;
                break;
            case Op::Le:
                if (!(v <= c.version)) return false;
                break;
        }
    }
    return true;
}

} // namespace hpm