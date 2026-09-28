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
#include <hpm/model/toolchain_spec.h>

#include <string>
#include <vector>

namespace hpm {

// 芯片模型：芯片驱动提供的全部信息
struct Chip {
    std::string name;                        // "stm32f103c8t6"
    std::string family;                      // "stm32f1"
    std::string package;                     // "LQFP48"

    std::vector<Pin> pins;                   // 所有引脚及其能力
    ToolchainSpec    toolchain;              // 用哪个工具链
    FlashConfig      flash;                  // 怎么烧录
    std::string      qemuMachine;            // QEMU machine（用于测试）

    // 便捷查询：按引脚名查找
    const Pin* findPin(const PinName& pinName) const {
        for (const auto& p : pins) {
            if (p.name == pinName) return &p;
        }
        return nullptr;
    }
};

} // namespace hpm