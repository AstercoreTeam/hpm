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