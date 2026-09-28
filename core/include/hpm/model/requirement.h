// core/include/hpm/model/requirement.h
#pragma once

#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

// 驱动 requires 里的一条需求
// 例：{ key: "bus",  type: "spi",     methods: ["transfer"], optional: false }
//     { key: "cs",   type: "gpio_out", methods: ["write"],    optional: false }
//     { key: "rst",  type: "gpio_out", methods: ["write"],    optional: true  }
struct Requirement {
    ResourceKey key;                          // "bus" / "cs" / "rst"
    ResourceType type;                        // "spi" / "gpio_out"
    std::vector<MethodName> methods;          // 需要下层驱动提供的方法
    bool optional = false;
};

} // namespace hpm