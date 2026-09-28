// core/src/json/json_value.cpp
#include <hpm/json/json_value.h>
#include "json_impl.h"

#include <utility>

namespace hpm::json {

using nlohmann::json;

// ---------- 构造 ----------

Value::Value()
    : impl_(std::make_unique<Impl>()) {}

Value::Value(bool b)
    : impl_(std::make_unique<Impl>(json(b))) {}

Value::Value(int32_t i)
    : impl_(std::make_unique<Impl>(json(i))) {}

Value::Value(int64_t i)
    : impl_(std::make_unique<Impl>(json(i))) {}

Value::Value(uint32_t i)
    : impl_(std::make_unique<Impl>(json(i))) {}

Value::Value(uint64_t i)
    : impl_(std::make_unique<Impl>(json(i))) {}

Value::Value(double d)
    : impl_(std::make_unique<Impl>(json(d))) {}

Value::Value(const char* s)
    : impl_(std::make_unique<Impl>(json(std::string(s ? s : "")))) {}

Value::Value(std::string s)
    : impl_(std::make_unique<Impl>(json(std::move(s)))) {}

// ---------- 拷贝 / 移动 ----------

Value::Value(const Value& other)
    : impl_(std::make_unique<Impl>(other.impl_->data)) {}

Value::Value(Value&& other) noexcept
    : impl_(std::move(other.impl_)) {}

Value& Value::operator=(const Value& other) {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(other.impl_->data);
    }
    return *this;
}

Value& Value::operator=(Value&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

Value::~Value() = default;

// ---------- 工厂 ----------

Value Value::array() {
    Value v;
    v.impl_->data = json::array();
    return v;
}

Value Value::object() {
    Value v;
    v.impl_->data = json::object();
    return v;
}

// ---------- 内部访问 ----------

Value::Impl& Value::impl() {
    return *impl_;
}

const Value::Impl& Value::impl() const {
    return *impl_;
}

// ---------- 类型查询 ----------

Type Value::type() const noexcept {
    const auto& d = impl_->data;
    if (d.is_null())           return Type::Null;
    if (d.is_boolean())        return Type::Bool;
    if (d.is_number_integer()) return Type::Int;
    if (d.is_number_float())   return Type::Double;
    if (d.is_string())         return Type::String;
    if (d.is_array())          return Type::Array;
    if (d.is_object())         return Type::Object;
    return Type::Null;
}

bool Value::isNull()   const noexcept { return impl_->data.is_null(); }
bool Value::isBool()   const noexcept { return impl_->data.is_boolean(); }
bool Value::isInt()    const noexcept { return impl_->data.is_number_integer(); }
bool Value::isDouble() const noexcept { return impl_->data.is_number_float(); }
bool Value::isNumber() const noexcept { return impl_->data.is_number(); }
bool Value::isString() const noexcept { return impl_->data.is_string(); }
bool Value::isArray()  const noexcept { return impl_->data.is_array(); }
bool Value::isObject() const noexcept { return impl_->data.is_object(); }

// ---------- 标量访问 ----------

Result<bool> Value::asBool() const {
    if (!isBool()) {
        return Error(ErrorCode::InvalidField, "字段不是布尔值");
    }
    return impl_->data.get<bool>();
}

Result<int64_t> Value::asInt() const {
    if (!isInt()) {
        return Error(ErrorCode::InvalidField, "字段不是整数");
    }
    return impl_->data.get<int64_t>();
}

Result<double> Value::asDouble() const {
    if (!isNumber()) {
        return Error(ErrorCode::InvalidField, "字段不是数字");
    }
    return impl_->data.get<double>();
}

Result<std::string> Value::asString() const {
    if (!isString()) {
        return Error(ErrorCode::InvalidField, "字段不是字符串");
    }
    return impl_->data.get<std::string>();
}

// ---------- 数组访问 ----------

Result<size_t> Value::size() const {
    if (isArray())  return impl_->data.size();
    if (isObject()) return impl_->data.size();
    return Error(ErrorCode::InvalidField, "值不是数组或对象");
}

Result<Value> Value::at(size_t index) const {
    if (!isArray()) {
        return Error(ErrorCode::InvalidField, "值不是数组");
    }
    if (index >= impl_->data.size()) {
        return Error(ErrorCode::InvalidArgument, "数组下标越界");
    }
    Value out;
    out.impl_->data = impl_->data[index];
    return out;
}

void Value::push(Value v) {
    if (!isArray()) {
        impl_->data = json::array();
    }
    impl_->data.push_back(std::move(v.impl_->data));
}

// ---------- 对象访问 ----------

bool Value::has(const std::string& key) const {
    return isObject() && impl_->data.contains(key);
}

Result<Value> Value::get(const std::string& key) const {
    if (!isObject()) {
        return Error(ErrorCode::InvalidField, "值不是对象");
    }
    if (!impl_->data.contains(key)) {
        return Error(ErrorCode::MissingField, "缺少字段: " + key);
    }
    Value out;
    out.impl_->data = impl_->data[key];
    return out;
}

void Value::set(const std::string& key, Value v) {
    if (!isObject()) {
        impl_->data = json::object();
    }
    impl_->data[key] = std::move(v.impl_->data);
}

Result<std::vector<std::string>> Value::keys() const {
    if (!isObject()) {
        return Error(ErrorCode::InvalidField, "值不是对象");
    }
    std::vector<std::string> out;
    out.reserve(impl_->data.size());
    for (auto it = impl_->data.begin(); it != impl_->data.end(); ++it) {
        out.push_back(it.key());
    }
    return out;
}

// ---------- 便捷访问 ----------

bool Value::getBool(const std::string& key, bool def) const {
    if (!has(key)) return def;
    const auto& v = impl_->data[key];
    return v.is_boolean() ? v.get<bool>() : def;
}

int64_t Value::getInt(const std::string& key, int64_t def) const {
    if (!has(key)) return def;
    const auto& v = impl_->data[key];
    return v.is_number_integer() ? v.get<int64_t>() : def;
}

double Value::getDouble(const std::string& key, double def) const {
    if (!has(key)) return def;
    const auto& v = impl_->data[key];
    return v.is_number() ? v.get<double>() : def;
}

std::string Value::getString(const std::string& key, const std::string& def) const {
    if (!has(key)) return def;
    const auto& v = impl_->data[key];
    return v.is_string() ? v.get<std::string>() : def;
}

} // namespace hpm::json