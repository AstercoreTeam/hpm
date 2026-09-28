// core/src/version.cpp
#include "hpm/hpm.h"

namespace hpm {

const char* version() noexcept {
    return HPM_VERSION_STRING;
}

} // namespace hpm