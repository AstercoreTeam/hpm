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

#include "http.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace hpm::cli {

HttpResult httpPostJson(const std::string& url,
                        const std::string& body,
                        const std::string& authHeader) {
    HttpResult result;

    std::string s = url;
    bool isHttps = true;
    if (s.rfind("https://", 0) == 0) {
        s = s.substr(8);
    } else if (s.rfind("http://", 0) == 0) {
        s = s.substr(7);
        isHttps = false;
    }

    std::string host;
    std::string path;
    auto slash = s.find('/');
    if (slash == std::string::npos) {
        host = s;
        path = "/";
    } else {
        host = s.substr(0, slash);
        path = s.substr(slash);
    }

    httplib::Client cli(isHttps ? ("https://" + host) : ("http://" + host));
    cli.set_connection_timeout(30);
    cli.set_read_timeout(30);

    httplib::Headers headers;
    headers.emplace("Content-Type", "application/json");
    if (!authHeader.empty()) {
        auto colon = authHeader.find(':');
        if (colon != std::string::npos) {
            std::string key = authHeader.substr(0, colon);
            std::string value = authHeader.substr(colon + 1);
            while (!value.empty() && value.front() == ' ') value.erase(value.begin());
            headers.emplace(key, value);
        }
    }

    auto res = cli.Post(path.c_str(), headers, body, "application/json");
    if (!res) {
        result.status = 0;
        result.body = "HTTP 请求失败: " + httplib::to_string(res.error());
        return result;
    }

    result.status = res->status;
    result.body = res->body;
    return result;
}

} // namespace hpm::cli