// core/include/hpm/generator/generation_plan.h
#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace hpm {

// 一个要生成的文件
struct PlannedFile {
    std::filesystem::path   relativePath;   // 相对 driver/ 的路径
    std::string             content;        // 文件内容
    bool                    overwrite = true;  // 是否覆盖已有
};

// 生成计划：所有要写的文件
struct GenerationPlan {
    std::vector<PlannedFile> files;

    void addFile(std::filesystem::path rel, std::string content) {
        files.push_back({std::move(rel), std::move(content), true});
    }
};

} // namespace hpm