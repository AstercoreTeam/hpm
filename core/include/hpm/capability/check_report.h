// core/include/hpm/capability/check_report.h
#pragma once

#include <hpm/result.h>

#include <string>
#include <vector>

namespace hpm {

// 检查中的一条问题
struct CheckIssue {
    ErrorCode   code;
    std::string message;
    std::string location;   // "devices.door_reader.cs" 之类

    CheckIssue() = default;
    CheckIssue(ErrorCode c, std::string msg, std::string loc = {})
        : code(c), message(std::move(msg)), location(std::move(loc)) {}
};

// 检查报告
class CheckReport {
public:
    void addError(ErrorCode code, std::string message, std::string location = {}) {
        errors_.emplace_back(code, std::move(message), std::move(location));
    }

    void addWarning(std::string message, std::string location = {}) {
        warnings_.emplace_back(ErrorCode::Ok, std::move(message), std::move(location));
    }

    bool ok() const noexcept { return errors_.empty(); }
    explicit operator bool() const noexcept { return ok(); }

    const std::vector<CheckIssue>& errors()   const noexcept { return errors_; }
    const std::vector<CheckIssue>& warnings() const noexcept { return warnings_; }

    // 汇总成一条错误（用于返回给上层）
    Error toError() const {
        if (errors_.empty()) {
            return Error(ErrorCode::Ok, "");
        }
        std::string msg = "能力检查失败 (" + std::to_string(errors_.size()) + " 个问题):";
        for (const auto& e : errors_) {
            msg += "\n  - ";
            if (!e.location.empty()) msg += "[" + e.location + "] ";
            msg += e.message;
        }
        return Error(errors_.front().code, msg);
    }

private:
    std::vector<CheckIssue> errors_;
    std::vector<CheckIssue> warnings_;
};

} // namespace hpm