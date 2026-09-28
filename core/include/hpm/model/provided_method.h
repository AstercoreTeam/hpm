// core/include/hpm/model/provided_method.h
#pragma once

#include <hpm/types.h>

#include <string>

namespace hpm {

// 驱动 provides 里的一个方法
// 签名暂时用字符串（后续可解析成参数列表做严格检查）
struct ProvidedMethod {
    MethodName name;      // "transfer" / "read_card"
    std::string signature; // 可选："int(uint8_t*, uint8_t*, size_t)"
};

} // namespace hpm