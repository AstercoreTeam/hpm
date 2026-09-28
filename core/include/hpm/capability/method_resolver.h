// core/include/hpm/capability/method_resolver.h
#pragma once

#include <hpm/capability/check_report.h>
#include <hpm/capability/driver_library.h>
#include <hpm/model/project.h>
#include <hpm/types.h>

namespace hpm {

// 检查下层驱动是否提供了上层需要的方法
class MethodResolver {
public:
    MethodResolver(const DriverLibrary& lib) : lib_(lib) {}

    // 检查 targetDeviceName 的 driver 是否提供了 req.methods 里的所有方法
    // targetDeviceName 是下层设备的设备名，用来查它的驱动清单
    bool check(const DeviceName& device,
               const ResourceKey& key,
               const Requirement& req,
               const Device& target,
               CheckReport& report) const;

private:
    const DriverLibrary& lib_;
};

} // namespace hpm