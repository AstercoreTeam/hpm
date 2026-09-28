// core/include/hpm/capability/capability_checker.h
#pragma once

#include <hpm/capability/check_report.h>
#include <hpm/capability/driver_library.h>
#include <hpm/model/chip.h>
#include <hpm/model/project.h>

namespace hpm {

// 能力检查总调度
class CapabilityChecker {
public:
    CapabilityChecker(const DriverLibrary& lib, const Chip& chip)
        : lib_(lib), chip_(chip) {}

    // 检查整个项目，返回报告（不抛异常，不返回值错误）
    CheckReport check(const Project& project) const;

private:
    // 单设备检查：遍历它的每个 requires key
    void checkDevice(const Project& project,
                     const Device& device,
                     class PinAllocator& allocator,
                     CheckReport& report) const;

    // 名字唯一性检查
    void checkNames(const Project& project, CheckReport& report) const;

    const DriverLibrary& lib_;
    const Chip&          chip_;
};

} // namespace hpm