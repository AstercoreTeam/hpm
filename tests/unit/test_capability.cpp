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

// tests/unit/test_capability.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/capability/capability_checker.h>
#include <hpm/capability/driver_library.h>
#include <hpm/json/json_io.h>
#include <hpm/manifest/chip_loader.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/manifest/project_loader.h>
#include <hpm/resolver/resolver.h>

using namespace hpm;
using namespace hpm::manifest;

namespace {

// ─────────────── 芯片 ───────────────

Chip makeChip() {
    const char* text = R"({
        "name": "stm32f103c8t6",
        "toolchain": { "name": "arm-none-eabi-gcc" },
        "flash": { "tool": "st-flash" },
        "pins": {
            "PA3": { "capabilities": [{ "type": "gpio_out" }] },
            "PA4": { "capabilities": [{ "type": "gpio_out" }] },
            "PA5": { "shareable": true, "capabilities": [{ "type": "spi_sck", "instance": "SPI1" }] },
            "PA6": { "shareable": true, "capabilities": [{ "type": "spi_miso", "instance": "SPI1" }] },
            "PA7": { "shareable": true, "capabilities": [{ "type": "spi_mosi", "instance": "SPI1" }] },
            "PA8": { "capabilities": [{ "type": "gpio_out" }] }
        }
    })";
    auto j = json::parse(text);
    return *loadChip(*j);
}

// ─────────────── 驱动库 ───────────────

DriverLibrary makeLib() {
    DriverLibrary lib;

    {
        const char* text = R"({
            "name": "rc522",
            "version": "1.0.0",
            "requires": {
                "bus": { "type": "spi", "methods": ["transfer"] },
                "cs":  { "type": "gpio_out", "methods": ["write"] },
                "rst": { "type": "gpio_out", "methods": ["write"], "optional": true }
            },
            "provides": { "methods": ["init", "read_card"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    {
        const char* text = R"({
            "name": "spi",
            "version": "1.0.0",
            "requires": {
                "sck":  { "type": "spi_sck" },
                "miso": { "type": "spi_miso" },
                "mosi": { "type": "spi_mosi" }
            },
            "provides": { "methods": ["transfer", "set_speed"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    return lib;
}

// ─────────────── 构造展开后的 DriverState ───────────────

DriverState makeValidState() {
    DriverState s;

    Device dr;
    dr.name = "door_reader";
    dr.driver = "rc522";
    dr.version = *Version::parse("1.0.0");
    dr.bindings["bus"] = "__spi_a3f2";
    dr.bindings["cs"]  = "PA4";
    s.userDevices.push_back(dr);

    IntermediateDevice idev;
    idev.id = "__spi_a3f2";
    idev.name = "__spi_a3f2";
    idev.driver = "spi";
    idev.version = *Version::parse("1.0.0");
    idev.bindings["sck"]  = "PA5";
    idev.bindings["miso"] = "PA6";
    idev.bindings["mosi"] = "PA7";
    s.intermediateDevices.push_back(idev);

    return s;
}

// 从 project.json 造 Project，再用 Resolver 展开
Result<DriverState> makeStateFromProject(const char* json_text,
                                          const DriverLibrary& lib) {
    auto j = json::parse(json_text);
    if (!j) return j.error();
    auto proj = loadProject(*j);
    if (!proj) return proj.error();
    Resolver resolver(lib, *proj);
    return resolver.resolve();
}

} // namespace

// ─────────────────────────────────────────────
// 测试
// ─────────────────────────────────────────────

TEST_CASE("capability check passes for valid state", "[capability]") {
    auto chip = makeChip();
    auto lib  = makeLib();
    auto st   = makeValidState();

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(st);

    for (const auto& e : report.errors()) {
        INFO("error: " << e.message);
    }
    REQUIRE(report.ok());
}

TEST_CASE("pin conflict detected", "[capability]") {
    auto chip = makeChip();
    auto lib  = makeLib();

    // 两个 rc522 都用 PA4 做 cs
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": {
                "driver": "rc522",
                "cs": "PA4",
                "sck": "PA5", "miso": "PA6", "mosi": "PA7"
            },
            "gate_reader": {
                "driver": "rc522",
                "cs": "PA4",
                "sck": "PA5", "miso": "PA6", "mosi": "PA7"
            }
        }
    })";
    auto st = makeStateFromProject(text, lib);
    REQUIRE(st.ok());

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(*st);

    REQUIRE_FALSE(report.ok());
    bool hasConflict = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::PinConflict) hasConflict = true;
    }
    REQUIRE(hasConflict);
}

