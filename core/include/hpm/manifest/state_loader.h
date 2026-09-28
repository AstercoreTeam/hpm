// core/include/hpm/manifest/state_loader.h
#pragma once

#include <hpm/json/json_value.h>
#include <hpm/model/driver_state.h>
#include <hpm/result.h>

#include <filesystem>

namespace hpm::manifest {

Result<DriverState> loadDriverState(const json::Value& json);
Result<DriverState> loadDriverStateFile(const std::filesystem::path& path);

// 序列化 DriverState → JSON
json::Value saveDriverState(const DriverState& state);

} // namespace hpm::manifest