# tests/CMakeLists.txt

add_executable(hpm_tests
    unit/test_version.cpp
    unit/test_json.cpp
    unit/test_model.cpp
    unit/test_manifest.cpp
    unit/test_capability.cpp
    unit/test_resolver.cpp
)

target_link_libraries(hpm_tests
    PRIVATE
        hpm_core
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(hpm_tests)