// core/include/hpm/model/driver_state.h
#pragma once

#include <hpm/model/device.h>
#include <hpm/model/intermediate_device.h>
#include <hpm/model/version.h>
#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// driver.json 的内存表示
// 锁定：中间设备 ID、驱动版本、用户设备列表
struct DriverState {
    // 用户设备（来自 project.json，但版本已锁定）
    std::vector<Device> userDevices;

    // 中间设备（工具生成）
    std::vector<IntermediateDevice> intermediateDevices;

    // 便捷查询
    const IntermediateDevice* findIntermediate(const DeviceName& id) const {
        for (const auto& d : intermediateDevices) {
            if (d.id == id || d.name == id) return &d;
        }
        return nullptr;
    }
};

} // namespace hpm