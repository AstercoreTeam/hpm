// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
        if (k == "driver" || k == "version" || k == "name" || k == "config") continue;
        auto v = obj.get(k);
        if (!v) return v.error();
        if (!v->isString()) continue;
        auto s = v->asString();
        if (!s) continue;
        d.bindings[k] = *s;
    }

    // config
    if (obj.has("config")) {
        auto cfgObj = obj.get("config");
        if (cfgObj && cfgObj->isObject()) {
            auto cfgKeys = cfgObj->keys();
            if (cfgKeys) {
                for (const auto& k : *cfgKeys) {
                    auto v = cfgObj->get(k);
                    if (!v) continue;
                    if (v->isString()) {
                        auto s = v->asString();
                        if (s) d.config[k] = *s;
                    } else if (v->isInt()) {
                        auto i = v->asInt();
                        if (i) d.config[k] = std::to_string(*i);
                    }
                }
            }
        }
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
    if (!d.config.empty()) {
        auto cfg = json::Value::object();
        for (const auto& [k, v] : d.config) {
            cfg.set(k, json::Value(v));
        }
        obj.set("config", cfg);
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
            idev.config = std::move(d->config);   // ← 加这行

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
        Device dev;
        dev.name = d.name;
        dev.driver = d.driver;
        dev.version = d.version;
        dev.bindings = d.bindings;
        dev.config = d.config;   // ← 加这行
        dev.userDeclared = false;

        auto obj = saveDevice(dev);
        obj.set("name", json::Value(d.name));
        inter.set(d.id, obj);
    }
    root.set("intermediate_devices", inter);

    return root;
}

} // namespace hpm::manifest