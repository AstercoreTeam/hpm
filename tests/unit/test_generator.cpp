// tests/unit/test_generator.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/generator/generator.h>
#include <hpm/json/json_io.h>
#include <hpm/manifest/driver_loader.h>

#include <filesystem>
#include <fstream>

using namespace hpm;
using namespace hpm::manifest;
namespace fs = std::filesystem;

namespace {

DriverLibrary makeLib() {
    DriverLibrary lib;
    {
        const char* text = R"({
            "name": "rc522",
            "version": "1.2.0",
            "requires": {
                "bus": { "type": "spi" },
                "cs":  { "type": "gpio_out" },
                "rst": { "type": "gpio_out", "optional": true }
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
            "provides": { "methods": ["init", "transfer"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    {
        const char* text = R"({
            "name": "stm32f103c8t6",
            "version": "1.0.0",
            "provides": { "methods": ["init"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    return lib;
}

DriverState makeState() {
    DriverState s;
    Device dr;
    dr.name = "door_reader";
    dr.driver = "rc522";
    dr.version = *Version::parse("1.2.0");
    dr.bindings["bus"] = "__spi_a3f2";
    dr.bindings["cs"] = "PA4";
    dr.bindings["rst"] = "PA3";
    s.userDevices.push_back(dr);

    IntermediateDevice idev;
    idev.id = "__spi_a3f2";
    idev.name = "__spi_a3f2";
    idev.driver = "spi";
    idev.version = *Version::parse("1.0.0");
    idev.bindings["sck"] = "PA5";
    idev.bindings["miso"] = "PA6";
    idev.bindings["mosi"] = "PA7";
    s.intermediateDevices.push_back(idev);

    return s;
}

Project makeProject() {
    Project p;
    p.mcu = "stm32f103c8t6";
    return p;
}

// 造假的驱动包
void setupFakePackages(const fs::path& root) {
    {
        auto d = root / "rc522-1.2.0";
        fs::create_directories(d / "src");
        fs::create_directories(d / "include");
        std::ofstream(d / "src" / "rc522.c") << "// rc522 driver\n";
        std::ofstream(d / "include" / "rc522.h") << "// rc522 header\n";
    }
    {
        auto d = root / "spi-1.0.0";
        fs::create_directories(d / "src");
        fs::create_directories(d / "include");
        std::ofstream(d / "src" / "spi.c") << "// spi driver\n";
        std::ofstream(d / "include" / "spi.h") << "// spi header\n";
    }
    {
        auto d = root / "stm32f103c8t6-1.0.0";
        fs::create_directories(d / "src");
        fs::create_directories(d / "include");
        std::ofstream(d / "src" / "stm32f103c8t6.c") << "// chip driver\n";
        std::ofstream(d / "include" / "stm32f103c8t6.h") << "// chip header\n";
    }
}

} // namespace

TEST_CASE("generator creates per-instance dirs", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto proj = makeProject();
    auto tmp = fs::temp_directory_path() / "hpm_gen_test";
    fs::remove_all(tmp);
    setupFakePackages(tmp / "packages");

    Generator gen(state, lib, proj, tmp / "packages", tmp / "driver",
                  "stm32f103c8t6");
    auto plan = gen.plan();
    REQUIRE(plan.ok());

    // 检查实例目录和 hpm_dev.h
    bool foundDoorReaderMacro = false;
    for (const auto& f : plan->files) {
        if (f.relativePath == "door_reader/hpm_dev.h") {
            foundDoorReaderMacro = true;
            REQUIRE(f.content.find("#define DEVICE_NAME  door_reader") != std::string::npos);
            REQUIRE(f.content.find("#define cs  hpm_pin_PA4") != std::string::npos);
            REQUIRE(f.content.find("#define rst  hpm_pin_PA3") != std::string::npos);
            REQUIRE(f.content.find("#define bus  __spi_a3f2") != std::string::npos);
        }
    }
    REQUIRE(foundDoorReaderMacro);

    bool foundSpiMacro = false;
    for (const auto& f : plan->files) {
        if (f.relativePath == "__spi_a3f2/hpm_dev.h") {
            foundSpiMacro = true;
            REQUIRE(f.content.find("#define DEVICE_NAME  __spi_a3f2") != std::string::npos);
            REQUIRE(f.content.find("#define sck  hpm_pin_PA5") != std::string::npos);
            REQUIRE(f.content.find("#define miso  hpm_pin_PA6") != std::string::npos);
            REQUIRE(f.content.find("#define mosi  hpm_pin_PA7") != std::string::npos);
        }
    }
    REQUIRE(foundSpiMacro);

    // 驱动源码被拷贝到实例目录
    bool foundRc522Src = false;
    for (const auto& f : plan->files) {
        if (f.relativePath == "door_reader/rc522.c") {
            foundRc522Src = true;
        }
    }
    REQUIRE(foundRc522Src);

    // 芯片驱动目录
    bool foundChip = false;
    for (const auto& f : plan->files) {
        if (f.relativePath == "stm32f103c8t6/stm32f103c8t6.c") {
            foundChip = true;
        }
    }
    REQUIRE(foundChip);

    fs::remove_all(tmp);
}

TEST_CASE("generator driver.h has user API", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto proj = makeProject();
    auto tmp = fs::temp_directory_path() / "hpm_gen_api_test";
    fs::remove_all(tmp);
    setupFakePackages(tmp / "packages");

    Generator gen(state, lib, proj, tmp / "packages", tmp / "driver",
                  "stm32f103c8t6");
    auto plan = gen.plan();
    REQUIRE(plan.ok());

    for (const auto& f : plan->files) {
        if (f.relativePath == "driver.h") {
            REQUIRE(f.content.find("door_reader_init") != std::string::npos);
            REQUIRE(f.content.find("door_reader_read_card") != std::string::npos);
        }
    }

    fs::remove_all(tmp);
}

TEST_CASE("generator writes files to disk", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto proj = makeProject();
    auto tmp = fs::temp_directory_path() / "hpm_gen_write_test";
    fs::remove_all(tmp);
    setupFakePackages(tmp / "packages");

    Generator gen(state, lib, proj, tmp / "packages", tmp / "driver",
                  "stm32f103c8t6");
    auto st = gen.generate();
    REQUIRE(st.ok());

    REQUIRE(fs::exists(tmp / "driver" / "driver.h"));
    REQUIRE(fs::exists(tmp / "driver" / "driver.c"));
    REQUIRE(fs::exists(tmp / "driver" / "Makefile"));
    REQUIRE(fs::exists(tmp / "driver" / "door_reader" / "hpm_dev.h"));
    REQUIRE(fs::exists(tmp / "driver" / "door_reader" / "rc522.c"));
    REQUIRE(fs::exists(tmp / "driver" / "__spi_a3f2" / "hpm_dev.h"));
    REQUIRE(fs::exists(tmp / "driver" / "__spi_a3f2" / "spi.c"));
    REQUIRE(fs::exists(tmp / "driver" / "stm32f103c8t6" / "stm32f103c8t6.c"));

    fs::remove_all(tmp);
}

TEST_CASE("generator missing package", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto proj = makeProject();
    auto tmp = fs::temp_directory_path() / "hpm_gen_missing_test";
    fs::remove_all(tmp);
    // 不建 packages

    Generator gen(state, lib, proj, tmp / "packages", tmp / "driver",
                  "stm32f103c8t6");
    auto plan = gen.plan();
    REQUIRE_FALSE(plan.ok());
    REQUIRE(plan.error().code() == ErrorCode::NotFound);

    fs::remove_all(tmp);
}