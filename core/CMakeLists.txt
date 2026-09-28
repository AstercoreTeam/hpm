# core/CMakeLists.txt

add_library(hpm_core STATIC
    src/version.cpp
    src/model/version.cpp
    src/model/version_range.cpp
    src/json/json_value.cpp
    src/json/json_io.cpp
    src/manifest/field_reader.cpp
    src/manifest/project_loader.cpp
    src/manifest/driver_loader.cpp
    src/manifest/state_loader.cpp
    src/manifest/chip_loader.cpp
    src/capability/pin_allocator.cpp
    src/capability/pin_matcher.cpp
    src/capability/method_resolver.cpp
    src/capability/capability_checker.cpp
    src/resolver/id_generator.cpp
    src/resolver/resolver.cpp
)

target_include_directories(hpm_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hpm_core
    PUBLIC
        nlohmann_json::nlohmann_json
        fmt::fmt
)