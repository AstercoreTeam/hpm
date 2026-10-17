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

// core/include/hpm/generator/hpm_h_generator.h
#pragma once

#include <hpm/capability/driver_library.h>
#include <hpm/model/chip.h>
#include <hpm/model/driver_state.h>

#include <sstream>
#include <string>

#include <string>

namespace hpm {

// 生成 hpm.h（底层 API，全部暴露）
// - 芯片引脚函数
// - 芯片初始化
// - 用户设备方法
// - 中间设备方法
// - hpm_init_all
class HpmHGenerator {
public:
    HpmHGenerator(const DriverState& state,
                  const DriverLibrary& lib,
                  const Chip& chip)
        : state_(state), lib_(lib), chip_(chip) {}

    std::string generate() const;

private:
    void generatePinApi(std::ostringstream& out) const;
    void generateChipApi(std::ostringstream& out) const;
    void generateUserDevices(std::ostringstream& out) const;
    void generateIntermediateDevices(std::ostringstream& out) const;

    const DriverState&   state_;
    const DriverLibrary& lib_;
    const Chip&          chip_;
};

} // namespace hpm
