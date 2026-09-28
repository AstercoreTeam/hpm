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

// core/include/hpm/model/pin_capability.h
#pragma once

#include <hpm/types.h>
#include <string>

namespace hpm {

// 引脚的一种能力：类型 + 实例
// 例：{ type: "spi_sck", instance: "SPI1" }
//     { type: "gpio_out", instance: "" }
//     { type: "tim_ch",   instance: "TIM1", channel: 1 }
struct PinCapability {
    ResourceType type;       // spi_sck / spi_miso / spi_mosi / gpio_out / gpio_in / ...
    std::string  instance;   // SPI1 / TIM1 / "" 表示无实例
    int          channel = 0; // 可选：定时器通道等

    bool operator==(const PinCapability& o) const {
        return type == o.type
            && instance == o.instance
            && channel == o.channel;
    }
};

} // namespace hpm