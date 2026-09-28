// core/include/hpm/manifest/project_loader.h
#pragma once

#include <hpm/json/json_value.h>
#include <hpm/model/project.h>
#include <hpm/result.h>

#include <filesystem>

namespace hpm::manifest {

// 从 JSON 对象加载 Project
Result<Project> loadProject(const json::Value& json);

// 从文件加载 Project
Result<Project> loadProjectFile(const std::filesystem::path& path);

} // namespace hpm::manifest