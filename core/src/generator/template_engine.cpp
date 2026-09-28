// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// core/src/generator/template_engine.cpp
#include <hpm/generator/template_engine.h>

namespace hpm {

std::string renderTemplate(
    const std::string& tmpl,
    const std::map<std::string, std::string>& vars) {
    std::string out;
    out.reserve(tmpl.size());

    size_t i = 0;
    while (i < tmpl.size()) {
        if (i + 1 < tmpl.size() && tmpl[i] == '{' && tmpl[i + 1] == '{') {
            auto end = tmpl.find("}}", i + 2);
            if (end == std::string::npos) {
                out += tmpl.substr(i);
                break;
            }
            std::string key = tmpl.substr(i + 2, end - i - 2);
            // trim
            while (!key.empty() && std::isspace(static_cast<unsigned char>(key.front())))
                key.erase(key.begin());
            while (!key.empty() && std::isspace(static_cast<unsigned char>(key.back())))
                key.pop_back();

            auto it = vars.find(key);
            if (it != vars.end()) {
                out += it->second;
            } else {
                out += "{{" + key + "}}";
            }
            i = end + 2;
        } else {
            out += tmpl[i++];
        }
    }
    return out;
}

} // namespace hpm