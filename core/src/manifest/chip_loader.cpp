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

// core/src/manifest/chip_loader.cpp
#include <hpm/manifest/chip_loader.h>

#include <hpm/json/json_io.h>
#include <hpm/manifest/field_reader.h>

namespace hpm::manifest {

namespace {

Result<PinCapability> loadCapability(const json::Value& obj,
                                     const std::string& ctx) {
    FieldReader r(obj, ctx);
    PinCapability cap;
    auto type = r.requireString("type");
    if (!type) return type.error();
    cap.type = *type;
    cap.instance = r.optString("instance");
    cap.channel = static_cast<int>(r.optInt("channel", 0));
    return cap;
}

Result<Pin> loadPin(const std::string& name, const json::Value& obj) {
    FieldReader r(obj, "pins." + name);
    Pin p;
    p.name = name;
    p.shareable = r.optBool("shareable", false);

    auto capsJson = r.optArray("capabilities");
    if (!capsJson) return capsJson.error();
    for (size_t i = 0; i < capsJson->size(); ++i) {
        auto cap = loadCapability((*capsJson)[i],
                                   "pins." + name + ".capabilities[" +
                                       std::to_string(i) + "]");
        if (!cap) return cap.error();
        p.capabilities.push_back(std::move(*cap));
    }
    return p;
}

Result<ToolchainSpec> loadToolchain(const json::Value& obj) {
    FieldReader r(obj, "toolchain");
    ToolchainSpec spec;
    auto name = r.requireString("name");
    if (!name) return name.error();
    spec.name = *name;

    auto ver = r.optString("version");
    if (!ver.empty()) {
        auto range = VersionRange::parse(ver);
        if (!range) return range.error();
        spec.version = *range;
    }
    spec.url = r.optString("url");
    return spec;
}

Result<FlashConfig> loadFlash(const json::Value& obj) {
    FieldReader r(obj, "flash");
    FlashConfig fc;
    auto tool = r.requireString("tool");
    if (!tool) return tool.error();
    fc.tool = *tool;

    auto argsJson = r.optArray("args");
    if (!argsJson) return argsJson.error();
    for (const auto& a : *argsJson) {
        auto s = a.asString();
        if (!s) {
            return Error(ErrorCode::InvalidField,
                         "flash.args 元素应为字符串");
        }
        fc.args.push_back(*s);
    }
    fc.baseAddress = r.optString("base_address");
    return fc;
}

} // namespace

Result<Chip> loadChip(const json::Value& json) {
    if (!json.isObject()) {
        return Error(ErrorCode::InvalidField, "芯片清单顶层必须是对象");
    }

    FieldReader r(json);

    Chip chip;

    auto name = r.requireString("name");
    if (!name) return name.error();
    chip.name = *name;

    chip.family = r.optString("family");
    chip.package = r.optString("package");
    chip.qemuMachine = r.optString("qemu_machine");

    // toolchain（必填）
    auto tcJson = r.requireObject("toolchain");
    if (!tcJson) return tcJson.error();
    auto tc = loadToolchain(*tcJson);
    if (!tc) return tc.error();
    chip.toolchain = std::move(*tc);

    // flash（必填）
    auto flashJson = r.requireObject("flash");
    if (!flashJson) return flashJson.error();
    auto fc = loadFlash(*flashJson);
    if (!fc) return fc.error();
    chip.flash = std::move(*fc);

    // pins（必填）
    auto pinsJson = r.requireObject("pins");
    if (!pinsJson) return pinsJson.error();
    FieldReader pr(*pinsJson, "pins");
    auto keys = pr.keys();
    if (!keys) return keys.error();
    for (const auto& pname : *keys) {
        auto obj = pinsJson->get(pname);
        if (!obj) return obj.error();
        if (!obj->isObject()) {
            return Error(ErrorCode::InvalidField,
                         "pins." + pname + " 必须是对象");
        }
        auto p = loadPin(pname, *obj);
        if (!p) return p.error();
        chip.pins.push_back(std::move(*p));
    }

    return chip;
}

Result<Chip> loadChipFile(const std::filesystem::path& path) {
    auto json = json::parseFile(path);
    if (!json) return json.error();
    return loadChip(*json);
}

} // namespace hpm::manifest