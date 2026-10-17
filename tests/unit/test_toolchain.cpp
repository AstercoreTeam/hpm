// tests/unit/test_toolchain.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/model/toolchain_spec.h>

using namespace hpm;

TEST_CASE("toolchain prefix resolves", "[toolchain]") {
    ToolchainSpec s;
    s.prefix = "arm-none-eabi-";
    REQUIRE(s.resolveCc() == "arm-none-eabi-gcc");
    REQUIRE(s.resolveObjcopy() == "arm-none-eabi-objcopy");
    REQUIRE(s.resolveSize() == "arm-none-eabi-size");
}

TEST_CASE("toolchain explicit cc", "[toolchain]") {
    ToolchainSpec s;
    s.cc = "clang";
    s.objcopy = "llvm-objcopy";
    REQUIRE(s.resolveCc() == "clang");
    REQUIRE(s.resolveObjcopy() == "llvm-objcopy");
}

TEST_CASE("toolchain legacy name", "[toolchain]") {
    ToolchainSpec s;
    s.name = "arm-none-eabi-gcc";
    REQUIRE(s.resolveCc() == "arm-none-eabi-gcc");
    REQUIRE(s.resolveObjcopy() == "arm-none-eabi-objcopy");
}

TEST_CASE("toolchain empty fallback", "[toolchain]") {
    ToolchainSpec s;
    REQUIRE(s.resolveCc() == "gcc");
    REQUIRE(s.resolveObjcopy() == "objcopy");
}