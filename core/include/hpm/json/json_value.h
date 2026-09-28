// core/include/hpm/json/json_value.h
#pragma once

#include <hpm/result.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hpm::json {

// JSON 值类型
enum class Type {
    Null,
    Bool,
    Int,
    Double,
    String,
    Array,
    Object,
};

// 轻量 JSON 值封装（值语义），错误用 Result 表达，不抛异常
class Value {
public:
    // ---------- 构造 ----------
    Value();                                  // null
    Value(bool b);
    Value(int32_t i);
    Value(int64_t i);
    Value(uint32_t i);
    Value(uint64_t i);
    Value(double d);
    Value(const char* s);
    Value(std::string s);

    // 值语义：拷贝深拷贝，移动转移
    Value(const Value& other);
    Value(Value&& other) noexcept;
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;
    ~Value();

    // ---------- 工厂 ----------
    static Value array();
    static Value object();

    // ---------- 类型查询 ----------
    Type type() const noexcept;
    bool isNull()   const noexcept;
    bool isBool()   const noexcept;
    bool isInt()    const noexcept;
    bool isDouble() const noexcept;
    bool isNumber() const noexcept;
    bool isString() const noexcept;
    bool isArray()  const noexcept;
    bool isObject() const noexcept;

    // ---------- 标量访问（类型不符返回 Error） ----------
    Result<bool>        asBool()   const;
    Result<int64_t>     asInt()    const;
    Result<double>      asDouble() const;
    Result<std::string> asString() const;

    // ---------- 数组访问 ----------
    Result<size_t> size() const;                 // 数组或对象的元素数
    Result<Value>  at(size_t index) const;       // 越界返回 Error
    void           push(Value v);                // 非数组时自动转成数组

    // ---------- 对象访问 ----------
    bool           has(const std::string& key) const;
    Result<Value>  get(const std::string& key) const;    // 缺失返回 Error
    void           set(const std::string& key, Value v); // 非对象时自动转成对象
    Result<std::vector<std::string>> keys() const;

    // ---------- 便捷访问：带默认值（缺失或类型不符时返回默认） ----------
    bool        getBool  (const std::string& key, bool def = false) const;
    int64_t     getInt   (const std::string& key, int64_t def = 0) const;
    double      getDouble(const std::string& key, double def = 0.0) const;
    std::string getString(const std::string& key, const std::string& def = "") const;

    // ---------- 内部：访问底层 nlohmann 对象（供 parser/writer 使用） ----------
    struct Impl;
    Impl&       impl();
    const Impl& impl() const;

private:
    std::unique_ptr<Impl> impl_;
};

} // namespace hpm::json