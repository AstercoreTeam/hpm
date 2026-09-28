// core/include/hpm/json/json_io.h
#pragma once

#include <hpm/json/json_value.h>
#include <hpm/result.h>

#include <filesystem>
#include <string>

namespace hpm::json {

// 从字符串解析
Result<Value> parse(const std::string& text);

// 从文件解析
Result<Value> parseFile(const std::filesystem::path& path);

// 序列化（indent 为 -1 时不缩进）
std::string dump(const Value& v, int indent = 2);

// 写文件（自动创建父目录，原子写）
Result<void> writeFile(const Value& v,
                       const std::filesystem::path& path,
                       int indent = 2);

} // namespace hpm::json