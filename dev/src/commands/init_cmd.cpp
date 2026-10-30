// dev/src/commands/init_cmd.cpp
#include "init_cmd.h"

#include "../dev_config.h"

#include <hpm/util/process.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <cctype>

namespace hpm::cli {

namespace fs = std::filesystem;

namespace {

void ok(const std::string& msg)   { std::cout << "\033[32mok\033[0m " << msg << "\n"; }
void err(const std::string& msg)  { std::cerr << "\033[31merror\033[0m " << msg << "\n"; }
void info(const std::string& msg) { std::cout << "\033[34minfo\033[0m " << msg << "\n"; }

bool isTokenExpired(const std::string& expiresAt) {
    return expiresAt.empty();
}

// raw 地址 -> SSH git 地址
std::string rawToSshUrl(const std::string& raw) {
    std::string url = raw;
    while (!url.empty() && url.back() == '/') url.pop_back();

    // GitHub raw
    const std::string ghPrefix = "https://raw.githubusercontent.com/";
    if (url.rfind(ghPrefix, 0) == 0) {
        std::string rest = url.substr(ghPrefix.size()); // owner/repo/ref/...
        auto p1 = rest.find('/');
        if (p1 != std::string::npos) {
            auto p2 = rest.find('/', p1 + 1);
            if (p2 != std::string::npos) {
                std::string owner = rest.substr(0, p1);
                std::string repo  = rest.substr(p1 + 1, p2 - p1 - 1);
                return "git@github.com:" + owner + "/" + repo + ".git";
            }
        }
        return raw;
    }

    // GitLab: .../-/raw/<ref>/...
    const std::string glMarker = "/-/raw/";
    auto glPos = url.find(glMarker);
    if (url.rfind("https://gitlab.com/", 0) == 0 && glPos != std::string::npos) {
        std::string path = url.substr(std::string("https://gitlab.com/").size(),
                                      glPos - std::string("https://gitlab.com/").size());
        return "git@gitlab.com:" + path + ".git";
    }

    // Gitee: /<owner>/<repo>/raw/<ref>/...
    if (url.rfind("https://gitee.com/", 0) == 0) {
        auto rawPos = url.find("/raw/");
        if (rawPos != std::string::npos) {
            std::string path = url.substr(std::string("https://gitee.com/").size(),
                                          rawPos - std::string("https://gitee.com/").size());
            return "git@gitee.com:" + path + ".git";
        }
    }

    return raw;
}

bool askYesNo(const std::string& prompt) {
    std::cout << prompt << " [y/N] ";
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    auto begin = line.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return false;
    return std::tolower(static_cast<unsigned char>(line[begin])) == 'y';
}

hpm::Status runGit(const fs::path& cwd,
                                const std::vector<std::string>& args,
                                bool capture = false) {
    std::vector<std::string> full;
    full.reserve(args.size() + 2);
    full.push_back("-C");
    full.push_back(cwd.string());
    full.insert(full.end(), args.begin(), args.end());
    return hpm::util::runProcess("git", full, capture);
}

// publish 目录是否为空（不存在也算空）
bool dirIsEmpty(const fs::path& dir) {
    if (!fs::exists(dir)) return true;
    if (!fs::is_directory(dir)) return false;
    return fs::directory_iterator(dir) == fs::directory_iterator();
}

} // namespace

void InitCmd::setup(CLI::App& app) {
    app.add_flag("--force", force_, "强制重新配置 remote");
}

int InitCmd::run(DevContext& ctx) {
    (void)ctx;

    const fs::path pubDir = fs::current_path() / "publish";

    // 1. 读配置
    auto cfg = loadDevConfig();
    const bool haveCfg = cfg && !isTokenExpired(cfg->expiresAt) && !cfg->repoUrl.empty();

    std::string gitUrl;
    bool userConfirmedRemote = false;
    if (haveCfg) {
        gitUrl = rawToSshUrl(cfg->repoUrl);
        if (gitUrl != cfg->repoUrl) {
            info("raw 地址已转换:");
            std::cout << "  raw : " << cfg->repoUrl << "\n";
            std::cout << "  git : " << gitUrl << "\n";
        } else {
            info("使用远程地址: " + gitUrl);
        }
        userConfirmedRemote = askYesNo("是否将此地址作为远程仓库并克隆到 publish/？");
    } else if (cfg) {
        info("未登录或 AccessToken 已过期，跳过远程配置");
        info("登录后运行 hpm-dev login，然后重新运行 hpm-dev init");
    } else {
        info("未登录，跳过远程配置");
        info("登录后运行 hpm-dev login，然后重新运行 hpm-dev init");
    }

    // 2. 情况一：用户确认远程 -> clone 到 publish
    if (userConfirmedRemote) {
        // publish 已存在且非空：走 fetch + reset 更新
        if (fs::exists(pubDir / ".git")) {
            info("publish 已是 git 仓库，尝试更新远程...");
            auto curUrl = runGit(pubDir, {"remote", "get-url", "origin"}, true);
            if (!curUrl.ok()) {
                auto st = runGit(pubDir, {"remote", "add", "origin", gitUrl});
                if (!st.ok()) { err("添加 remote 失败: " + st.error().message()); return 1; }
                ok("已设置远程仓库: " + gitUrl);
            } else if (force_) {
                auto st = runGit(pubDir, {"remote", "set-url", "origin", gitUrl});
                if (!st.ok()) { err("更新 remote 失败: " + st.error().message()); return 1; }
                ok("已更新远程仓库: " + gitUrl);
            }

            auto fetch = runGit(pubDir, {"fetch", "origin", "main"});
            if (!fetch.ok()) {
                info("fetch 失败（可能是空仓库），继续: " + fetch.error().message());
                return 0;
            }
            auto reset = runGit(pubDir, {"reset", "--hard", "origin/main"});
            if (!reset.ok()) { err("reset 失败: " + reset.error().message()); return 1; }
            ok("已更新到 origin/main");
            return 0;
        }

        // publish 不存在或为空 -> 直接 clone
        if (!dirIsEmpty(pubDir)) {
            err("publish 目录已存在且非空，且不是 git 仓库: " + pubDir.string());
            info("请清空该目录或改用 --force 后重试");
            return 1;
        }

        // clone 要求目标目录为空或不存在：若已存在空目录，先删除
        if (fs::exists(pubDir)) {
            std::error_code ec;
            fs::remove_all(pubDir, ec);
            if (ec) { err("清理空 publish 目录失败: " + ec.message()); return 1; }
        }

        info("克隆远程仓库到 publish/ ...");
        // clone 的最后一个参数是目标目录名；注意不能用 git -C（目录还不存在），
        // 所以在 current_path 下执行，目标写 "publish"
        auto clone = hpm::util::runProcess(
            "git", {"clone", gitUrl, "publish"}, false);
        if (!clone.ok()) {
            err("git clone 失败: " + clone.error().message());
            return 1;
        }
        ok("已克隆到 " + pubDir.string());
        return 0;
    }

    // 3. 情况二：没有远程 / 用户拒绝 -> 仅本地 init
    if (!fs::exists(pubDir)) {
        std::error_code ec;
        if (!fs::create_directories(pubDir, ec) || ec) {
            err("创建 publish 目录失败: " + ec.message());
            return 1;
        }
        ok("已创建目录: " + pubDir.string());
    } else {
        info("publish 目录已存在: " + pubDir.string());
    }

    if (fs::exists(pubDir / ".git")) {
        info("publish 目录已是 git 仓库");
        return 0;
    }

    auto st = runGit(pubDir, {"init", "-b", "main"});
    if (!st.ok()) {
        st = runGit(pubDir, {"init"});
        if (!st.ok()) { err("git init 失败: " + st.error().message()); return 1; }
        runGit(pubDir, {"branch", "-M", "main"});
    }
    ok("已在 publish 目录初始化 git 仓库（main 分支）");
    return 0;
}

} // namespace hpm::cli