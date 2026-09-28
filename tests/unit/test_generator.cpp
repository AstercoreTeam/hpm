// tests/unit/test_generator.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/generator/generator.h>
#include <hpm/generator/template_engine.h>
#include <hpm/json/json_io.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/manifest/state_loader.h>

#include <filesystem>
#include <fstream>

using namespace hpm;
using namespace hpm::manifest;
namespace fs = std::filesystem;

TEST_CASE("template engine basic", "[generator]") {
    std::map<std::string, std::string> vars = {
        {"DEVICE_NAME", "door_reader"},
        {"bus", "__spi_a3f2"},
    };
    auto out = renderTemplate(
        "int {{DEVICE_NAME}}_init(void);  // bus={{bus}}", vars);
    REQUIRE(out == "int door_reader_init(void);  // bus=__spi_a3f2");
}

TEST_CASE("template engine unknown key", "[generator]") {
    std::map<std::string, std::string> vars = {{"a", "1"}};
    auto out = renderTemplate("{{a}} {{b}}", vars);
    REQUIRE(out == "1 {{b}}");
}

namespace {

DriverLibrary makeLib() {
    DriverLibrary lib;
    {
        const char* text = R"({
            "name": "rc522",
            "version": "1.2.0",
            "provides": { "methods": ["init", "read_card"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    {
        const char* text = R"({
            "name": "spi",
            "version": "1.0.0",
            "provides": { "methods": ["transfer"] }
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

// 造一个假的驱动包缓存目录
void setupFakePackages(const fs::path& root) {
    // rc522-1.2.0
    {
        auto d = root / "rc522-1.2.0" / "src";
        fs::create_directories(d);
        std::ofstream(d / "rc522.c") << "// rc522 driver\n";
        auto inc = root / "rc522-1.2.0" / "include";
        fs::create_directories(inc);
        std::ofstream(inc / "rc522.h") << "// rc522 header\n";
    }
    // spi-1.0.0
    {
        auto d = root / "spi-1.0.0" / "src";
        fs::create_directories(d);
        std::ofstream(d / "spi.c") << "// spi driver\n";
        auto inc = root / "spi-1.0.0" / "include";
        fs::create_directories(inc);
        std::ofstream(inc / "spi.h") << "// spi header\n";
    }
}

} // namespace

TEST_CASE("generator produces driver.h", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto tmp = fs::temp_directory_path() / "hpm_gen_test";
    fs::remove_all(tmp);
    setupFakePackages(tmp / "packages");

    Generator gen(state, lib, tmp / "packages", tmp / "driver");
    auto plan = gen.plan();
    REQUIRE(plan.ok());

    // 找到 driver.h
    bool foundDriverH = false;
    for (const auto& f : plan->files) {
        if (f.relativePath == "driver.h") {
            foundDriverH = true;
            REQUIRE(f.content.find("door_reader_init") != std::string::npos);
            REQUIRE(f.content.find("door_reader_read_card") != std::string::npos);
        }
    }
    REQUIRE(foundDriverH);

    // 找宏文件
    bool foundMacros = false;
    for (const auto& f : plan->files) {
        if (f.relativePath == "rc522/door_reader_macros.h") {
            foundMacros = true;
            REQUIRE(f.content.find("#define DEVICE_NAME  door_reader") != std::string::npos);
            REQUIRE(f.content.find("#define cs  PA4") != std::string::npos);
        }
    }
    REQUIRE(foundMacros);

    fs::remove_all(tmp);
}

TEST_CASE("generator writes files", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto tmp = fs::temp_directory_path() / "hpm_gen_write_test";
    fs::remove_all(tmp);
    setupFakePackages(tmp / "packages");

    Generator gen(state, lib, tmp / "packages", tmp / "driver");
    auto st = gen.generate();
    REQUIRE(st.ok());

    REQUIRE(fs::exists(tmp / "driver" / "driver.h"));
    REQUIRE(fs::exists(tmp / "driver" / "driver.c"));
    REQUIRE(fs::exists(tmp / "driver" / "Makefile"));
    REQUIRE(fs::exists(tmp / "driver" / "rc522" / "src" / "rc522.c"));
    REQUIRE(fs::exists(tmp / "driver" / "spi" / "src" / "spi.c"));

    fs::remove_all(tmp);
}

TEST_CASE("generator missing package", "[generator]") {
    auto lib = makeLib();
    auto state = makeState();
    auto tmp = fs::temp_directory_path() / "hpm_gen_missing_test";
    fs::remove_all(tmp);
    // 不建 packages

    Generator gen(state, lib, tmp / "packages", tmp / "driver");
    auto plan = gen.plan();
    REQUIRE_FALSE(plan.ok());
    REQUIRE(plan.error().code() == ErrorCode::NotFound);

    fs::remove_all(tmp);
}