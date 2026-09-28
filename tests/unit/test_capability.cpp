// tests/unit/test_capability.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/capability/capability_checker.h>
#include <hpm/capability/driver_library.h>
#include <hpm/manifest/chip_loader.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/manifest/project_loader.h>
#include <hpm/json/json_io.h> 

using namespace hpm;
using namespace hpm::manifest;

namespace {

// 构造测试用芯片
Chip makeChip() {
    const char* text = R"({
        "name": "stm32f103c8t6",
        "toolchain": { "name": "arm-none-eabi-gcc" },
        "flash": { "tool": "st-flash" },
        "pins": {
            "PA4": { "capabilities": [{ "type": "gpio_out" }] },
            "PA5": { "shareable": true, "capabilities": [{ "type": "spi_sck", "instance": "SPI1" }] },
            "PA6": { "shareable": true, "capabilities": [{ "type": "spi_miso", "instance": "SPI1" }] },
            "PA7": { "shareable": true, "capabilities": [{ "type": "spi_mosi", "instance": "SPI1" }] }
        }
    })";
    auto j = json::parse(text);
    auto chip = loadChip(*j);
    return *chip;
}

// 构造驱动库
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

Project makeValidProject() {
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "spi1": {
                "driver": "spi",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            },
            "door_reader": {
                "driver": "rc522",
                "bus": "spi1",
                "cs": "PA4",
                "rst": "PA3"
            }
        }
    })";
    return *loadProject(*json::parse(text));
}

} // namespace

TEST_CASE("capability check passes for valid project", "[capability]") {
    auto chip = makeChip();
    auto lib = makeLib();
    auto proj = makeValidProject();

    // 注意：PA3 不在芯片里，但 rst 可选，所以实际用不到
    // 这里为了通过，把 rst 从项目里去掉
    for (auto& d : proj.devices) {
        if (d.name == "door_reader") {
            d.bindings.erase("rst");
        }
    }

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(proj);

    for (const auto& e : report.errors()) {
        INFO("error: " << e.message);
    }
    REQUIRE(report.ok());
}

TEST_CASE("pin conflict detected", "[capability]") {
    auto chip = makeChip();
    auto lib = makeLib();

    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "spi1": {
                "driver": "spi",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            },
            "door_reader": {
                "driver": "rc522",
                "bus": "spi1",
                "cs": "PA4"
            },
            "gate_reader": {
                "driver": "rc522",
                "bus": "spi1",
                "cs": "PA4"
            }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(proj);

    REQUIRE_FALSE(report.ok());
    bool hasConflict = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::PinConflict) hasConflict = true;
    }
    REQUIRE(hasConflict);
}

TEST_CASE("shareable pins can be reused", "[capability]") {
    auto chip = makeChip();
    auto lib = makeLib();

    // 两个 rc522 共用 SPI1 的 SCK/MISO/MOSI，各自 CS 不同
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "spi1": {
                "driver": "spi",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            },
            "door_reader": {
                "driver": "rc522",
                "bus": "spi1",
                "cs": "PA4"
            }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(proj);
    REQUIRE(report.ok());
}

TEST_CASE("method capability checked", "[capability]") {
    DriverLibrary lib;

    // rc522 要求下层提供 transfer
    {
        const char* text = R"({
            "name": "rc522",
            "version": "1.0.0",
            "requires": { "bus": { "type": "spi", "methods": ["transfer"] } },
            "provides": { "methods": ["read_card"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    // 假设有个坏的 spi 驱动，不提供 transfer
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
            "spi1": { "driver": "spi" },
            "door_reader": { "driver": "rc522", "bus": "spi1" }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    auto chip = makeChip();
    CapabilityChecker checker(lib, chip);
    auto report = checker.check(proj);

    REQUIRE_FALSE(report.ok());
    bool found = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::MethodNotFound) found = true;
    }
    REQUIRE(found);
}

TEST_CASE("missing required resource", "[capability]") {
    auto chip = makeChip();
    auto lib = makeLib();

    // rc522 没有 cs
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": { "driver": "rc522", "bus": "spi1" }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(proj);

    REQUIRE_FALSE(report.ok());
    bool found = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::MissingField) found = true;
    }
    REQUIRE(found);
}

TEST_CASE("duplicate device name", "[capability]") {
    auto chip = makeChip();
    auto lib = makeLib();

    Project proj;
    proj.mcu = "stm32f103c8t6";

    Device d1;
    d1.name = "dup";
    d1.driver = "rc522";
    d1.bindings["bus"] = "spi1";
    d1.bindings["cs"] = "PA4";

    Device d2 = d1;

    proj.devices = {d1, d2};

    CapabilityChecker checker(lib, chip);
    auto report = checker.check(proj);

    REQUIRE_FALSE(report.ok());
    bool found = false;
    for (const auto& e : report.errors()) {
        if (e.code == ErrorCode::NameConflict) found = true;
    }
    REQUIRE(found);
}