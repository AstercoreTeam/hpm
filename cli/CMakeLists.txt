# cli/CMakeLists.txt

add_executable(hpm
    src/main.cpp
)

target_link_libraries(hpm
    PRIVATE
        hpm_core
        CLI11::CLI11
)