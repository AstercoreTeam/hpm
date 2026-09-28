// tests/unit/test_resolver.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/capability/driver_library.h>
#include <hpm/json/json_io.h>
#include <hpm/manifest/driver_loader.h>
#include <hpm/manifest/project_loader.h>
#include <hpm/resolver/resolver.h>

using namespace hpm;
using namespace hpm::manifest;

namespace {

DriverLibrary makeLib() {
    DriverLibrary lib;

    {
        const char* text = R"({
            "name": "rc522",
            "version": "1.2.0",
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
            "provides": { "methods": ["transfer"] }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    return lib;
}

Project makeProject() {
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": {
                "driver": "rc522",
                "cs": "PA4",
                "rst": "PA3",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            }
        }
    })";
    return *loadProject(*json::parse(text));
}

} // namespace

TEST_CASE("resolve creates intermediate spi device", "[resolver]") {
    auto lib = makeLib();
    auto proj = makeProject();

    Resolver resolver(lib, proj);
    auto state = resolver.resolve();
    REQUIRE(state.ok());

    // 用户设备
    REQUIRE(state->userDevices.size() == 1);
    const auto& dr = state->userDevices[0];
    REQUIRE(dr.name == "door_reader");
    REQUIRE(dr.driver == "rc522");
    REQUIRE(dr.version.toString() == "1.2.0");

    // 用户设备保留 cs / rst，sck/miso/mosi 被转移
    REQUIRE(dr.bindings.count("cs") == 1);
    REQUIRE(dr.bindings.count("rst") == 1);
    REQUIRE(dr.bindings.count("bus") == 1);
    REQUIRE(dr.bindings.count("sck") == 0);
    REQUIRE(dr.bindings.count("miso") == 0);
    REQUIRE(dr.bindings.count("mosi") == 0);

    // bus 引用中间设备
    std::string busRef = dr.bindings.at("bus");
    REQUIRE(busRef.find("__spi_") == 0);

    // 中间设备
    REQUIRE(state->intermediateDevices.size() == 1);
    const auto& idev = state->intermediateDevices[0];
    REQUIRE(idev.driver == "spi");
    REQUIRE(idev.name == busRef);
    REQUIRE(idev.bindings.at("sck") == "PA5");
    REQUIRE(idev.bindings.at("miso") == "PA6");
    REQUIRE(idev.bindings.at("mosi") == "PA7");
}

TEST_CASE("resolve reuses existing intermediate id", "[resolver]") {
    auto lib = makeLib();
    auto proj = makeProject();

    // 第一次解析
    Resolver r1(lib, proj);
    auto state1 = r1.resolve();
    REQUIRE(state1.ok());
    std::string id1 = state1->intermediateDevices[0].name;

    // 第二次解析，传入第一次的状态
    Resolver r2(lib, proj, &*state1);
    auto state2 = r2.resolve();
    REQUIRE(state2.ok());
    std::string id2 = state2->intermediateDevices[0].name;

    // ID 不变
    REQUIRE(id1 == id2);
}

TEST_CASE("resolve missing required pin", "[resolver]") {
    auto lib = makeLib();

    // 缺 sck
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": {
                "driver": "rc522",
                "cs": "PA4",
                "miso": "PA6",
                "mosi": "PA7"
            }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    Resolver resolver(lib, proj);
    auto state = resolver.resolve();
    REQUIRE_FALSE(state.ok());
    REQUIRE(state.error().code() == ErrorCode::MissingField);
}

TEST_CASE("resolve cyclic dependency", "[resolver]") {
    DriverLibrary lib;

    // a 依赖 b，b 依赖 a
    {
        const char* text = R"({
            "name": "a",
            "version": "1.0.0",
            "requires": { "sub": { "type": "b" } }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }
    {
        const char* text = R"({
            "name": "b",
            "version": "1.0.0",
            "requires": { "sub": { "type": "a" } }
        })";
        lib.add(*loadDriverManifest(*json::parse(text)));
    }

    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "dev": { "driver": "a" }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    Resolver resolver(lib, proj);
    auto state = resolver.resolve();
    REQUIRE_FALSE(state.ok());
    REQUIRE(state.error().code() == ErrorCode::CyclicDependency);
}

TEST_CASE("resolve optional missing is ok", "[resolver]") {
    auto lib = makeLib();

    // 不写 rst（可选）
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": {
                "driver": "rc522",
                "cs": "PA4",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    Resolver resolver(lib, proj);
    auto state = resolver.resolve();
    REQUIRE(state.ok());
    REQUIRE(state->userDevices[0].bindings.count("rst") == 0);
}

TEST_CASE("resolve shares intermediate across devices", "[resolver]") {
    auto lib = makeLib();

    // 两个设备共用同一组 SPI 引脚
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "dev1": {
                "driver": "rc522",
                "cs": "PA4",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            },
            "dev2": {
                "driver": "rc522",
                "cs": "PA8",
                "sck": "PA5",
                "miso": "PA6",
                "mosi": "PA7"
            }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    Resolver resolver(lib, proj);
    auto state = resolver.resolve();
    REQUIRE(state.ok());

    REQUIRE(state->userDevices.size() == 2);
    REQUIRE(state->intermediateDevices.size() == 1);

    std::string bus1 = state->userDevices[0].bindings.at("bus");
    std::string bus2 = state->userDevices[1].bindings.at("bus");
    REQUIRE(bus1 == bus2);
}

TEST_CASE("resolve name conflict on same pin key", "[resolver]") {
    auto lib = makeLib();

    // rc522 要 sck（没有，这里假设），spi 也要 sck，冲突
    // 实际 rc522 不要 sck，我们测一个假想场景：
    // 用户设备同时被两个驱动用同一个 key
    // 先简化：测 spi 和另一个驱动都要 sck 的场景
    DriverLibrary lib2;
    {
        const char* text = R"({
            "name": "top",
            "version": "1.0.0",
            "requires": {
                "bus1": { "type": "spi" },
                "bus2": { "type": "spi" }
            }
        })";
        lib2.add(*loadDriverManifest(*json::parse(text)));
    }
    {
        const char* text = R"({
            "name": "spi",
            "version": "1.0.0",
            "requires": { "sck": { "type": "spi_sck" } },
            "provides": { "methods": ["transfer"] }
        })";
        lib2.add(*loadDriverManifest(*json::parse(text)));
    }

    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "dev": { "driver": "top", "sck": "PA5" }
        }
    })";
    auto proj = *loadProject(*json::parse(text));

    Resolver resolver(lib2, proj);
    auto state = resolver.resolve();
    REQUIRE_FALSE(state.ok());
    REQUIRE(state.error().code() == ErrorCode::NameConflict);
}