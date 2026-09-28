// core/include/hpm/resolver/id_generator.h
#pragma once

#include <string>

namespace hpm {

// 生成随机中间设备名，如 "__spi_a3f2"
std::string generateIntermediateId(const std::string& prefix);

} // namespace hpm