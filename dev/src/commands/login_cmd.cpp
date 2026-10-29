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

// dev/src/commands/login_cmd.cpp
#include "login_cmd.h"

#include "../dev_config.h"

#include <hpm/config.h>
#include <hpm/json/json_io.h>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <atomic>
#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>

namespace hpm::cli {

namespace {

void ok(const std::string& msg) {
    std::cout << "\033[32mok\033[0m " << msg << "\n";
}

void err(const std::string& msg) {
    std::cerr << "\033[31merror\033[0m " << msg << "\n";
}

void openBrowser(const std::string& url) {
#ifdef _WIN32
    std::system(("start \"\" \"" + url + "\"").c_str());
#elif __APPLE__
    std::system(("open \"" + url + "\"").c_str());
#else
    std::system(("xdg-open \"" + url + "\"").c_str());
#endif
}

// time_t → ISO 8601（UTC）
std::string toIso8601(std::time_t t) {
    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

// 用 httplib::Client 发 POST JSON
struct HttpResult {
    int status = 0;
    std::string body;
};

HttpResult httpPostJson(const std::string& url,
                         const std::string& body,
                         const std::string& authHeader = "") {
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

// 确认 y/n/e
enum class ConfirmResult { Yes, No, Edit };

ConfirmResult confirmWithEdit(const std::string& prompt) {
    std::cout << prompt << " [y/n/e] ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer == "y" || answer == "Y") return ConfirmResult::Yes;
    if (answer == "e" || answer == "E") return ConfirmResult::Edit;
    return ConfirmResult::No;
}

std::string promptInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

} // namespace

void LoginCmd::setup(CLI::App& app) {
    (void)app;
}

int LoginCmd::run(DevContext& ctx) {
    (void)ctx;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 1. 启动本地 HTTP 服务器
    int port = 30000 + (rand() % 10000);
    std::string code;
    std::mutex codeMutex;

    httplib::Server svr;
    svr.Get("/callback", [&](const httplib::Request& req, httplib::Response& res) {
        {
            std::lock_guard<std::mutex> lock(codeMutex);
            code = req.get_param_value("code");
        }
        res.set_content(
            "<html><head><meta charset=\"utf-8\"></head><body>"
            "<h1>登录成功</h1>"
            "<p>可以关闭此页面</p>"
            "<script>window.close();</script>"
            "</body></html>",
            "text/html; charset=utf-8");
        svr.stop();
    });

    std::string redirectUri = "http://localhost:" + std::to_string(port) + "/callback";
    std::string oauthUrl = std::string(HPM_DEV_CENTER_OAUTH_URL) +
                           "?app_id=" + HPM_DEV_CENTER_APP_ID +
                           "&redirect_uri=" + redirectUri;

    std::cout << "请在浏览器中完成授权...\n";
    std::cout << "如果浏览器没有自动打开，请访问：\n";
    std::cout << "  " << oauthUrl << "\n\n";

    openBrowser(oauthUrl);

    // 2. 等回调
    if (!svr.listen("localhost", port)) {
        err("无法启动本地 HTTP 服务器（端口 " + std::to_string(port) + " 被占用？）");
        return 1;
    }

    std::string codeCopy;
    {
        std::lock_guard<std::mutex> lock(codeMutex);
        codeCopy = code;
    }

    if (codeCopy.empty()) {
        err("未收到授权码");
        return 1;
    }

    // 3. 换 token
    auto tokenBody = json::Value::object();
    tokenBody.set("code", json::Value(codeCopy));
    tokenBody.set("appId", json::Value(HPM_DEV_CENTER_APP_ID));
    tokenBody.set("appKey", json::Value(HPM_DEV_CENTER_APP_KEY));

    auto tokenResp = httpPostJson(HPM_DEV_CENTER_TOKEN_API,
                                   json::dump(tokenBody, -1));
    if (tokenResp.status != 200) {
        err("获取 token 失败: HTTP " + std::to_string(tokenResp.status) +
            " " + tokenResp.body);
        return 1;
    }

    auto tokenJson = json::parse(tokenResp.body);
    if (!tokenJson) {
        err("token 响应解析失败");
        return 1;
    }

    std::string accessToken = tokenJson->getString("accessToken");
    int64_t expiresIn = tokenJson->getInt("expiresIn", 86400);

    if (accessToken.empty()) {
        err("accessToken 为空");
        return 1;
    }
    ok("已获取 accessToken");

    // 4. 拉用户信息
    auto userBody = json::Value::object();
    userBody.set("accessToken", json::Value(accessToken));
    userBody.set("action", json::Value("get_user"));

    auto userResp = httpPostJson(HPM_DEV_CENTER_API,
                                  json::dump(userBody, -1),
                                  "Authorization: Bearer " + std::string(HPM_DEV_CENTER_APP_KEY));
    if (userResp.status != 200) {
        err("拉用户信息失败: HTTP " + std::to_string(userResp.status) +
            " " + userResp.body);
        return 1;
    }

    auto userJson = json::parse(userResp.body);
    if (!userJson) {
        err("用户信息解析失败");
        return 1;
    }

    std::string name = userJson->getString("name");
    std::string email = userJson->getString("email");
    std::string githubUrl = userJson->getString("githubUrl");
    std::string repoUrl = userJson->getString("repoUrl");

    ok("用户: " + name + " (" + email + ")");

    // 5. 确认仓库地址
    std::string finalRepoUrl;
    if (!repoUrl.empty()) {
        auto r = confirmWithEdit("您的私域仓库地址是 " + repoUrl + "，确认吗？");
        if (r == ConfirmResult::No) {
            std::cout << "已取消\n";
            return 0;
        } else if (r == ConfirmResult::Yes) {
            finalRepoUrl = repoUrl;
        } else {
            finalRepoUrl = promptInput("请输入仓库地址：");
        }
    } else if (!githubUrl.empty()) {
        std::string defaultUrl = githubUrl + "/hpm-repo";
        auto r = confirmWithEdit("您绑定了 GitHub " + githubUrl +
                                  "，用 " + defaultUrl + " 作为仓库地址吗？");
        if (r == ConfirmResult::No) {
            std::cout << "已取消\n";
            return 0;
        } else if (r == ConfirmResult::Yes) {
            finalRepoUrl = defaultUrl;
        } else {
            finalRepoUrl = promptInput("请输入仓库地址：");
        }
    } else {
        finalRepoUrl = promptInput("请输入仓库地址：");
    }

    if (finalRepoUrl.empty()) {
        err("仓库地址不能为空");
        return 1;
    }
    ok("仓库地址: " + finalRepoUrl);

    // 6. 确认开发者名
    std::string finalDeveloper;
    auto r = confirmWithEdit("您的开发者名是 " + name + "，确认吗？");
    if (r == ConfirmResult::No) {
        std::cout << "已取消\n";
        return 0;
    } else if (r == ConfirmResult::Yes) {
        finalDeveloper = name;
    } else {
        finalDeveloper = promptInput("请输入开发者名（英文，如 zhangsan）：");
    }

    if (finalDeveloper.empty()) {
        err("开发者名不能为空");
        return 1;
    }
    ok("开发者名: " + finalDeveloper);

    // 7. 保存 dev.json
    DevConfig cfg;
    cfg.accessToken = accessToken;
    cfg.expiresAt = toIso8601(std::time(nullptr) + expiresIn);
    cfg.repoUrl = finalRepoUrl;
    cfg.githubUrl = githubUrl;
    cfg.developer = finalDeveloper;

    auto st = saveDevConfig(cfg);
    if (!st.ok()) {
        err("保存 dev.json 失败: " + st.error().message());
        return 1;
    }

    ok("已保存到 " + devConfigPath().string());
    return 0;
}

} // namespace hpm::cli