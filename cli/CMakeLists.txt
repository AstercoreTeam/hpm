# cli/CMakeLists.txt

add_executable(hpm
    src/main.cpp
    src/cmd_context.cpp
    src/command_registry.cpp
    src/output.cpp
    src/commands/init_cmd.cpp
    src/commands/sync_cmd.cpp
    src/commands/build_cmd.cpp
    src/commands/flash_cmd.cpp
    src/commands/clean_cmd.cpp
    src/commands/dev_export_cmd.cpp
)

target_link_libraries(hpm
    PRIVATE
        hpm_core
        CLI11::CLI11
)