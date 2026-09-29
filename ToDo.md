# HPM 项目当前状态与待处理清单

按模块分类。标注「已完成」「简化中」「未开始」。

---

## 一、整体状态

**已跑通的最小链路**：

```
project.json  →  hpm sync  →  driver/  →  hpm build  →  firmware.elf / .bin
```

- 能解析 `project.json`、`driver.json`
- 能从本地包目录加载芯片驱动和驱动
- 能展开依赖、创建中间设备、复用 ID
- 能做能力检查（引脚、方法、名字）
- 能生成 `driver/`（实例目录 + `hpm_dev.h` + `driver.h` + `Makefile`）
- 能用 `arm-none-eabi-gcc` 交叉编译
- 产物是 ARM ELF（`arm-none-eabi-gcc` + 链接脚本 + 启动文件）

**已验证**：
- `hpm init` / `hpm sync` / `hpm build` 端到端可跑
- `file build/firmware.elf` → `ELF 32-bit LSB executable, ARM, EABI5`
- 有 `stm32f103c8t6` mock 芯片驱动 + `rc522`/`spi` mock 驱动

**未验证**：
- 真机烧录（`hpm flash` 未测试）
- 真寄存器操作（`hpm_pin.c` 是假寄存器）
- 点灯

---

## 二、`version_range` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| `^0.x.y` 语义 | 按标准 semver（`^0.2.3` → `>=0.2.3 <1.0.0`） | 是否对齐 npm（`<0.3.0`）待定 |
| 预发布版本 | 约束匹配不特殊处理预发布 | 正式约束默认排除预发布 |
| 或运算 | 不支持 `\|\|` | 需要时加 |
| 范围简写 | 不支持 `1.2.x`、`1.x` | 需要时加 |

**影响**：中。当前驱动版本简单，`^`/`~` 够用。

---

## 三、`json` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| Schema 校验 | 手工字段校验 | 可选 |
| `Value` 深拷贝 | `get/at` 返回值拷贝 | 性能敏感时改引用 |
| 文件编码 | 假设 UTF-8，无 BOM 处理 | Windows 编辑器兼容 |

**影响**：低。当前没遇到问题。

---

## 四、`model` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| `ProvidedMethod.signature` | 字符串，不解析 | 解析成参数列表，做严格检查 |
| `PinCapability.channel` | 字段存在但未用 | 定时器 PWM 通道需要 |
| `ToolchainSpec.url` | 只存不解析 | 下载工具链时用 |
| `Device.version` | 用户可指定，锁定逻辑简化 | 明确「用户指定」vs「锁定」 |
| `ChipBuildConfig` | 已加，被 Generator 使用 | — |

**影响**：中。`signature` 不解析导致 `driver.h` 只能生成 `int fn(void)`。

---

## 五、`manifest` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| 字段白名单 | 未知字段静默忽略 | 严格模式可警告 |
| `project.json` 字段 | 除 `driver`/`version` 外都当资源绑定 | 明确元数据字段 |
| 跨字段引用 | `bus: spi1` 不检查存在性 | capability 阶段才查 |
| 数组元素类型 | 只查顶层 | 复杂结构递归 |
| `state_loader` | 未处理 `id != name` | 用户改名后重载 |
| Schema 版本 | 无 | 加 `schema_version` |

**影响**：低。当前格式简单。

---

## 六、`capability` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| 引脚能力匹配 | `spi` 匹配 `spi_sck/miso/mosi` 任一 | 严格按 instance 匹配（SPI1 vs SPI2） |
| Instance 校验 | `PinCapability.instance` 未参与判定 | 引脚复用时检查同一 instance |
| `bindings` 二义性 | 引脚名 vs 设备名靠 `state.findXxx` 判断 | 显式标注资源类型 |
| 方法签名检查 | 只检查名字 | 参数、返回类型 |
| 中间设备引脚 | 已检查 | — |
| 警告机制 | `addWarning` 定义但未用 | 「可选缺失」「版本过旧」 |
| 循环引用（设备级） | 只检测驱动级 | 设备 A 引 B、B 引 A |
| **输入改为 `DriverState`** | **已完成** | — |

**影响**：中。当前测试覆盖主要场景，但 instance 区分缺失。

---

