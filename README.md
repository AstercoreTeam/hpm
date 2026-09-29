# HPM · Hardware Package Manager

**声明式嵌入式开发工具链。** 用一份清单描述硬件，自动拉取驱动、解析依赖、生成代码、编译烧录。

```json
{
  "mcu": "stm32f103c8t6",
  "devices": {
    "door_reader": {
      "driver": "rc522",
      "cs": "PA4",
      "sck": "PA5",
      "miso": "PA6",
      "mosi": "PA7"
    }
  }
}
```

```bash
hpm init      # 初始化项目
hpm sync      # 拉驱动、解析依赖、生成 driver/
hpm build     # 编译固件
hpm flash     # 烧录
```

没有 `#include "stm32f1xx_hal.h"`，没有 `HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, ...)`，没有手动配时钟树和引脚复用。

用户只写 `door_reader_read_card()`。

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

**核心理念**：驱动是模板，硬件是参数，工具是渲染器。

---

## 核心概念

### 设备名 vs 硬件名

- **硬件名**：`spi1`、`PA4`、`rc522` —— 物理事实
- **设备名**：`door_reader` —— 用户起的应用语义名

`project.json` 里写的设备名，决定生成的 API：`door_reader_read_card()`。

### 引脚是唯一资源

芯片驱动提供引脚，每个引脚带「能力」（能做 `spi_sck` / `gpio_out` / `uart_tx`…）。所有上层驱动都建立在引脚资源之上。

每个引脚有 `shareable` 属性：总线信号（SCK/MISO/MOSI）可共享，CS/RST 独占。

### 驱动分层

```
芯片驱动（提供引脚能力）              ← 唯一碰寄存器的层
    ↓
总线驱动（spi / i2c / uart）         ← 申请引脚，提供总线接口
    ↓
设备驱动（rc522 / bmp280 / oled）    ← 申请总线 + 引脚，提供应用接口
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
int HPM_FN(init)(void) {              // → door_reader_init
    HPM_CALL(cs, init)();             // → hpm_pin_PA4_init
    HPM_CALL(cs, write)(1);           // → hpm_pin_PA4_write(1)
    HPM_CALL(bus, init)();            // → __spi_a3f2_init
    return 0;
}
```

同一份 `rc522.c`，编译两次（`-Idoor_reader` 和 `-Igate_reader`），生成两套符号。驱动源码不改。

---

## 安装

### 从源码编译

需要 CMake 3.20+、C++20 编译器。

```bash
git clone https://github.com/yourname/hpm.git
cd hpm
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cp build/bin/hpm /usr/local/bin/
```

### 工具链

HPM 会从芯片驱动读取需要的工具链。当前版本需要手动安装：

```bash
# Ubuntu/Debian
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi

# macOS
brew install --cask gcc-arm-embedded

# Windows
# 下载 https://developer.arm.com/downloads/-/gnu-rm
```

自动下载工具链在 Roadmap 中。

---

## 快速开始

### 1. 初始化项目

```bash
mkdir my-project && cd my-project
hpm init --mcu stm32f103c8t6
```

生成 `project.json` 和 `.gitignore`。

### 2. 编辑清单

`project.json`：

```json
{
  "mcu": "stm32f103c8t6",
  "devices": {
    "door_reader": {
      "driver": "rc522",
      "cs": "PA4",
      "rst": "PA3",
      "sck": "PA5",
      "miso": "PA6",
      "mosi": "PA7"
    }
  }
}
```

引脚都写在应用设备下。工具自动识别 `sck`/`miso`/`mosi` 属于 SPI 总线，创建中间设备。

### 3. 写应用

`main.c`：

```c
#include "driver.h"

int main(void) {
    hpm_init_all();
    int card = door_reader_read_card();
    // ...
    return 0;
}
```

### 4. 同步、编译、烧录

