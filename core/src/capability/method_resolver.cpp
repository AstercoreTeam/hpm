// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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