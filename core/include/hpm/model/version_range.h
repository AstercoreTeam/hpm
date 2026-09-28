// core/include/hpm/model/version_range.h
#pragma once

#include <hpm/model/version.h>
#include <string>
#include <vector>

namespace hpm {

// 版本约束：支持 ^、~、精确、>=、>、<=、<、=、*、范围（用空格分隔的多个条件，AND 关系）
// 例：
//   "^1.2.0"   >=1.2.0 <2.0.0
//   "~1.2.0"   >=1.2.0 <1.3.0
//   "1.2.3"    精确
//   ">=1.2.0"  下界
//   "*"        任意
//   ">=1.2.0 <2.0.0"  显式范围
class VersionRange {
public:
    VersionRange() = default;

    // 解析约束表达式
    static Result<VersionRange> parse(const std::string& text);

    // 判断某版本是否满足约束
    bool matches(const Version& v) const;

    // 原始表达式
    const std::string& toString() const noexcept { return raw_; }

private:
    // 单个条件
    enum class Op { Exact, Gt, Ge, Lt, Le };

    struct Comparator {
        Op       op;
        Version  version;
    };

    std::vector<Comparator> comparators_;
    std::string             raw_;
};

} // namespace hpm