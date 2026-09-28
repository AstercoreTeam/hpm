// core/include/hpm/manifest/driver_loader.h
#pragma once

#include <hpm/json/json_value.h>
#include <hpm/model/driver_manifest.h>
#include <hpm/result.h>

#include <filesystem>

namespace hpm::manifest {

Result<DriverManifest> loadDriverManifest(const json::Value& json);
Result<DriverManifest> loadDriverManifestFile(const std::filesystem::path& path);

} // namespace hpm::manifest