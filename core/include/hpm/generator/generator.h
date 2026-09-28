/*
 * Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// core/include/hpm/generator/generator.h
#pragma once

#include <hpm/capability/driver_library.h>
#include <hpm/generator/generation_plan.h>
#include <hpm/model/chip.h>
#include <hpm/model/driver_state.h>
#include <hpm/model/project.h>
#include <hpm/result.h>

#include <filesystem>
#include <string>

namespace hpm {

class Generator {
public:
    Generator(const DriverState& state,
              const DriverLibrary& lib,
              const Project& project,
              const Chip& chip,
              std::filesystem::path packageCacheDir,
              std::filesystem::path driverDir)
        : state_(state),
          lib_(lib),
          project_(project),
          chip_(chip),
          packageCacheDir_(std::move(packageCacheDir)),
          driverDir_(std::move(driverDir)) {}

    Result<GenerationPlan> plan();
    Status write(const GenerationPlan& plan);
    Status generate();

private:
    void planDriverHeader(GenerationPlan& plan);
    void planDriverSource(GenerationPlan& plan);
    void planMakefile(GenerationPlan& plan);

    Status planInstance(GenerationPlan& plan,
                        const std::string& instanceName,
                        const std::string& driverName,
                        const Version& version,
                        const std::map<ResourceKey, std::string>& bindings);

    Status planChipDriver(GenerationPlan& plan);

    Status copyPackageSources(GenerationPlan& plan,
                              const std::string& driver,
                              const Version& version,
                              const std::filesystem::path& targetDir);

    std::string macroValue(const std::string& bindingValue) const;

    struct InstanceInfo {
        std::string instanceName;
        std::string driver;
        Version     version;
        std::map<ResourceKey, std::string> bindings;
    };
    std::vector<InstanceInfo> collectInstances() const;

    const DriverState&   state_;
    const DriverLibrary& lib_;
    const Project&       project_;
    const Chip&          chip_;
    std::filesystem::path packageCacheDir_;
    std::filesystem::path driverDir_;
};

} // namespace hpm