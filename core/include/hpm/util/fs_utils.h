// core/include/hpm/util/fs_utils.h
#pragma once

#include <hpm/result.h>

#include <filesystem>
#include <string>

namespace hpm::util {

// 原子写文件：先写临时文件再 rename
Status writeFileAtomic(const std::filesystem::path& path,
                       const std::string& content);

// 读整个文件
Result<std::string> readFile(const std::filesystem::path& path);

// 判断文件是否存在
bool exists(const std::filesystem::path& path);

} // namespace hpm::util