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

// core/src/manifest/driver_loader.cpp
#include <hpm/manifest/driver_loader.h>

#include <hpm/json/json_io.h>
#include <hpm/manifest/field_reader.h>

namespace hpm::manifest {

namespace {

Result<Requirement> loadRequirement(const std::string& key,
                                    const json::Value& obj) {
    FieldReader r(obj, "requires." + key);

    Requirement req;
    req.key = key;

    auto type = r.requireString("type");
    if (!type) return type.error();
    req.type = *type;

    req.optional = r.optBool("optional", false);

    // methods 可选，可以是字符串数组
    auto methods = r.optArray("methods");
    if (!methods) return methods.error();
    for (const auto& m : *methods) {
        auto s = m.asString();
        if (!s) {
            return Error(ErrorCode::InvalidField,
                         "requires." + key + ".methods 元素应为字符串");
        }
        req.methods.push_back(*s);
    }

    return req;
}

Result<ProvidedMethod> loadProvidedMethod(const json::Value& v,
                                          const std::string& ctx) {
    if (v.isString()) {
        auto s = v.asString();
        if (!s) return s.error();
        ProvidedMethod m;
        m.name = *s;
        return m;
    }
    if (v.isObject()) {
        FieldReader r(v, ctx);
        ProvidedMethod m;
        auto name = r.requireString("name");
        if (!name) return name.error();
        m.name = *name;
        m.signature = r.optString("signature");
        return m;
    }
    return Error(ErrorCode::InvalidField,
                 ctx + " 应为字符串或对象");
}

} // namespace

Result<DriverManifest> loadDriverManifest(const json::Value& json) {
    if (!json.isObject()) {
        return Error(ErrorCode::InvalidField,
                     "驱动清单顶层必须是对象");
    }

    FieldReader r(json);

    DriverManifest dm;

    auto name = r.requireString("name");
    if (!name) return name.error();
    dm.name = *name;

    auto verStr = r.requireString("version");
    if (!verStr) return verStr.error();
    auto ver = Version::parse(*verStr);
    if (!ver) return ver.error();
    dm.version = *ver;

    dm.description = r.optString("description");

    // requires
    auto reqJson = r.optObject("requires");
    if (!reqJson) return reqJson.error();
    if (reqJson->isObject()) {
        FieldReader rr(*reqJson, "requires");
        auto keys = rr.keys();
        if (!keys) return keys.error();
        for (const auto& key : *keys) {
            auto obj = reqJson->get(key);
            if (!obj) return obj.error();
            if (!obj->isObject()) {
                return Error(ErrorCode::InvalidField,
                             "requires." + key + " 必须是对象");
            }
            auto req = loadRequirement(key, *obj);
            if (!req) return req.error();
            dm.requirements.push_back(std::move(*req));
        }
    }

    // provides
    auto provJson = r.optObject("provides");
    if (!provJson) return provJson.error();
    if (provJson->isObject()) {
        auto methodsJson = provJson->get("methods");
        if (methodsJson && methodsJson->isArray()) {
            auto n = methodsJson->size();
            if (!n) return n.error();
            for (size_t i = 0; i < *n; ++i) {
                auto item = methodsJson->at(i);
                if (!item) return item.error();
                auto m = loadProvidedMethod(*item,
                                             "provides.methods[" + std::to_string(i) + "]");
                if (!m) return m.error();
                dm.provides.push_back(std::move(*m));
            }
        }
    }

    return dm;
}

Result<DriverManifest> loadDriverManifestFile(const std::filesystem::path& path) {
    auto json = json::parseFile(path);
    if (!json) return json.error();
    return loadDriverManifest(*json);
}

} // namespace hpm::manifest