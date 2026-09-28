// core/src/capability/pin_allocator.cpp
#include <hpm/capability/pin_allocator.h>

namespace hpm {

void PinAllocator::allocate(const PinName& pin,
                            const DeviceName& device,
                            const ResourceKey& key,
                            CheckReport& report) {
    // 引脚是否在芯片上
    const Pin* pinInfo = chip_.findPin(pin);
    if (!pinInfo) {
        report.addError(ErrorCode::PinCapabilityMismatch,
                        "芯片 " + chip_.name + " 没有引脚 " + pin,
                        "devices." + device + "." + key);
        return;
    }

    auto& uses = users_[pin];

    // 已占用：检查是否可共享
    if (!uses.empty()) {
        bool shareable = pinInfo->shareable;
        if (!shareable) {
            std::string occupiedBy;
            for (size_t i = 0; i < uses.size(); ++i) {
                if (i > 0) occupiedBy += ", ";
                occupiedBy += uses[i].device + "." + uses[i].key;
            }
            report.addError(ErrorCode::PinConflict,
                            "引脚 " + pin + " 已被占用（不可共享），现有占用者: " + occupiedBy,
                            "devices." + device + "." + key);
            return;
        }
    }

    uses.push_back({device, key, pinInfo->shareable});
}

const std::vector<PinUse>* PinAllocator::users(const PinName& pin) const {
    auto it = users_.find(pin);
    return it == users_.end() ? nullptr : &it->second;
}

} // namespace hpm