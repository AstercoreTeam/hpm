// core/src/capability/method_resolver.cpp
#include <hpm/capability/method_resolver.h>

namespace hpm {

bool MethodResolver::check(const DeviceName& device,
                           const ResourceKey& key,
                           const Requirement& req,
                           const Device& target,
                           CheckReport& report) const {
    if (req.methods.empty()) return true;

    const DriverManifest* dm = lib_.find(target.driver);
    if (!dm) {
        // 下层驱动清单缺失，交给别的地方报错
        return true;
    }

    for (const auto& m : req.methods) {
        if (!dm->providesMethod(m)) {
            report.addError(
                ErrorCode::MethodNotFound,
                "驱动 " + target.driver + "（设备 " + target.name +
                    "）未提供方法 " + m +
                    "，但设备 " + device + " 的 " + key + " 需要它",
                "devices." + device + "." + key);
            return false;
        }
    }
    return true;
}

} // namespace hpm