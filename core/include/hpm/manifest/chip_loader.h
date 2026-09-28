// core/include/hpm/manifest/chip_loader.h
#pragma once

#include <hpm/json/json_value.h>
#include <hpm/model/chip.h>
#include <hpm/result.h>

#include <filesystem>

namespace hpm::manifest {

Result<Chip> loadChip(const json::Value& json);
Result<Chip> loadChipFile(const std::filesystem::path& path);

} // namespace hpm::manifest