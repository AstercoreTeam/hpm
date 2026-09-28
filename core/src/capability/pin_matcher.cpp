// core/src/capability/pin_matcher.cpp
#include <hpm/capability/pin_matcher.h>

namespace hpm {

namespace {

// 资源类型 → 引脚能力类型
// "gpio_out" → "gpio_out"
// "spi"      → "spi_sck" / "spi_miso" / "spi_mosi"
// "i2c"      → "i2c_scl" / "i2c_sda"
// "uart"     → "uart_tx" / "uart_rx"
bool typeMatches(const Pin& pin, const std::string& reqType) {
    // 直接匹配（gpio_out / gpio_in 等）
    if (pin.hasCapability(reqType)) return true;

    // 总线类型需要检查该引脚是否有对应的任一子能力
    // 这个判断其实是给「用户把总线引脚直接绑到设备资源上」的场景用的
    // 在 HPM 里总线引脚通常是通过中间设备绑定的，这里保持宽松
    if (reqType == "spi") {
        return pin.hasCapability("spi_sck")
            || pin.hasCapability("spi_miso")
            || pin.hasCapability("spi_mosi");
    }
    if (reqType == "i2c") {
        return pin.hasCapability("i2c_scl")
            || pin.hasCapability("i2c_sda");
    }
    if (reqType == "uart") {
        return pin.hasCapability("uart_tx")
            || pin.hasCapability("uart_rx");
    }
    return false;
}

} // namespace

bool PinMatcher::check(const PinName& pin,
                       const Requirement& req,
                       const DeviceName& device,
                       CheckReport& report) const {
    const Pin* p = chip_.findPin(pin);
    if (!p) {
        // 引脚不存在，交给 PinAllocator 报错，这里跳过
        return true;
    }

    if (!typeMatches(*p, req.type)) {
        report.addError(
            ErrorCode::PinCapabilityMismatch,
            "引脚 " + pin + " 不支持能力 " + req.type +
                "（需求来自设备 " + device + " 的 " + req.key + "）",
            "devices." + device + "." + req.key);
        return false;
    }
    return true;
}

} // namespace hpm