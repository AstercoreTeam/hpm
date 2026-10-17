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

// core/include/hpm/model/chip.h
#pragma once

#include <hpm/model/flash_config.h>
#include <hpm/model/pin.h>
#include <hpm/model/provided_method.h>
#include <hpm/model/toolchain_spec.h>

#include <string>
#include <vector>

namespace hpm {

// 芯片构建配置
struct ChipBuildConfig {
    std::string cpu;
    std::vector<std::string> flags;
    std::string ldscript;
    std::string startup;
};

struct Chip {
    std::string name;
    std::string family;
    std::string package;
    std::string license; 

    std::vector<Pin> pins;
    ToolchainSpec    toolchain;
    FlashConfig      flash;
    ChipBuildConfig  build;
    std::string      qemuMachine;

    // 芯片驱动提供的方法（如 init）
    std::vector<ProvidedMethod> provides;

    const Pin* findPin(const PinName& pinName) const {
        for (const auto& p : pins) {
            if (p.name == pinName) return &p;
        }
        return nullptr;
    }

    bool providesMethod(const MethodName& m) const {
        for (const auto& p : provides) {
            if (p.name == m) return true;
        }
        return false;
    }
};

} // namespace hpm