```bash
hpm sync
hpm build
hpm flash
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
  "mcu": "stm32f103c8t6",
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

### `driver.json`（工具生成 + 可改）

锁定中间设备 ID 和驱动版本。**提交到 git**。

```json
{
  "user_devices": {
    "door_reader": {
      "driver": "rc522",
      "version": "1.2.0",
      "bus": "__spi_a3f2",
      "cs": "PA4"
    }
  },
  "intermediate_devices": {
    "__spi_a3f2": {
      "name": "__spi_a3f2",
      "driver": "spi",
      "version": "1.0.0",
      "sck": "PA5",
      "miso": "PA6",
      "mosi": "PA7"
    }
  }
}
```

用户可以：
- 改 `__spi_a3f2.name`（改成 `spi_door`），引用它的 `bus` 字段自动跟改
- 改 `version` 手动升级驱动
- 改完运行 `hpm sync` 生效

---

## 项目结构

```
my-project/
├── project.json      # 用户手写，声明硬件
├── driver.json       # 工具生成，锁定状态（提交 git）
├── main.c            # 用户应用
├── driver/           # 生成产物（不提交）
│   ├── driver.h      # 用户 API
│   ├── driver.c      # 胶水
│   ├── Makefile
│   ├── door_reader/  # 实例1
│   │   ├── hpm_dev.h
│   │   ├── rc522.c
│   │   └── rc522.h
│   ├── __spi_a3f2/   # 中间设备
│   │   ├── hpm_dev.h
│   │   ├── spi.c
│   │   └── spi.h
│   └── stm32f103c8t6/  # 芯片驱动（全局一份）
│       ├── stm32f103c8t6.c
│       ├── hpm_pin.c
│       ├── startup.c
│       └── stm32f103c8t6.ld
└── build/            # 编译产物（不提交）
    ├── firmware.elf
    └── firmware.bin
```

**提交**：`project.json`、`driver.json`、`main.c`
**不提交**：`driver/`、`build/`、`.hpm/`

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

生成 `driver.h`（用户 API）、`driver.c`（胶水）、`Makefile`。

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
    "cs":  { "type": "gpio_out", "methods": ["init", "write"] },
    "rst": { "type": "gpio_out", "methods": ["init", "write"], "optional": true }
  },
  "provides": {
    "methods": ["init", "read_card"]
  }
}
```

**requires 的 key** = `hpm_dev.h` 里的宏名
**requires 的 type**：
- 是驱动名（如 `"spi"`）→ 工具递归创建中间设备
- 否则是引脚能力（如 `"gpio_out"`）→ 从上层 bindings 取引脚

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

int HPM_FN(read_card)(void) {
    unsigned char tx[4] = {0x01, 0x02, 0x03, 0x04};
    unsigned char rx[4] = {0};
    HPM_CALL(bus, transfer)(tx, rx, sizeof(tx));
    return (int)rx[0];
}
```

### 关键规则

**C 预处理器只匹配独立 token**。`cs` 宏不会匹配 `cs_init` 里的 `cs`。必须用拼接宏 `HPM_CALL(cs, init)`。

**不要用 `printf`**。驱动是硬件抽象层，不该有 IO 副作用。裸机没有 libc。

**不要硬编码引脚名**。用宏（`cs`、`bus`），让工具注入。

---

## 写芯片驱动

### 目录结构

```
stm32f103c8t6-1.0.0/
├── chip.json
├── src/
│   ├── stm32f103c8t6.c
│   ├── hpm_pin.c
│   └── startup.c
├── include/
│   ├── stm32f103c8t6.h
│   └── hpm_pin.h
└── ld/
    └── stm32f103c8t6.ld
