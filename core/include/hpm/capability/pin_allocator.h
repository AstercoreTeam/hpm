/*
 * Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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