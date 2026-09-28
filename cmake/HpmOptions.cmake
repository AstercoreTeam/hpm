# Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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