```

### `chip.json`

```json
{
  "name": "stm32f103c8t6",
  "family": "stm32f1",
  "package": "LQFP48",
  "build": {
    "cpu": "cortex-m3",
    "flags": ["-mthumb", "-mcpu=cortex-m3", "-nostdlib", "-nostartfiles"],
    "ldscript": "ld/stm32f103c8t6.ld",
    "startup": "src/startup.c"
  },
  "toolchain": { "name": "arm-none-eabi-gcc" },
  "flash": {
    "tool": "st-flash",
    "args": ["write", "{bin}", "0x8000000"]
  },
  "pins": {
    "PA4": { "capabilities": [{ "type": "gpio_out" }] },
    "PA5": { "shareable": true, "capabilities": [{ "type": "spi_sck", "instance": "SPI1" }] },
    "PA6": { "shareable": true, "capabilities": [{ "type": "spi_miso", "instance": "SPI1" }] },
    "PA7": { "shareable": true, "capabilities": [{ "type": "spi_mosi", "instance": "SPI1" }] }
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

### `src/hpm_pin.c`

为每个引脚提供 `hpm_pin_<PIN>_<method>`：

```c
#include "hpm_pin.h"

// 真实芯片：操作寄存器
// Mock：操作假寄存器
static volatile uint32_t g_gpio_state = 0;

#define PIN_BIT(s)  ((((s)[1] - 'A') * 16) + ((s)[2] - '0'))

#define DEF_PIN(name) \
    void hpm_pin_##name##_init(void) { } \
    void hpm_pin_##name##_write(int level) { \
        int bit = PIN_BIT(#name); \
        if (level) g_gpio_state |=  (1u << bit); \
        else       g_gpio_state &= ~(1u << bit); \
    } \
    int hpm_pin_##name##_read(void) { \
        int bit = PIN_BIT(#name); \
        return (int)((g_gpio_state >> bit) & 1u); \
    }

DEF_PIN(PA3)
DEF_PIN(PA4)
DEF_PIN(PA5)
DEF_PIN(PA6)
DEF_PIN(PA7)
```

**函数命名约定**：`hpm_pin_<PIN>_<method>`。工具生成的 `hpm_dev.h` 里 `#define cs hpm_pin_PA4`，驱动源码 `HPM_CALL(cs, init)` 展开成 `hpm_pin_PA4_init`。

### 裸机启动

芯片驱动负责提供：
- `startup.c`：向量表 + `Reset_Handler`
- `linker.ld`：Flash/RAM 布局
- 编译选项（`build.flags`）

---

## Roadmap

### 已完成

- [x] semver、版本约束
- [x] JSON 读写（值语义，不抛异常）
- [x] 清单解析（project / driver / state / chip）
- [x] 能力检查（引脚、方法、名字）
- [x] 依赖解析（两阶段，中间设备复用）
- [x] 代码生成（实例目录、hpm_dev.h、Makefile）
- [x] CLI（init / sync / build / flash / clean / dev）
- [x] 裸机编译（交叉工具链、链接脚本、启动文件）

### 进行中

- [ ] 从芯片驱动读工具链前缀（现在硬编码推导）
- [ ] Makefile 源文件扫描（现在假定命名）
- [ ] 方法签名解析（现在 `driver.h` 用 `int fn(void)`）
- [ ] `hpm.h` 完整实现

### 计划中

- [ ] **registry**：从 GitHub 拉驱动包和索引
- [ ] **toolchain 管理**：自动下载工具链
- [ ] **hpm-dev**：驱动开发工具（init / test / validate / publish）
- [ ] **QEMU 测试**：`hpm-dev test` 支持多芯片模拟
- [ ] **hpm-gui**：Qt 图形化生成 `project.json`
- [ ] **KiCad 导入**：从原理图生成 `project.json`
- [ ] **AI 驱动包生成**：从文档 + 开源代码整理驱动包
- [ ] **多镜像支持**：项目 / 用户 / 全局优先级
- [ ] **增量生成**：内容哈希跳过未变文件
- [ ] **许可证处理**：驱动包带 license
- [ ] **签名校验**：包完整性

---

## 参与贡献

### 开发

```bash
git clone https://github.com/yourname/hpm.git
cd hpm
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### 代码风格

- C++20
- 四空格缩进
- `.clang-format` 已配置
- 所有函数返回 `Result<T>`，不抛异常

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

### 文档

`docs/` 目录下：
- `architecture.md`：架构说明
- `project_json.md`：project.json 格式
- `driver_manifest.md`：驱动清单格式
- `chip_driver.md`：如何写芯片驱动
- `device_driver.md`：如何写设备驱动

