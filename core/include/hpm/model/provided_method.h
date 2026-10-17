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

// core/include/hpm/model/provided_method.h
#pragma once

#include <hpm/types.h>

#include <string>
#include <vector>

namespace hpm {

struct MethodParam {
    std::string type;   // "const unsigned char*"
    std::string name;   // "tx"
};

// 驱动 provides 里的一个方法
struct ProvidedMethod {
    MethodName name;
    std::string returnType = "int";
    std::vector<MethodParam> params;

    // 生成参数列表字符串： "(const unsigned char* tx, unsigned char* rx, size_t len)"
    std::string paramList() const {
        if (params.empty()) return "(void)";
        std::string out = "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) out += ", ";
            out += params[i].type;
            if (!params[i].name.empty()) {
                out += " " + params[i].name;
            }
        }
        out += ")";
        return out;
    }

    // 生成完整声明
    std::string declaration(const std::string& prefix) const {
        return returnType + " " + prefix + name + paramList() + ";";
    }
};

} // namespace hpm