// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// dev/src/dev_config.cpp
#include "dev_config.h"

#include <hpm/json/json_io.h>
#include <hpm/util/fs_utils.h>

#include <cstdlib>

namespace hpm::cli {

std::filesystem::path devConfigPath() {
    if (const char* env = std::getenv("HPM_HOME"); env && *env) {
        return std::filesystem::path(env) / "dev.json";
    }
#ifdef _WIN32
    if (const char* up = std::getenv("USERPROFILE"); up && *up) {
        return std::filesystem::path(up) / ".hpm" / "dev.json";
    }
#else
    if (const char* home = std::getenv("HOME"); home && *home) {
        return std::filesystem::path(home) / ".hpm" / "dev.json";
    }
#endif
    return std::filesystem::current_path() / ".hpm" / "dev.json";
}

Result<DevConfig> loadDevConfig() {
    auto path = devConfigPath();
    if (!std::filesystem::exists(path)) {
        return Error(ErrorCode::NotFound, "dev.json 不存在");
    }

    auto j = json::parseFile(path);
    if (!j) return j.error();

    DevConfig cfg;
    cfg.accessToken = j->getString("accessToken");
    cfg.expiresAt   = j->getString("expiresAt");
    cfg.repoUrl     = j->getString("repoUrl");
    cfg.githubUrl   = j->getString("githubUrl");
    cfg.developer   = j->getString("developer");
    return cfg;
}

Status saveDevConfig(const DevConfig& cfg) {
    auto path = devConfigPath();
    std::filesystem::create_directories(path.parent_path());

    auto j = json::Value::object();
    j.set("accessToken", json::Value(cfg.accessToken));
    j.set("expiresAt", json::Value(cfg.expiresAt));
    j.set("repoUrl", json::Value(cfg.repoUrl));
    j.set("githubUrl", json::Value(cfg.githubUrl));
    j.set("developer", json::Value(cfg.developer));

    return json::writeFile(j, path);
}

} // namespace hpm::cli