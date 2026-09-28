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

// core/include/hpm/util/process.h
#pragma once

#include <hpm/result.h>

#include <string>
#include <vector>

namespace hpm::util {

// 运行外部命令，捕获输出
// - verbose 为 true 时把 stdout/stderr 直接透传到终端
// - 返回非零退出码 → Error
Status runProcess(const std::string& program,
                  const std::vector<std::string>& args,
                  bool verbose = false);

} // namespace hpm::util