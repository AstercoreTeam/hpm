// tests/unit/test_json.cpp
#include <catch2/catch_test_macros.hpp>
#include <hpm/json/json_io.h>
#include <hpm/json/json_value.h>

#include <filesystem>

using namespace hpm;
using namespace hpm::json;

TEST_CASE("Value basic types", "[json]") {
    Value b(true);
    REQUIRE(b.isBool());
    REQUIRE(*b.asBool() == true);

    Value i(int64_t(42));
    REQUIRE(i.isInt());
    REQUIRE(*i.asInt() == 42);

    Value s(std::string("hello"));
    REQUIRE(s.isString());
    REQUIRE(*s.asString() == "hello");

    Value n;
    REQUIRE(n.isNull());
}

TEST_CASE("Value object", "[json]") {
    auto obj = Value::object();
    obj.set("name", Value(std::string("hpm")));
    obj.set("version", Value(int64_t(1)));

    REQUIRE(obj.has("name"));
    REQUIRE(obj.has("version"));
    REQUIRE_FALSE(obj.has("missing"));

    auto name = obj.get("name");
    REQUIRE(name.ok());
    REQUIRE(*name->asString() == "hpm");

    REQUIRE(obj.getString("name") == "hpm");
    REQUIRE(obj.getInt("version") == 1);
    REQUIRE(obj.getString("missing", "default") == "default");
}

TEST_CASE("Value array", "[json]") {
    auto arr = Value::array();
    arr.push(Value(int64_t(1)));
    arr.push(Value(int64_t(2)));
    arr.push(Value(int64_t(3)));

    REQUIRE(*arr.size() == 3);
    REQUIRE(*arr.at(0)->asInt() == 1);
    REQUIRE(*arr.at(2)->asInt() == 3);

    auto oob = arr.at(10);
    REQUIRE_FALSE(oob.ok());
    REQUIRE(oob.error().code() == ErrorCode::InvalidArgument);
}

TEST_CASE("Value missing field", "[json]") {
    auto obj = Value::object();
    auto r = obj.get("nope");
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code() == ErrorCode::MissingField);
}

TEST_CASE("Value type mismatch", "[json]") {
    Value s(std::string("text"));
    auto r = s.asInt();
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code() == ErrorCode::InvalidField);
}

TEST_CASE("parse and dump", "[json]") {
    const char* text = R"({
        "mcu": "stm32f103c8t6",
        "devices": {
            "door_reader": {
                "driver": "rc522",
                "cs": "PA4"
            }
        }
    })";

    auto v = parse(text);
    REQUIRE(v.ok());
    REQUIRE(v->getString("mcu") == "stm32f103c8t6");

    auto devices = v->get("devices");
    REQUIRE(devices.ok());
    REQUIRE(devices->has("door_reader"));

    auto reader = devices->get("door_reader");
    REQUIRE(reader.ok());
    REQUIRE(reader->getString("driver") == "rc522");
    REQUIRE(reader->getString("cs") == "PA4");
}

TEST_CASE("parse error", "[json]") {
    auto v = parse("{not valid json}");
    REQUIRE_FALSE(v.ok());
    REQUIRE(v.error().code() == ErrorCode::ManifestParseError);
}

TEST_CASE("write and read file", "[json]") {
    auto tmp = std::filesystem::temp_directory_path() / "hpm_test.json";

    auto obj = Value::object();
    obj.set("name", Value(std::string("test")));
    obj.set("count", Value(int64_t(5)));

    REQUIRE(writeFile(obj, tmp).ok());

    auto loaded = parseFile(tmp);
    REQUIRE(loaded.ok());
    REQUIRE(loaded->getString("name") == "test");
    REQUIRE(loaded->getInt("count") == 5);

    std::filesystem::remove(tmp);
}

TEST_CASE("parseFile missing", "[json]") {
    auto r = parseFile("/nonexistent/path/xxx.json");
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code() == ErrorCode::IoError);
}