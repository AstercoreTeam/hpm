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

// core/src/generator/hpm_h_generator.cpp
#include <hpm/generator/hpm_h_generator.h>

#include <sstream>

namespace hpm {

std::string HpmHGenerator::generate() const {
    std::ostringstream out;
    out << "// 由 HPM 自动生成（hpm dev --export）\n";
    out << "// 底层 API，绕过 driver.h 的应用抽象\n";
    out << "#pragma once\n\n";
    out << "#include <stdint.h>\n";
    out << "#include <stddef.h>\n\n";
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";

    generatePinApi(out);
    generateChipApi(out);
    generateUserDevices(out);
    generateIntermediateDevices(out);

    out << "// ========== 全局 ==========\n";
    out << "void hpm_init_all(void);\n\n";

    out << "#ifdef __cplusplus\n}\n#endif\n";
    return out.str();
}

void HpmHGenerator::generatePinApi(std::ostringstream& out) const {
    out << "// ========== 芯片引脚 ==========\n";
    for (const auto& pin : chip_.pins) {
        out << "void hpm_pin_" << pin.name << "_init(void);\n";
        out << "void hpm_pin_" << pin.name << "_write(int level);\n";
        out << "int  hpm_pin_" << pin.name << "_read(void);\n";
    }
    out << "\n";
}

void HpmHGenerator::generateChipApi(std::ostringstream& out) const {
    out << "// ========== 芯片初始化 ==========\n";
    for (const auto& m : chip_.provides) {
        out << m.declaration(chip_.name + "_") << "\n";
    }
    out << "\n";
}

void HpmHGenerator::generateUserDevices(std::ostringstream& out) const {
    out << "// ========== 用户设备 ==========\n";
    for (const auto& dev : state_.userDevices) {
        out << "// --- " << dev.name << " (" << dev.driver << ") ---\n";
        const DriverManifest* dm = lib_.find(dev.driver);
        if (!dm) continue;
        for (const auto& m : dm->provides) {
            out << m.declaration(dev.name + "_") << "\n";
        }
        out << "\n";
    }
}

void HpmHGenerator::generateIntermediateDevices(std::ostringstream& out) const {
    out << "// ========== 中间设备 ==========\n";
    for (const auto& idev : state_.intermediateDevices) {
        out << "// --- " << idev.name << " (" << idev.driver << ") ---\n";
        const DriverManifest* dm = lib_.find(idev.driver);
        if (!dm) continue;
        for (const auto& m : dm->provides) {
            out << m.declaration(idev.name + "_") << "\n";
        }
        out << "\n";
    }
}

} // namespace hpm