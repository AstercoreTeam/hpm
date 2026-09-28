// core/include/hpm/capability/driver_library.h
#pragma once

#include <hpm/model/driver_manifest.h>
#include <hpm/result.h>

#include <map>
#include <string>
#include <vector>

namespace hpm {

// 驱动清单集合：按驱动名索引
class DriverLibrary {
public:
    void add(DriverManifest manifest) {
        manifests_[manifest.name] = std::move(manifest);
    }

    const DriverManifest* find(const DriverName& name) const {
        auto it = manifests_.find(name);
        return it == manifests_.end() ? nullptr : &it->second;
    }

    bool has(const DriverName& name) const {
        return manifests_.count(name) > 0;
    }

    size_t size() const noexcept { return manifests_.size(); }

    const std::map<DriverName, DriverManifest>& all() const noexcept {
        return manifests_;
    }

private:
    std::map<DriverName, DriverManifest> manifests_;
};

} // namespace hpm