TEST_CASE("shareable pins can be reused", "[capability]") {
    auto chip = makeChip();
    auto lib  = makeLib();

    // 两个 rc522 共用 SPI，CS 不同
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": {
                "driver": "rc522",
                "cs": "PA4",
                "sck": "PA5", "miso": "PA6", "mosi": "PA7"
            },
            "gate_reader": {
                "driver": "rc522",
                "cs": "PA8",
                "sck": "PA5", "miso": "PA6", "mosi": "PA7"
            }
        }
    })";
    auto st = makeStateFromProject(text, lib);
    REQUIRE(st.ok());

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(*st);

    for (const auto& e : report.errors()) {
        INFO("error: " << e.message);
    }
    REQUIRE(report.ok());
}

TEST_CASE("method capability checked", "[capability]") {
    DriverLibrary lib;

    // rc522 要 bus.transfer
    {
        const char* text = R"({
            "name": "rc522",
            "version": "1.0.0",
            "requires": { "bus": { "type": "spi", "methods": ["transfer"] } },
            "provides": { "methods": ["read_card"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    // 坏 spi：不提供 transfer
    {
        const char* text = R"({
            "name": "spi",
            "version": "1.0.0",
            "provides": { "methods": ["set_speed"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }

    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": { "driver": "rc522", "bus": "spi1" }
        }
    })";
    auto j = json::parse(text);
    auto proj = loadProject(*j);
    REQUIRE(proj.ok());

    // 直接用 Project 检查（旧重载），bus 当引脚处理不了，我们改用 DriverState
    DriverState st;
    Device dr;
    dr.name = "door_reader";
    dr.driver = "rc522";
    dr.bindings["bus"] = "__spi_a3f2";
    st.userDevices.push_back(dr);

    IntermediateDevice idev;
    idev.id = "__spi_a3f2";
    idev.name = "__spi_a3f2";
    idev.driver = "spi";
    st.intermediateDevices.push_back(idev);

    auto chip = makeChip();
    CapabilityChecker checker(lib, chip);
    auto report = checker.check(st);

    REQUIRE_FALSE(report.ok());
    bool found = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::MethodNotFound) found = true;
    }
    REQUIRE(found);
}

TEST_CASE("missing required resource", "[capability]") {
    auto chip = makeChip();
    auto lib  = makeLib();

    // door_reader 有 bus，但没有 cs
    DriverState st;
    Device dr;
    dr.name = "door_reader";
    dr.driver = "rc522";
    dr.bindings["bus"] = "__spi_a3f2";
    st.userDevices.push_back(dr);

    IntermediateDevice idev;
    idev.id = "__spi_a3f2";
    idev.name = "__spi_a3f2";
    idev.driver = "spi";
    idev.bindings["sck"]  = "PA5";
    idev.bindings["miso"] = "PA6";
    idev.bindings["mosi"] = "PA7";
    st.intermediateDevices.push_back(idev);

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(st);

    REQUIRE_FALSE(report.ok());
    bool found = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::MissingField) found = true;
    }
    REQUIRE(found);
}

TEST_CASE("duplicate device name", "[capability]") {
    auto chip = makeChip();
    auto lib  = makeLib();

    DriverState st;

    Device d1;
    d1.name = "dup";
    d1.driver = "rc522";
    d1.bindings["bus"] = "__spi_a3f2";
    d1.bindings["cs"]  = "PA4";

    Device d2 = d1;

    st.userDevices = {d1, d2};

    IntermediateDevice idev;
    idev.id = "__spi_a3f2";
    idev.name = "__spi_a3f2";
    idev.driver = "spi";
    idev.bindings["sck"]  = "PA5";
    idev.bindings["miso"] = "PA6";
    idev.bindings["mosi"] = "PA7";
    st.intermediateDevices.push_back(idev);

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(st);

    REQUIRE_FALSE(report.ok());
    bool found = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::NameConflict) found = true;
    }
    REQUIRE(found);
}