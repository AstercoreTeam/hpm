// core/src/manifest/project_loader.cpp
#include <hpm/manifest/project_loader.h>

#include <hpm/json/json_io.h>
#include <hpm/manifest/field_reader.h>

namespace hpm::manifest {

namespace {

Result<Device> loadDevice(const std::string& name,
                          const json::Value& obj) {
    FieldReader r(obj, "devices." + name);

    Device d;
    d.name = name;
    d.userDeclared = true;

    auto driver = r.requireString("driver");
    if (!driver) return driver.error();
    d.driver = *driver;

    // 其他字段全部当作资源绑定（pin/bus/...）
    // 约定：字段名 == requires 里的 key
    auto keys = obj.keys();
    if (!keys) return keys.error();
    for (const auto& k : *keys) {
        if (k == "driver" || k == "version") continue;
        auto v = obj.get(k);
        if (!v) return v.error();
        if (!v->isString()) {
            return Error(ErrorCode::InvalidField,
                         "devices." + name + "." + k +
                         " 应为字符串（资源绑定的值）");
        }
        auto s = v->asString();
        if (!s) return s.error();
        d.bindings[k] = *s;
    }

    // 可选：版本
    if (obj.has("version")) {
        auto vs = r.requireString("version");
        if (!vs) return vs.error();
        auto ver = Version::parse(*vs);
        if (!ver) return ver.error();
        d.version = *ver;
    }

    return d;
}

} // namespace

Result<Project> loadProject(const json::Value& json) {
    if (!json.isObject()) {
        return Error(ErrorCode::InvalidField,
                     "project.json 顶层必须是对象");
    }

    FieldReader r(json);

    Project proj;

    auto mcu = r.requireString("mcu");
    if (!mcu) return mcu.error();
    proj.mcu = *mcu;

    auto devicesJson = r.requireObject("devices");
    if (!devicesJson) return devicesJson.error();

    FieldReader dr(*devicesJson, "devices");
    auto keys = dr.keys();
    if (!keys) return keys.error();

    for (const auto& name : *keys) {
        auto obj = devicesJson->get(name);
        if (!obj) return obj.error();
        if (!obj->isObject()) {
            return Error(ErrorCode::InvalidField,
                         "devices." + name + " 必须是对象");
        }
        auto dev = loadDevice(name, *obj);
        if (!dev) return dev.error();
        proj.devices.push_back(std::move(*dev));
    }

    return proj;
}

Result<Project> loadProjectFile(const std::filesystem::path& path) {
    auto json = json::parseFile(path);
    if (!json) return json.error();
    return loadProject(*json);
}

} // namespace hpm::manifest