## 七、`resolver` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| 「驱动名 vs 引脚」判定 | `lib_.find(req.type)` 命中 | 显式标记 `kind` |
| 多总线 | 支持递归，同名 key 冲突报错 | 前缀或嵌套语法 |
| 循环检测 | 只查驱动名 | 按 `(driver, pins)` 缓存 |
| 版本约束 | 取库最新/沿用旧版 | 真正版本求解 |
| `consumed` 作用域 | **已确认每设备独立** | — |
| 中间设备复用 | 本次 + `existing_` | 跨项目全局缓存 |
| 中间设备 name vs 用户设备 name | 未检查冲突 | 生成时统一检查 |
| 废弃中间设备 | `existing_` 有但本次不需要，不清理 | 清理逻辑 |
| **`created_` 缓存** | **已完成** | 同 Project 内多设备共享中间设备 |

**影响**：低。当前场景能跑。

---

## 八、`generator` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| **实例目录** | **已完成**，每实例一个目录 | — |
| **`hpm_dev.h`** | **已完成**，只有 `DEVICE_NAME` + 资源宏 | — |
| **多实例编译** | **已完成**，编译两次，靠 `-I` 切换 | — |
| **链接脚本 + 启动文件** | **已完成**，从 `chip.build` 读 | — |
| **工具链前缀** | **已完成**，从 `toolchain.name` 推导 | 改成显式声明 |
| 方法签名 | 仍是 `int fn(void)` | 从 `signature` 解析 |
| 驱动模板化 | 直接复制，不做 `{{}}` 替换 | 用宏（已确定方案） |
| Makefile 源文件扫描 | 假定 `<instance>/<driver>.c` | 扫描目录所有 `.c` |
| Makefile include 路径 | 已加 `-I<instance>` | — |
| `driver.c` 内容 | 只生成 `hpm_init_all` | 完整实例绑定胶水 |
| 编译标志 | 从 `chip.build.flags` 读 | — |
| `hpm.h` | 最小占位，函数声明假 | `hpm dev --export` 时补 |
| 增量生成 | 无，每次全量 | 内容哈希对比 |
| 生成文件冲突 | 不检测多个设备同名宏 | 加校验 |
| `.gitignore` 生成 | `hpm init` 时写 | — |

**影响**：高。源文件扫描和签名解析是下一个关键点。

---

## 九、`util` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| **`process.h/.cpp`** | **已完成**，子进程调用 | — |
| **`fs_utils.h/.cpp`** | **已完成**，原子写文件 | — |
| 日志系统 | 只有 `Output` 简单打印 | 分级、文件 |
| 进度显示 | 无 | 拉工具链/驱动时进度条 |
| 环境变量 | 支持 `HPM_HOME` | 加 `HPM_CACHE` 等 |
| 路径规范化 | 直接用 `std::filesystem` | Windows 兼容 |

**影响**：低。

---

## 十、`cli` 模块

| 项 | 现状 | 后续 |
|----|------|------|
| **`init`** | **已完成** | — |
| **`sync`** | **已完成** | — |
| **`build`** | **已完成** | — |
| **`flash`** | **已完成**，未真机测试 | 测试 |
| **`clean`** | **已完成** | — |
| **`dev --export`** | **已完成**，最小版 | 完整实现 |
| 日志级别 | 只有 `-v` | `-q`、`--no-color` |
| 配置文件 | 无 `~/.hpmrc` | 镜像、缓存路径 |
| Shell 补全 | 无 | 可选 |

**影响**：中。命令齐了，细节待补。

---

## 十一、整体架构层面

| 项 | 现状 | 后续 |
|----|------|------|
| **本地包目录** | `~/.hpm/packages/<name>-<version>/` | — |
| **`chip.json`** | 已支持，含 `build`、`flash`、`pins`、`toolchain` | — |
| **`manifest.json`** | 已支持 | — |
| registry 客户端 | 无，手动放包 | `github_backend`/`http_backend` |
| 工具链管理 | 手动装 `arm-none-eabi-gcc` | 自动下载 |
| 本地缓存 | 只用 `packages/` | 索引缓存、工具链缓存 |
| 锁文件对比 | 每次全量算 | 对比 `driver.json` 跳过 |
| `hpm-dev` | 未开始 | init/test/validate/publish |
| `hpm-gui` | 未开始 | Qt |
| KiCad 导入 | 未开始 | — |
| 芯片能力自动生成 | 手写 | 从 SVD/PDF 提取 |
| AI 驱动包生成 | 未开始 | — |
| 多镜像支持 | 无 | 项目/用户/全局 |
| 许可证处理 | 无 | 驱动包带 license |
| 签名校验 | 无 | 包完整性 |

