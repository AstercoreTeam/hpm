// core/include/hpm/model/pin.h
#pragma once

#include <hpm/model/pin_capability.h>
#include <hpm/types.h>

#include <vector>

namespace hpm {

// 一个引脚：名字 + 能力列表 + 是否可共享
// shareable 表示该引脚在被一个设备占用后，是否还能被其他设备共享
// （总线信号如 SCK/MISO/MOSI 通常可共享，CS/RST 通常独占）
struct Pin {
    PinName name;                            // "PA5"
    std::vector<PinCapability> capabilities; // 能做哪些功能
    bool shareable = false;

    // 查询：这个引脚能否承担某种能力
    bool hasCapability(const ResourceType& type,
                       const std::string& instance = "") const {
        for (const auto& cap : capabilities) {
            if (cap.type != type) continue;
            if (instance.empty() || cap.instance == instance) return true;
        }
        return false;
    }
};

} // namespace hpm