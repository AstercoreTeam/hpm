// core/src/json/json_impl.h
#pragma once

// 这个头文件只应该在 core/src/json/ 下的 .cpp 里 include，
// 不应该被公共头文件或外部代码引用。
//
// 它定义了 Value::Impl 的完整结构，让多个 .cpp 能操作底层 nlohmann::json。

#include <nlohmann/json.hpp>

#include <hpm/json/json_value.h>

namespace hpm::json {

struct Value::Impl {
    nlohmann::json data;

    Impl() = default;
    explicit Impl(nlohmann::json j) : data(std::move(j)) {}
};

} // namespace hpm::json