**影响**：高。registry 和 toolchain 是「用户不用手动配置」的关键。

---

## 十二、测试层面

| 项 | 现状 | 后续 |
|----|------|------|
| **单元测试** | 约 55 个，覆盖 model / manifest / capability / resolver / generator | — |
| 集成测试 | 无 | 完整 `sync + build` |
| 真机测试 | 无 | QEMU |
| 性能测试 | 无 | 大项目解析速度 |
| Fuzzing | 无 | 清单解析健壮性 |
| Fixtures | 内联 JSON 字符串 | 提取到 `tests/fixtures/` |

---

## 十三、工程层面

| 项 | 现状 | 后续 |
|----|------|------|
| CI 配置 | 无 | GitHub Actions |
| 格式化检查 | `.clang-format` 存在但未集成 | pre-commit |
| 静态分析 | `.clang-tidy` 存在但未跑 | 定期 |
| 文档 | README 已写 | `docs/` 展开 |
| 示例项目 | 无 | `examples/` |
| 安装脚本 | 无 | `install.sh` / 包管理器 |
| 版本发布流程 | 无 | tag + release |
| 二进制分发 | 无 | 各平台预编译 |

---

## 十四、设计决策已定

| 项 | 决定 |
|----|------|
| 驱动源码的宏体系 | `hpm_dev.h` 每实例一份，驱动源码用 `HPM_CALL` 拼接 |
| 多实例代码组织 | 每实例一个目录，编译多次，靠 `-I` 切换 |
| 引脚宏值 | 函数前缀（`hpm_pin_PA4`） |
| 芯片驱动自带 | 启动文件、链接脚本、`build.flags` |
| `driver.json` | 提交 git，锁定 ID 和版本 |
| `driver/` | 不提交，生成产物 |
| 中间设备 ID | 首次随机，写入 `driver.json` 后复用 |
| 中间设备复用 | 按 `(driver, pins)` |
| 名字冲突 | 报错 |
| `hpm build` 工具链 | 从 `chip.toolchain.name` 推前缀 |

## 十五、设计决策未定

| 项 | 现状 |
|----|------|
| 驱动包格式 | 目录，未定 tarball |
| 索引格式 | `index.json` 未设计 |
| 镜像协议 | HTTP API 未设计 |
| 工具链下载源 | 官方 CDN？镜像？ |
| 版本求解算法 | npm 式？PubGrub？ |
| `hpm.h` 暴露范围 | 未定 |
| `driver.json` schema 版本 | 未定 |
| 中间设备改名流程 | 未实现 |
| 签名结构化格式 | `signature` 是字符串还是结构化 |

---

## 十六、优先建议

### 高优先级

| # | 项 | 说明 |
|---|----|------|
| 1 | **Makefile 源文件扫描** | 驱动包多个 `.c` 会漏，改成扫目录 |
| 2 | **方法签名解析** | `signature` 解析成参数列表，`driver.h` 生成正确声明 |
| 3 | **工具链显式声明** | `chip.json` 里写 `cc`/`objcopy`，别从名字推导 |
| 4 | **registry 客户端** | 从 GitHub 拉包，替代手动放 `~/.hpm/packages/` |
| 5 | **toolchain 管理** | 自动下载 `arm-none-eabi-gcc` |
| 6 | **真机验证** | 把 mock 的 `hpm_pin.c` 换成真 STM32 寄存器，点灯 |

### 中优先级

| # | 项 | 说明 |
|---|----|------|
| 7 | 增量生成 | 内容哈希跳过未变 |
| 8 | 日志系统 | 分级、颜色 |
| 9 | 中间设备改名流程 | 引用自动跟改 |
| 10 | `hpm.h` 完整实现 | 暴露引脚 + 中间设备 |
| 11 | 集成测试 | 完整 `sync + build` |
| 12 | 版本求解 | 多约束交集 |

### 低优先级（后续迭代）

| # | 项 |
|---|----|
| 13 | `hpm-dev` |
| 14 | `hpm-gui` |
| 15 | KiCad 导入 |
| 16 | AI 驱动包生成 |
| 17 | 芯片能力自动生成 |
| 18 | 多镜像 |
| 19 | 签名校验 |
