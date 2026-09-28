// core/include/hpm/result.h
#pragma once

#include <string>
#include <utility>
#include <variant>

namespace hpm {

// 错误码
enum class ErrorCode {
    Ok = 0,

    // 通用
    InvalidArgument,
    NotFound,
    AlreadyExists,
    PermissionDenied,
    IoError,

    // 清单
    ManifestParseError,
    ManifestValidationError,
    MissingField,
    InvalidField,

    // 能力检查
    PinConflict,
    PinCapabilityMismatch,
    MethodNotFound,
    NameConflict,

    // 依赖
    VersionNotFound,
    VersionConflict,
    CyclicDependency,

    // 网络
    NetworkError,
    DownloadFailed,
    ChecksumMismatch,

    // 工具链 / 构建
    ToolchainNotFound,
    BuildFailed,
    FlashFailed,

    // 内部
    Internal,
};

// 错误对象
class Error {
public:
    Error() = default;

    Error(ErrorCode code, std::string message)
        : code_(code), message_(std::move(message)) {}

    ErrorCode code() const noexcept { return code_; }
    const std::string& message() const noexcept { return message_; }
    bool ok() const noexcept { return code_ == ErrorCode::Ok; }

private:
    ErrorCode   code_ = ErrorCode::Ok;
    std::string message_;
};

// Result<T>：要么是 T，要么是 Error
template <typename T>
class Result {
public:
    // 成功构造
    Result(T value) : data_(std::move(value)) {}

    // 失败构造
    Result(Error err) : data_(std::move(err)) {}

    bool ok() const noexcept { return std::holds_alternative<T>(data_); }
    explicit operator bool() const noexcept { return ok(); }

    // 取值（调用前请检查 ok()）
    T&        value()       { return std::get<T>(data_); }
    const T&  value() const { return std::get<T>(data_); }

    // 取错误
    const Error& error() const { return std::get<Error>(data_); }

    // 便捷访问
    T&        operator*()       { return value(); }
    const T&  operator*() const { return value(); }
    T*        operator->()      { return &value(); }
    const T*  operator->() const { return &value(); }

private:
    std::variant<T, Error> data_;
};

// 无返回值的 Result
using Status = Result<void>;

} // namespace hpm

// core/include/hpm/result.h 末尾追加

namespace hpm {

template <>
class Result<void> {
public:
    Result() : err_() {}
    Result(Error err) : err_(std::move(err)) {}

    bool ok() const noexcept { return err_.ok(); }
    explicit operator bool() const noexcept { return ok(); }

    const Error& error() const { return err_; }

private:
    Error err_;
};

} // namespace hpm