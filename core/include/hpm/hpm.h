/*
 * Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// core/include/hpm/hpm.h
#pragma once

// 公共 API 总入口
#include "hpm/result.h"
#include "hpm/types.h"

#define HPM_VERSION_MAJOR 0
#define HPM_VERSION_MINOR 1
#define HPM_VERSION_PATCH 0
#define HPM_VERSION_STRING "0.1.0"

namespace hpm {

const char* version() noexcept;

} // namespace hpm