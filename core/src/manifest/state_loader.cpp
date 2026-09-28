// core/src/manifest/state_loader.cpp
#include <hpm/manifest/state_loader.h>

#include <hpm/json/json_io.h>
#include <hpm/manifest/field_reader.h>

namespace hpm::manifest {

namespace {

Result<Device> loadDeviceEntry(const std::string& name,
                               const json::Value& obj,
                               bool userDeclared) {
    FieldReader r(obj, "devices." + name);

    Device d;
    d.name = name;
    d.userDeclared = userDeclared;

    auto driver = r.requireString("driver");
    if (!driver) return driver.error();
    d.driver = *driver;

    auto verStr = r.optString("version");
    if (!verStr.empty()) {
        auto v = Version::parse(verStr);
        if (!v) return v.error();
        d.version = *v;
    }

    auto keys = obj.keys();
    if (!keys) return keys.error();
    for (const auto& k : *keys) {
        if (k == "driver" || k == "version" || k == "name") continue;
        auto v = obj.get(k);
        if (!v) return v.error();
        if (!v->isString()) continue;
        auto s = v->asString();
        if (!s) continue;
        d.bindings[k] = *s;
    }

    return d;
}

json::Value saveDevice(const Device& d) {
    auto obj = json::Value::object();
    obj.set("driver", json::Value(d.driver));
    if (d.version.toString() != "0.0.0") {
        obj.set("version", json::Value(d.version.toString()));
    }
    for (const auto& [k, v] : d.bindings) {
        obj.set(k, json::Value(v));
    }
    return obj;
}

} // namespace

Result<DriverState> loadDriverState(const json::Value& json) {
    if (!json.isObject()) {
        return Error(ErrorCode::InvalidField,
                     "driver.json 顶层必须是对象");
    }

    FieldReader r(json);
    DriverState state;

    // user_devices
    auto userJson = r.optObject("user_devices");
    if (!userJson) return userJson.error();
    if (userJson->isObject()) {
        FieldReader ur(*userJson, "user_devices");
        auto keys = ur.keys();
        if (!keys) return keys.error();
        for (const auto& name : *keys) {
            auto obj = userJson->get(name);
            if (!obj) return obj.error();
            auto d = loadDeviceEntry(name, *obj, true);
            if (!d) return d.error();
            state.userDevices.push_back(std::move(*d));
        }
    }

    // intermediate_devices
    auto interJson = r.optObject("intermediate_devices");
    if (!interJson) return interJson.error();
    if (interJson->isObject()) {
        FieldReader ir(*interJson, "intermediate_devices");
        auto keys = ir.keys();
        if (!keys) return keys.error();
        for (const auto& id : *keys) {
            auto obj = interJson->get(id);
            if (!obj) return obj.error();
            auto d = loadDeviceEntry(id, *obj, false);
            if (!d) return d.error();

            IntermediateDevice idev;
            idev.id = id;
            idev.name = obj->getString("name", id);
            idev.driver = d->driver;
            idev.version = d->version;
            idev.bindings = std::move(d->bindings);

            state.intermediateDevices.push_back(std::move(idev));
        }
    }

    return state;
}

Result<DriverState> loadDriverStateFile(const std::filesystem::path& path) {
    auto json = json::parseFile(path);
    if (!json) return json.error();
    return loadDriverState(*json);
}

json::Value saveDriverState(const DriverState& state) {
    auto root = json::Value::object();

    auto user = json::Value::object();
    for (const auto& d : state.userDevices) {
        user.set(d.name, saveDevice(d));
    }
    root.set("user_devices", user);

    auto inter = json::Value::object();
    for (const auto& d : state.intermediateDevices) {
        auto obj = saveDevice(Device{
            .name = d.name,
            .driver = d.driver,
            .version = d.version,
            .bindings = d.bindings,
            .userDeclared = false,
        });
        obj.set("name", json::Value(d.name));
        inter.set(d.id, obj);
    }
    root.set("intermediate_devices", inter);

    return root;
}

} // namespace hpm::manifest