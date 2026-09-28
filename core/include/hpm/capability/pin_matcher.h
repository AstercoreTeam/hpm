// core/include/hpm/capability/pin_matcher.h
#pragma once

#include <hpm/capability/check_report.h>
#include <hpm/model/chip.h>
#include <hpm/model/requirement.h>
#include <hpm/types.h>

namespace hpm {

// 检查一个引脚是否满足某种能力需求
// requirement.type 是资源类型，如 "gpio_out"；如果是总线资源（spi/i2c/...），
// 引脚需要具备该类型的一个能力（如 spi_sck）
class PinMatcher {
public:
    explicit PinMatcher(const Chip& chip) : chip_(chip) {}

    // 检查 pin 是否满足 req.type 对应的能力
    // 返回 true 表示通过；否则写入 report
    bool check(const PinName& pin,
               const Requirement& req,
               const DeviceName& device,
               CheckReport& report) const;

private:
    const Chip& chip_;
};

} // namespace hpm