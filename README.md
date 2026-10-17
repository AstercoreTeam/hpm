# HPM · Hardware Package Manager

**硬件包管理器** 用一份清单描述硬件，自动拉取驱动、解析依赖、生成代码、编译烧录。

---

## 目录

- [设计理念](#设计理念)
- [核心概念](#核心概念)
- [安装](#安装)
- [快速开始](#快速开始)
- [命令参考](#命令参考)
- [清单文件](#清单文件)
- [项目结构](#项目结构)
- [工作原理](#工作原理)
- [写驱动包](#写驱动包)
- [写芯片驱动](#写芯片驱动)
- [已验证平台](#已验证平台)
- [Roadmap](#roadmap)
- [参与贡献](#参与贡献)

---

## 设计理念

传统嵌入式开发：

1. 读芯片数据手册，找引脚复用表
2. 下载厂商 SDK，配置 IDE
3. 手动写外设初始化代码
4. 遇到引脚冲突，改代码重来
5. 换芯片时，所有代码重写

HPM 的答案：

1. **声明式**：用户描述「要什么」，工具决定「怎么做」
2. **分层解耦**：驱动是模板，硬件是参数，生成期绑定
3. **能力检查**：所有接口契约在生成期验证，不等到运行时
4. **资源独占**：引脚是唯一资源，冲突在解析时报错
5. **可复现**：`driver.json` 锁定版本和 ID，`driver/` 不提交

---

## 核心概念

### 设备名 vs 硬件名

- **硬件名**：`P1.1`、`ch552g`、`ch552_usb_cdc` —— 物理事实
- **设备名**：`serial` —— 用户起的应用语义名

`project.json` 里写的设备名，决定生成的 API：`serial_write()`、`serial_read()`。

### 引脚是唯一资源

芯片驱动提供引脚，每个引脚带「能力」（能做 `gpio_out` / `spi_sck` / `uart_tx`…）。所有上层驱动都建立在引脚资源之上。

每个引脚有 `shareable` 属性：总线信号（SCK/MISO/MOSI）可共享，CS/RST 独占。

### 驱动分层

```
芯片驱动（提供引脚 + 芯片初始化）      ← 唯一碰寄存器的层
    ↓
总线驱动（spi / i2c / uart）         ← 申请引脚，提供总线接口
    ↓
设备驱动（rc522 / bmp280 / usb_cdc） ← 申请总线 + 引脚，提供应用接口
    ↓
用户应用（main.c）                    ← 只 include "driver.h"
```

每一层只依赖下一层的抽象接口。

### 能力检查

每一层声明 `requires` 和 `provides`，工具在生成期验证：

- 引脚能力是否满足（PA5 能不能做 SPI1_SCK）
- 方法是否存在（spi 驱动提不提供 transfer）
- 名字是否冲突
- 引脚是否被占用

### 中间虚拟设备

`rc522` 要 `spi`，用户只写了 `sck`/`miso`/`mosi` 引脚。工具自动创建中间设备 `__spi_a3f2`，把这三个引脚转移给它，`door_reader.bus` 指向它。

- 默认对用户不可见
- 随机 ID 写入 `driver.json`，之后复用
- 用户可改名
- `hpm dev --export` 后可暴露给底层调试

### 宏机制

每个实例一个目录，一份 `hpm_dev.h`：

```c
// driver/door_reader/hpm_dev.h
#define DEVICE_NAME  door_reader
#define bus          __spi_a3f2
#define cs           hpm_pin_PA4
```

驱动源码用拼接宏：

```c
#define HPM_CAT_(a, b) a##b
#define HPM_CAT(a, b)  HPM_CAT_(a, b)
#define HPM_FN(name)   HPM_CAT(DEVICE_NAME, HPM_CAT(_, name))
#define HPM_CALL(prefix, method)  HPM_CAT(prefix, HPM_CAT(_, method))

int HPM_FN(init)(void) {              // → door_reader_init
    HPM_CALL(cs, init)();             // → hpm_pin_PA4_init
    HPM_CALL(cs, write)(1);           // → hpm_pin_PA4_write(1)
    HPM_CALL(bus, init)();            // → __spi_a3f2_init
    return 0;
}
```

同一份 `rc522.c`，编译两次（`-Idoor_reader` 和 `-Igate_reader`），生成两套符号。驱动源码不改。

### 芯片初始化

芯片驱动也可以声明 `provides.methods`（如 `init`）。`hpm_init_all()` 自动调芯片初始化：

```c
// driver/driver.c（生成）
void hpm_init_all(void) {
    ch552g_init();       // 芯片初始化（时钟等）
    serial_init();       // 设备初始化（USB CDC）
}
```

用户 `main.c` 只需 `#include "driver.h"`。

---

## 安装

### 从源码编译

需要 CMake 3.20+、C++20 编译器。

```bash
git clone https://github.com/xzc842/hpm.git
cd hpm
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cp build/bin/hpm /usr/local/bin/
```

### 工具链

HPM 会从芯片驱动读取需要的工具链。当前版本需要手动安装：

```bash
# ARM Cortex-M
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi

# 8051（CH552）
sudo apt install sdcc

# 烧录工具
# STM32: st-flash（stlink-tools）
# CH552: wchisp（cargo install wchisp 或从源码编译）
```

自动下载工具链在 Roadmap 中。

---

## 快速开始

### 1. 初始化项目

```bash
mkdir my-project && cd my-project
hpm init --mcu ch552g
```

生成 `project.json` 和 `.gitignore`。

### 2. 编辑清单

`project.json`：

```json
{
  "mcu": "ch552g",
  "devices": {
    "serial": {
      "driver": "ch552_usb_cdc"
    }
  }
}
```

### 3. 写应用

`main.c`：

```c
#include "driver.h"

void main(void) {
    hpm_init_all();

    serial_write('H');
    serial_write('P');
    serial_write('M');
    serial_write('\n');

    while (1) {
        if (serial_available()) {
            serial_write(serial_read());
        }
    }
}
```

**只 include `driver.h`。**

### 4. 同步、编译、烧录

```bash
hpm sync
hpm build
hpm flash #烧录功能正在开发！
```

---

## 命令参考

### `hpm`

| 命令 | 说明 |
|------|------|
| `hpm init [--mcu <型号>]` | 初始化项目 |
| `hpm sync [--dry-run]` | 拉取依赖、解析、生成 `driver/` |
| `hpm build` | 编译固件到 `build/` |
| `hpm flash` | 烧录固件 |
| `hpm clean` | 删除 `build/` |
| `hpm dev --export` | 生成 `hpm.h`（底层 API） |

**全局选项**：

| 选项 | 说明 |
|------|------|
| `-C, --project-dir <dir>` | 项目目录（默认当前） |
| `--cache-dir <dir>` | 缓存目录（默认 `~/.hpm`） |
| `-v, --verbose` | 详细输出 |
| `--version` | 显示版本 |

---

## 清单文件

### `project.json`（用户写）

```json
{
  "mcu": "ch552g",
  "devices": {
    "<设备名>": {
      "driver": "<驱动名>",
      "<资源key>": "<引脚名或设备名>"
    }
  }
}
```

- `<设备名>`：应用语义名，决定生成的 API 前缀
- `<驱动名>`：驱动包名字
- `<资源key>`：跟驱动 `requires` 的 key 一致

---

## 工作原理

### `hpm sync`

```
1. 读 project.json → Project
2. 读已有 driver.json（可空）
3. 从本地包目录加载芯片驱动 → Chip
4. 从本地包目录加载所有驱动 → DriverLibrary
5. Resolver.resolve() → DriverState       （依赖解析，创建中间设备）
6. CapabilityChecker.check(state)         （能力检查、冲突检测）
7. 写 driver.json
8. Generator.generate() → driver/         （代码生成）
```

### 依赖解析（两阶段）

**阶段 1**：递归展开成 `ResolvedNode` 树
- 从用户设备的 bindings 里按 key 取值
- 遇到 `req.type` 是驱动名 → 创建子节点（中间设备）
- 检测循环依赖、同名 key 冲突

**阶段 2**：把树摊平成 `DriverState`
- 根节点 → `user_devices`
- 子节点 → `intermediate_devices`
- 中间设备复用：按 `(driver, pins)` 查已有状态

### 代码生成

对每个实例（用户设备 + 中间设备）：
1. 创建 `driver/<instance>/` 目录
2. 生成 `hpm_dev.h`（`DEVICE_NAME` + 资源宏）
3. 拷贝驱动源码
4. 拷贝芯片驱动（全局一份）

生成 `driver.h`（芯片 + 设备 API）、`driver.c`（`hpm_init_all`）、`Makefile`。

---

## 写驱动包

### 目录结构

```
mydevice-1.0.0/
├── manifest.json
├── src/
│   └── mydevice.c
└── include/
    └── mydevice.h
```

放到 `~/.hpm/packages/`。

### `manifest.json`

```json
{
  "name": "mydevice",
  "version": "1.0.0",
  "description": "My device driver",
  "requires": {
    "bus": { "type": "spi", "methods": ["init", "transfer"] },
    "cs":  { "type": "gpio_out", "methods": ["init", "write"] }
  },
  "provides": {
    "methods": [
      { "name": "init", "return": "int", "params": [] },
      {
        "name": "read",
        "return": "int",
        "params": [
          { "type": "unsigned char*", "name": "buf" },
          { "type": "unsigned char", "name": "len" }
        ]
      }
    ]
  }
}
```

**requires 的 key** = `hpm_dev.h` 里的宏名
**requires 的 type**：
- 是驱动名（如 `"spi"`）→ 工具递归创建中间设备
- 否则是引脚能力（如 `"gpio_out"`）→ 从上层 bindings 取引脚

**provides.methods**：
- `name`：方法名，生成 `<device>_<name>`
- `return`：返回类型，默认 `int`
- `params`：参数列表，每项 `{ type, name }`

### `include/mydevice.h`

```c
#pragma once
#include "hpm_dev.h"
```

**注意**：`hpm_dev.h` 由工具生成，驱动包里**没有这个文件**。

### `src/mydevice.c`

```c
#include "mydevice.h"
#include <stddef.h>

#define HPM_CAT_(a, b) a##b
#define HPM_CAT(a, b)  HPM_CAT_(a, b)
#define HPM_FN(name)   HPM_CAT(DEVICE_NAME, HPM_CAT(_, name))
#define HPM_CALL(prefix, method)  HPM_CAT(prefix, HPM_CAT(_, method))

int HPM_FN(init)(void) {
    HPM_CALL(cs, init)();
    HPM_CALL(cs, write)(1);
    HPM_CALL(bus, init)();
    return 0;
}

int HPM_FN(read)(unsigned char* buf, unsigned char len) {
    unsigned char tx[4] = {0x01, 0x02, 0x03, 0x04};
    unsigned char rx[4] = {0};
    HPM_CALL(bus, transfer)(tx, rx, sizeof(tx));
    for (unsigned char i = 0; i < len && i < 4; i++) {
        buf[i] = rx[i];
    }
    return 0;
}
```

### 关键规则

**C 预处理器只匹配独立 token**。`cs` 宏不会匹配 `cs_init` 里的 `cs`。必须用拼接宏 `HPM_CALL(cs, init)`。

**不要用 `printf`**。驱动是硬件抽象层，不该有 IO 副作用。裸机没有 libc。

**不要硬编码引脚名**。用宏（`cs`、`bus`），让工具注入。

**EP buffer 等共享数据用 `extern` 声明**。头文件 `extern`，`.c` 里定义，避免多个 `.c` include 导致重复符号。

---

## 写芯片驱动

### 目录结构

```
ch552g-1.0.0/
├── chip.json
├── src/
│   ├── ch552g.c
│   └── hpm_pin.c
└── include/
    ├── CH552.H
    ├── ch552g.h
    └── hpm_pin.h
```

### `chip.json`

```json
{
  "name": "ch552g",
  "family": "ch55x",
  "package": "SOP16",
  "build": {
    "cpu": "mcs51",
    "flags": ["-mmcs51", "--model-small"],
    "ldscript": "",
    "startup": ""
  },
  "toolchain": {
    "cc": "sdcc",
    "objcopy": "packihx",
    "size": ""
  },
  "flash": {
    "tool": "wchisp",
    "args": ["flash", "{hex}"]
  },
  "provides": {
    "methods": [
      { "name": "init", "return": "void", "params": [] }
    ]
  },
  "pins": {
    "P1_1": { "capabilities": [{ "type": "gpio_out" }] },
    "P1_4": { "capabilities": [{ "type": "gpio_out" }] },
    "P1_5": { "capabilities": [{ "type": "gpio_out" }] },
    "P3_0": { "capabilities": [{ "type": "gpio_out" }] },
    "P3_1": { "capabilities": [{ "type": "gpio_out" }] },
    "P3_2": { "capabilities": [{ "type": "gpio_out" }] },
    "P3_3": { "capabilities": [{ "type": "gpio_out" }] },
    "P3_4": { "capabilities": [{ "type": "gpio_out" }] }
  }
}
```

### 引脚能力类型

| 类型 | 说明 |
|------|------|
| `gpio_out` / `gpio_in` | 普通 GPIO |
| `spi_sck` / `spi_miso` / `spi_mosi` | SPI 信号 |
| `i2c_scl` / `i2c_sda` | I2C |
| `uart_tx` / `uart_rx` | UART |

`shareable: true` 用于总线信号。

### 芯片初始化

`provides.methods` 里的 `init` 会生成 `<chip>_init()`，被 `hpm_init_all()` 调用。

`ch552g.h`：

```c
#pragma once

#include <stdint.h>
#include "CH552.H"

static inline void ch552g_clock_24m(void) {
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_SEL) | 0x06;
    SAFE_MOD = 0x00;
}

void ch552g_init(void);
```

`ch552g.c`：

```c
#include "ch552g.h"

void ch552g_init(void) {
    ch552g_clock_24m();
}
```

### `src/hpm_pin.c`

为每个引脚提供 `hpm_pin_<PIN>_<method>`：

```c
#include "hpm_pin.h"
#include "ch552g.h"

#define DEF_PIN(port, bit) \
    void hpm_pin_##port##_##bit##_init(void) { \
        port##_MOD_OC &= ~(1 << bit); \
        port##_DIR_PU |=  (1 << bit); \
    } \
    void hpm_pin_##port##_##bit##_write(int level) { \
        if (level) port |=  (1 << bit); \
        else       port &= ~(1 << bit); \
    } \
    int hpm_pin_##port##_##bit##_read(void) { \
        return (port >> bit) & 1; \
    }

DEF_PIN(P1, 1)
DEF_PIN(P1, 4)
DEF_PIN(P1, 5)
DEF_PIN(P3, 0)
DEF_PIN(P3, 1)
DEF_PIN(P3, 2)
DEF_PIN(P3, 3)
DEF_PIN(P3, 4)
```

**函数命名约定**：`hpm_pin_<PIN>_<method>`。工具生成的 `hpm_dev.h` 里 `#define cs hpm_pin_PA4`，驱动源码 `HPM_CALL(cs, init)` 展开成 `hpm_pin_PA4_init`。

### 裸机启动

**ARM（STM32）**：芯片驱动负责提供
- `startup.c`：向量表 + `Reset_Handler`
- `linker.ld`：Flash/RAM 布局
- 编译选项（`build.flags`）

**8051（CH552）**：SDCC 自带启动代码，`build.startup` 留空。

---

## 已验证平台

### CH552G（8051 + USB CDC）

| 项目 | 内容 |
|------|------|
| 芯片驱动 | `ch552g-1.0.0` |
| 设备驱动 | `ch552_usb_cdc-1.0.0` |
| 工具链 | `sdcc` + `packihx` |
| 烧录 | `wchisp flash firmware.hex` |
| 验证内容 | USB 枚举、串口收发、回显、交互程序 |


---

## Roadmap

### 已完成

- [x] semver、版本约束
- [x] JSON 读写（值语义，不抛异常）
- [x] 清单解析（project / driver / state / chip）
- [x] 能力检查（引脚、方法、名字）
- [x] 依赖解析（两阶段，中间设备复用）
- [x] 代码生成（实例目录、hpm_dev.h、Makefile、方法签名）
- [x] CLI（init / sync / build / flash / clean / dev）
- [x] 裸机编译（ARM 交叉工具链、链接脚本、启动文件）
- [x] 芯片驱动 `provides`（`hpm_init_all` 调芯片初始化）
- [x] 工具链显式声明（`cc`/`objcopy`/`prefix`）
- [x] 方法签名结构化（`return` + `params`）
- [x] CH552G USB CDC 真机验证
- [x] Makefile 源文件扫描（支持一个驱动多个 `.c`）
- [x] `hpm.h` 完整实现
- [x] **registry**：从 GitHub 拉驱动包和索引
- [x] **签名校验**：包完整性
- [x] **多镜像支持**：项目 / 用户 / 全局优先级
- [x] **hpm-dev**：驱动开发工具（validate）
- [x] **许可证处理**：驱动包带 license

### 进行中

- [ ] **toolchain 管理**：自动下载工具链
- [ ] **hpm-dev**：驱动开发工具（init / test / publish）
- [ ] **QEMU 测试**：`hpm-dev test` 支持多芯片模拟

### 计划中


- [ ] **hpm-gui**：Qt 图形化生成 `project.json`
- [ ] **KiCad 导入**：从原理图生成 `project.json`
- [ ] **AI 驱动包生成**：从文档 + 开源代码整理驱动包

---

## 参与贡献

### 开发

```bash
git clone https://gitcode.com/xzc842/hpm.git
cd hpm
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### 测试

每个模块都有单元测试（`tests/unit/`）。新增功能请附带测试。

```bash
ctest --test-dir build -R "capability"
```

### 驱动包贡献

见 [写驱动包](#写驱动包)。驱动包先放本地 `~/.hpm/packages/` 测试，registry 开放后提交。

### 芯片驱动贡献

见 [写芯片驱动](#写芯片驱动)。优先支持：

- STM32F103C8T6（Blue Pill）
- ESP32-C3
- RP2040
- CH32V003
先放本地 `~/.hpm/packages/` 测试，registry 开放后提交。

### 文档

`docs/`[docs] 目录下

## 致谢

本项目的开发离不开以下优秀开源项目和社区的贡献，在此我们表示由衷的感谢。

| 项目 | 用途 | 开源协议 |
| :--- | :--- | :--- |
| [nlohmann/json](https://github.com/nlohmann/json.git) | JSON 解析 | MIT |
| [fmt](https://github.com/fmtlib/fmt.git) | 字符串格式化 | MIT |
| [CLI11](https://github.com/CLIUtils/CLI11.git) | 命令行解析 | BSD-3-Clause |
| [Catch2](https://github.com/catchorg/Catch2.git) | 单元测试 | BSL-1.0 |
| [libcurl](https://github.com/curl/curl.git) | HTTP 客户端（registry 用） | curl（MIT 类） |

## 开源许可

Apache-2.0 [LICENSE](LICENSE)

## 贡献

欢迎提交 Issue 和 Pull Request。请参阅[ToDo文档](ToDo.md)

## 联系方式
---

作者

Xue Zicheng <xuezicheng842@outlook.com>

GitCode: https://gitcode.com/xzc842

---
