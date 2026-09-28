// core/include/hpm/model/project.h
#pragma once

#include <hpm/model/device.h>
#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// 用户项目：project.json 的内存表示
struct Project {
    std::string mcu;                 // "stm32f103c8t6"
    std::vector<Device> devices;     // 用户声明的所有设备

    // 便捷查询：按设备名找设备
    const Device* findDevice(const DeviceName& name) const {
        for (const auto& d : devices) {
            if (d.name == name) return &d;
        }
        return nullptr;
    }

    Device* findDevice(const DeviceName& name) {
        for (auto& d : devices) {
            if (d.name == name) return &d;
        }
        return nullptr;
    }
};

} // namespace hpm