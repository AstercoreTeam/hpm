# 贡献指南 (Contributing to HPM)

首先，感谢你愿意为 HPM（Hardware Package Manager）贡献代码或驱动包！正是因为有了像你这样的开发者，嵌入式开发才能变得更现代、更高效。

## 🐛 发现 Bug 或有新想法？
- 请先搜索 [Issues](https://github.com/AstercoreTeam/hpm/issues) 确认是否已有人提出。
- 如果没有，请使用我们提供的 Issue 模板提交（请务必包含系统版本、`project.toml` 和完整的终端报错）。

## 🚀 提交代码 (Pull Request)
1. Fork 本仓库并克隆到本地。
2. 创建新的分支：`git checkout -b feat/your-feature-name`。
3. 本地编译测试：`cmake -B build -DCMAKE_BUILD_TYPE=Release`，`cmake --build build -j`。
4. 提交代码并附带清晰的 Commit Message。
5. 向 `main` 分支发起 PR，并勾选 PR 模板中的检查项。

## 📦 提交驱动包 (Driver Packages)
HPM 的核心是生态。如果你想发布驱动包，**请千万不要直接向本仓库提 PR**。
1. 访问 [HPM 开发者中心](https://hpm.astercore.dpdns.org/dashboard) 注册开发者账号。
2. 阅读 `docs/device_driver.md` 和 `docs/mcu_driver.md`。
3. 在本地使用 `hpm-dev publish` 发布到你的私域仓库。
4. 在开发者中心申请上架公共仓库。

## 🧑‍💻 开发环境
- OS: 原生优先支持 Linux（Windows 用户请使用 WSL2）。
- 编译器: GCC / Clang / SDCC / arm-none-eabi-gcc。
- 构建工具: CMake 3.20+。
- 依赖: libcurl, OpenSSL。