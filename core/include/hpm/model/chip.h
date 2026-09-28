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