// tests/unit/test_model.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/model/chip.h>
#include <hpm/model/device.h>
#include <hpm/model/driver_manifest.h>
#include <hpm/model/project.h>

using namespace hpm;

TEST_CASE("Pin capability check", "[model]") {
    Pin pa5;
    pa5.name = "PA5";
    pa5.shareable = true;
    pa5.capabilities.push_back({"spi_sck", "SPI1", 0});
    pa5.capabilities.push_back({"gpio_out", "", 0});

    REQUIRE(pa5.hasCapability("spi_sck", "SPI1"));
    REQUIRE(pa5.hasCapability("spi_sck"));            // 不指定 instance
    REQUIRE(pa5.hasCapability("gpio_out"));
    REQUIRE_FALSE(pa5.hasCapability("spi_sck", "SPI2")); // 实例不对
    REQUIRE_FALSE(pa5.hasCapability("uart_tx"));      // 没有这个能力
}

TEST_CASE("Requirement lookup", "[model]") {
    DriverManifest rc522;
    rc522.name = "rc522";
    rc522.version = *Version::parse("1.2.0");

    Requirement bus;
    bus.key = "bus";
    bus.type = "spi";
    bus.methods = {"transfer"};

    Requirement cs;
    cs.key = "cs";
    cs.type = "gpio_out";
    cs.methods = {"write"};

    Requirement rst;
    rst.key = "rst";
    rst.type = "gpio_out";
    rst.methods = {"write"};
    rst.optional = true;

    rc522.requirements = {bus, cs, rst};

    REQUIRE(rc522.findRequirement("bus") != nullptr);
    REQUIRE(rc522.findRequirement("bus")->type == "spi");
    REQUIRE(rc522.findRequirement("cs") != nullptr);
    REQUIRE(rc522.findRequirement("rst") != nullptr);
    REQUIRE(rc522.findRequirement("rst")->optional);
    REQUIRE(rc522.findRequirement("nope") == nullptr);
}

TEST_CASE("Provided method lookup", "[model]") {
    DriverManifest spi;
    spi.name = "spi";
    spi.provides = {
        {"transfer", "int(uint8_t*, uint8_t*, size_t)"},
        {"set_speed", "void(uint32_t)"},
    };

    REQUIRE(spi.providesMethod("transfer"));
    REQUIRE(spi.providesMethod("set_speed"));
    REQUIRE_FALSE(spi.providesMethod("read_card"));
}

TEST_CASE("Project device lookup", "[model]") {
    Project proj;
    proj.mcu = "stm32f103c8t6";

    Device d1;
    d1.name = "door_reader";
    d1.driver = "rc522";
    d1.bindings["bus"] = "spi1";
    d1.bindings["cs"] = "PA4";

    Device d2;
    d2.name = "gate_reader";
    d2.driver = "rc522";
    d2.bindings["bus"] = "spi1";
    d2.bindings["cs"] = "PA8";

    proj.devices = {d1, d2};

    REQUIRE(proj.findDevice("door_reader") != nullptr);
    REQUIRE(proj.findDevice("door_reader")->driver == "rc522");
    REQUIRE(proj.findDevice("gate_reader") != nullptr);
    REQUIRE(proj.findDevice("nope") == nullptr);

    auto* dev = proj.findDevice("door_reader");
    REQUIRE(dev->bindings.at("cs") == "PA4");
}

TEST_CASE("Chip pin lookup", "[model]") {
    Chip chip;
    chip.name = "stm32f103c8t6";
    chip.package = "LQFP48";

    Pin pa5;
    pa5.name = "PA5";
    pa5.shareable = true;
    pa5.capabilities.push_back({"spi_sck", "SPI1", 0});
    chip.pins.push_back(pa5);

    Pin pa4;
    pa4.name = "PA4";
    pa4.shareable = false;
    pa4.capabilities.push_back({"gpio_out", "", 0});
    chip.pins.push_back(pa4);

    REQUIRE(chip.findPin("PA5") != nullptr);
    REQUIRE(chip.findPin("PA5")->shareable);
    REQUIRE(chip.findPin("PA4") != nullptr);
    REQUIRE_FALSE(chip.findPin("PA4")->shareable);
    REQUIRE(chip.findPin("PB0") == nullptr);
}