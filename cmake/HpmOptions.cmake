# hpm/cmake/HpmOptions.cmake

option(HPM_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(HPM_ENABLE_SANITIZERS  "Enable ASan/UBSan (Debug only)" OFF)

# 通用警告
if(MSVC)
    add_compile_options(/W4 /permissive-)
    if(HPM_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
    if(HPM_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

# Sanitizer
if(HPM_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()