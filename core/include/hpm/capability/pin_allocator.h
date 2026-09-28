// core/include/hpm/capability/pin_allocator.h
#pragma once

#include <hpm/capability/check_report.h>
#include <hpm/model/chip.h>
#include <hpm/types.h>

#include <map>
#include <string>
#include <vector>

namespace hpm {

// 记录一个引脚被谁占用了
struct PinUse {
    DeviceName   device;     // "door_reader"
    ResourceKey  key;        // "cs"
    bool         shareable;  // 来自 Chip::Pin::shareable
};

// 引脚分配表：检查引脚冲突
class PinAllocator {
public:
    explicit PinAllocator(const Chip& chip) : chip_(chip) {}

    // 申请一个引脚，冲突时记录到 report
    // device/key 用于错误消息
    void allocate(const PinName& pin,
                  const DeviceName& device,
                  const ResourceKey& key,
                  CheckReport& report);

    // 查询某引脚被谁用了
    const std::vector<PinUse>* users(const PinName& pin) const;

private:
    const Chip& chip_;
    std::map<PinName, std::vector<PinUse>> users_;
};

} // namespace hpm