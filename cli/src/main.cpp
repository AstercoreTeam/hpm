// cli/src/main.cpp
#include <hpm/hpm.h>
#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"hpm - Hardware Package Manager"};
    app.set_version_flag("--version", hpm::version());

    if (argc == 1) {
        std::cout << app.help() << std::endl;
        return 0;
    }

    CLI11_PARSE(app, argc, argv);
    return 0;
}