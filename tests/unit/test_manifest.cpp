// tests/unit/test_manifest.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/json/json_io.h>
#include <hpm/manifest/chip_loader.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/manifest/project_loader.h>
#include <hpm/manifest/state_loader.h>

using namespace hpm;
using namespace hpm::manifest;

TEST_CASE("loadProject basic", "[manifest]") {
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

    auto j = json::parse(text);
    REQUIRE(j.ok());

    auto proj = loadProject(*j);
    REQUIRE(proj.ok());
    REQUIRE(proj->mcu == "stm32f103c8t6");
    REQUIRE(proj->devices.size() == 2);

    auto* dr = proj->findDevice("door_reader");
    REQUIRE(dr != nullptr);
    REQUIRE(dr->driver == "rc522");
    REQUIRE(dr->bindings.at("bus") == "spi1");
    REQUIRE(dr->bindings.at("cs") == "PA4");
    REQUIRE(dr->bindings.at("rst") == "PA3");

    auto* spi = proj->findDevice("spi1");
    REQUIRE(spi != nullptr);
    REQUIRE(spi->bindings.at("sck") == "PA5");
}

TEST_CASE("loadProject missing mcu", "[manifest]") {
    const char* text = R"({ "devices": {} })";
    auto j = json::parse(text);
    REQUIRE(j.ok());
    auto proj = loadProject(*j);
    REQUIRE_FALSE(proj.ok());
    REQUIRE(proj.error().code() == ErrorCode::MissingField);
}

TEST_CASE("loadProject missing driver", "[manifest]") {
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "bad": { "cs": "PA4" }
        }
    })";
    auto j = json::parse(text);
    auto proj = loadProject(*j);
    REQUIRE_FALSE(proj.ok());
    REQUIRE(proj.error().code() == ErrorCode::MissingField);
}

TEST_CASE("loadDriverManifest basic", "[manifest]") {
    const char* text = R"({
        "name": "rc522",
        "version": "1.2.0",
        "description": "RFID reader",
        "requires": {
            "bus": { "type": "spi", "methods": ["transfer"] },
            "cs":  { "type": "gpio_out", "methods": ["write"] },
            "rst": { "type": "gpio_out", "methods": ["write"], "optional": true }
        },
        "provides": {
            "methods": ["init", "read_card", "write_block"]
        }
    })";

    auto j = json::parse(text);
    REQUIRE(j.ok());

    auto dm = loadDriverManifest(*j);
    REQUIRE(dm.ok());
    REQUIRE(dm->name == "rc522");
    REQUIRE(dm->version.toString() == "1.2.0");
    REQUIRE(dm->requirements.size() == 3);

    auto* bus = dm->findRequirement("bus");
    REQUIRE(bus != nullptr);
    REQUIRE(bus->type == "spi");
    REQUIRE(bus->methods.size() == 1);
    REQUIRE(bus->methods[0] == "transfer");
    REQUIRE_FALSE(bus->optional);

    auto* rst = dm->findRequirement("rst");
    REQUIRE(rst != nullptr);
    REQUIRE(rst->optional);

    REQUIRE(dm->provides.size() == 3);
    REQUIRE(dm->providesMethod("read_card"));
    REQUIRE_FALSE(dm->providesMethod("nope"));
}

TEST_CASE("loadDriverManifest invalid version", "[manifest]") {
    const char* text = R"({
        "name": "bad",
        "version": "not-a-version"
    })";
    auto j = json::parse(text);
    auto dm = loadDriverManifest(*j);
    REQUIRE_FALSE(dm.ok());
}

TEST_CASE("loadChip basic", "[manifest]") {
    const char* text = R"({
        "name": "stm32f103c8t6",
        "family": "stm32f1",
        "package": "LQFP48",
        "qemu_machine": "stm32vldiscovery",
        "toolchain": {
            "name": "arm-none-eabi-gcc",
            "version": "^12.0.0"
        },
        "flash": {
            "tool": "st-flash",
            "args": ["write", "{bin}", "0x8000000"],
            "base_address": "0x8000000"
        },
        "pins": {
            "PA5": {
                "shareable": true,
                "capabilities": [
                    { "type": "spi_sck", "instance": "SPI1" },
                    { "type": "gpio_out" }
                ]
            },
            "PA4": {
                "capabilities": [
                    { "type": "gpio_out" }
                ]
            }
        }
    })";

    auto j = json::parse(text);
    REQUIRE(j.ok());

    auto chip = loadChip(*j);
    REQUIRE(chip.ok());
    REQUIRE(chip->name == "stm32f103c8t6");
    REQUIRE(chip->pins.size() == 2);
    REQUIRE(chip->toolchain.name == "arm-none-eabi-gcc");

    auto* pa5 = chip->findPin("PA5");
    REQUIRE(pa5 != nullptr);
    REQUIRE(pa5->shareable);
    REQUIRE(pa5->capabilities.size() == 2);
    REQUIRE(pa5->hasCapability("spi_sck", "SPI1"));

    auto* pa4 = chip->findPin("PA4");
    REQUIRE(pa4 != nullptr);
    REQUIRE_FALSE(pa4->shareable);
}

TEST_CASE("loadDriverState roundtrip", "[manifest]") {
    DriverState state;

    Device user;
    user.name = "door_reader";
    user.driver = "rc522";
    user.version = *Version::parse("1.2.0");
    user.bindings["bus"] = "__spi_a3f2";
    user.bindings["cs"] = "PA4";
    state.userDevices.push_back(user);

    IntermediateDevice inter;
    inter.id = "__spi_a3f2";
    inter.name = "spi_door";
    inter.driver = "spi";
    inter.version = *Version::parse("1.0.0");
    inter.bindings["sck"] = "PA5";
    inter.bindings["miso"] = "PA6";
    inter.bindings["mosi"] = "PA7";
    state.intermediateDevices.push_back(inter);

    // 序列化
    auto json = saveDriverState(state);
    REQUIRE(json.isObject());

    // 反序列化
    auto loaded = loadDriverState(json);
    REQUIRE(loaded.ok());
    REQUIRE(loaded->userDevices.size() == 1);
    REQUIRE(loaded->userDevices[0].name == "door_reader");
    REQUIRE(loaded->userDevices[0].bindings.at("cs") == "PA4");

    REQUIRE(loaded->intermediateDevices.size() == 1);
    REQUIRE(loaded->intermediateDevices[0].id == "__spi_a3f2");
    REQUIRE(loaded->intermediateDevices[0].name == "spi_door");
    REQUIRE(loaded->intermediateDevices[0].bindings.at("sck") == "PA5");
}