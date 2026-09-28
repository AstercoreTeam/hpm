// core/include/hpm/generator/template_engine.h
#pragma once

#include <map>
#include <string>

namespace hpm {

// 简单模板替换：把 "{{key}}" 替换为 value
// 未找到的 key 保留原样（便于排查）
std::string renderTemplate(
    const std::string& tmpl,
    const std::map<std::string, std::string>& vars);

} // namespace hpm