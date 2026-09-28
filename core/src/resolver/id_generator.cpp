// core/src/resolver/id_generator.cpp
#include <hpm/resolver/id_generator.h>

#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>

namespace hpm {

std::string generateIntermediateId(const std::string& prefix) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFF);

    std::ostringstream oss;
    oss << "__" << prefix << "_"
        << std::hex << std::setw(4) << std::setfill('0') << dist(gen);
    return oss.str();
}

} // namespace hpm