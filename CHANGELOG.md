# 更新日志 (Changelog)

## [v0.1.0-alpha] - 2026-11-01
### 🎉 首次公测版本发布 (First Public Beta)

#### ✨ 核心特性 (Core Features)
- **声明式开发**：支持 `project.toml` 和 `project.json` 配置，自动解析依赖与冲突。
- **跨平台代码生成**：一份代码，仅修改配置即可无缝编译至 STC90C51、CH552G、STM32F103 等异构芯片。
- **私域仓库**：支持多级私域仓库，采用多级签名信任链与公钥验证机制。

#### 🛠️ 开发者工具 (CLI & Developer Center)
- 全新的 `hpm` 与 `hpm-dev` 命令行。
- `hpm-dev login`：基于回环重定向的 OAuth 2.0 安全授权（含校验码防钓鱼）。
- 开发者中心上线：支持中英文切换、亮暗色主题、新设备/敏感操作邮件告警、2FA 登录。
- 支持通过开发者中心自动生成索引、管理 AppID/Key、以及私域仓库所有权验证。

#### ⚠️ 已知限制 (Known Limitations)
- 目前仅原生支持 Linux 环境，Windows 用户请使用 WSL2。
- GUI（`hpm-gui`）、仿真与调试器（`hpm-debug`）尚在开发中。