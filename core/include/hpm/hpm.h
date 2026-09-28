// core/include/hpm/hpm.h
#pragma once

// 公共 API 总入口
#include "hpm/result.h"
#include "hpm/types.h"

#define HPM_VERSION_MAJOR 0
#define HPM_VERSION_MINOR 1
#define HPM_VERSION_PATCH 0
#define HPM_VERSION_STRING "0.1.0"

namespace hpm {

const char* version() noexcept;

} // namespace hpm