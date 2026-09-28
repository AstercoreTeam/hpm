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

// tests/unit/test_version.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/hpm.h>
#include <hpm/model/version.h>
#include <hpm/model/version_range.h>

using namespace hpm;

TEST_CASE("hpm version string", "[version]") {
    REQUIRE(std::string(hpm::version()) == "0.1.0");
}

TEST_CASE("Version::parse basic", "[version]") {
    auto v = Version::parse("1.2.3");
    REQUIRE(v.ok());
    REQUIRE(v->major() == 1);
    REQUIRE(v->minor() == 2);
    REQUIRE(v->patch() == 3);
    REQUIRE_FALSE(v->isPrerelease());
    REQUIRE(v->toString() == "1.2.3");
}

TEST_CASE("Version::parse prerelease and build", "[version]") {
    auto v = Version::parse("1.2.3-alpha.1+build.5");
    REQUIRE(v.ok());
    REQUIRE(v->prerelease() == "alpha.1");
    REQUIRE(v->build() == "build.5");
    REQUIRE(v->isPrerelease());
    REQUIRE(v->toString() == "1.2.3-alpha.1+build.5");
}

TEST_CASE("Version::parse invalid", "[version]") {
    REQUIRE_FALSE(Version::parse("").ok());
    REQUIRE_FALSE(Version::parse("1.2").ok());
    REQUIRE_FALSE(Version::parse("1.2.x").ok());
    REQUIRE_FALSE(Version::parse("a.b.c").ok());
}

TEST_CASE("Version comparison", "[version]") {
    auto v1 = *Version::parse("1.2.3");
    auto v2 = *Version::parse("1.2.4");
    auto v3 = *Version::parse("1.3.0");
    auto v4 = *Version::parse("2.0.0");

    REQUIRE(v1 < v2);
    REQUIRE(v2 < v3);
    REQUIRE(v3 < v4);
    REQUIRE(v1 == *Version::parse("1.2.3"));
}

TEST_CASE("Version prerelease ordering", "[version]") {
    // 1.0.0-alpha < 1.0.0-alpha.1 < 1.0.0-beta < 1.0.0
    auto a  = *Version::parse("1.0.0-alpha");
    auto a1 = *Version::parse("1.0.0-alpha.1");
    auto b  = *Version::parse("1.0.0-beta");
    auto r  = *Version::parse("1.0.0");

    REQUIRE(a < a1);
    REQUIRE(a1 < b);
    REQUIRE(b < r);
}

TEST_CASE("Version build metadata ignored in comparison", "[version]") {
    auto a = *Version::parse("1.0.0+build.1");
    auto b = *Version::parse("1.0.0+build.2");
    REQUIRE(a == b);
}

TEST_CASE("VersionRange caret", "[version_range]") {
    auto r = *VersionRange::parse("^1.2.0");
    REQUIRE(r.matches(*Version::parse("1.2.0")));
    REQUIRE(r.matches(*Version::parse("1.2.5")));
    REQUIRE(r.matches(*Version::parse("1.9.9")));
    REQUIRE_FALSE(r.matches(*Version::parse("2.0.0")));
    REQUIRE_FALSE(r.matches(*Version::parse("1.1.9")));
}

TEST_CASE("VersionRange tilde", "[version_range]") {
    auto r = *VersionRange::parse("~1.2.0");
    REQUIRE(r.matches(*Version::parse("1.2.0")));
    REQUIRE(r.matches(*Version::parse("1.2.9")));
    REQUIRE_FALSE(r.matches(*Version::parse("1.3.0")));
}

TEST_CASE("VersionRange exact", "[version_range]") {
    auto r = *VersionRange::parse("1.2.3");
    REQUIRE(r.matches(*Version::parse("1.2.3")));
    REQUIRE_FALSE(r.matches(*Version::parse("1.2.4")));
}

TEST_CASE("VersionRange comparison operators", "[version_range]") {
    auto r = *VersionRange::parse(">=1.2.0 <2.0.0");
    REQUIRE(r.matches(*Version::parse("1.2.0")));
    REQUIRE(r.matches(*Version::parse("1.9.9")));
    REQUIRE_FALSE(r.matches(*Version::parse("2.0.0")));
    REQUIRE_FALSE(r.matches(*Version::parse("1.1.9")));
}

TEST_CASE("VersionRange wildcard", "[version_range]") {
    auto r = *VersionRange::parse("*");
    REQUIRE(r.matches(*Version::parse("0.0.1")));
    REQUIRE(r.matches(*Version::parse("99.99.99")));
}

TEST_CASE("VersionRange empty is wildcard", "[version_range]") {
    auto r = *VersionRange::parse("");
    REQUIRE(r.matches(*Version::parse("1.0.0")));
}