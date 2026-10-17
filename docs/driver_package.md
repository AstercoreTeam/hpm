# 驱动包格式规范

## 目录结构

```
<name>-<version>/
├── manifest.json       驱动清单（驱动包必须）
│   或 chip.json        芯片清单（芯片驱动包必须）
├── LICENSE             许可证（必须）
├── README.md           说明（可选）
├── src/                源码（.c，必须）
├── include/            头文件（.h，必须）
├── ld/                 链接脚本（ARM 芯片必须）
└── test/               测试用例（hpm-dev 用，可选）
```

## manifest.json（驱动包）

```json
{
  "name": "rc522",
  "version": "1.2.0",
  "description": "RC522 RFID reader",
  "license": "MIT",
  "requires": {
    "bus": { "type": "spi", "methods": ["init", "transfer"] },
    "cs":  { "type": "gpio_out", "methods": ["init", "write"] }
  },
  "provides": {
    "methods": [
      { "name": "init", "return": "int", "params": [] },
      { "name": "read_card", "return": "int", "params": [] }
    ]
  }
}
```

### 字段说明

| 字段 | 必填 | 说明 |
|------|------|------|
| `name` | 是 | 驱动名，跟目录名一致 |
| `version` | 是 | semver |
| `description` | 否 | 说明 |
| `license` | 是 | SPDX 标识（`MIT`、`Apache-2.0`、`BSD-3-Clause`…） |
| `requires` | 否 | 资源需求 |
| `provides` | 否 | 提供的方法 |

### requires

每个 key 对应 `hpm_dev.h` 里的宏名。

```json
"requires": {
  "bus": { "type": "spi", "methods": ["init", "transfer"] },
  "cs":  { "type": "gpio_out", "methods": ["init", "write"], "optional": false }
}
```

| 字段 | 必填 | 说明 |
|------|------|------|
| `type` | 是 | 驱动名（如 `spi`）或引脚能力（如 `gpio_out`） |
| `methods` | 否 | 需要下层驱动提供的方法 |
| `optional` | 否 | 可选资源，默认 `false` |

**`type` 是驱动名时**：工具递归创建中间设备。
**`type` 是引脚能力时**：从上层 bindings 取引脚。

### provides

```json
"provides": {
  "methods": [
    { "name": "init", "return": "int", "params": [] },
    {
      "name": "read_card",
      "return": "int",
      "params": [
        { "type": "unsigned char*", "name": "uid" }
      ]
    }
  ]
}
```

| 字段 | 必填 | 说明 |
|------|------|------|
| `name` | 是 | 方法名，生成 `<device>_<name>` |
| `return` | 否 | 返回类型，默认 `int` |
| `params` | 否 | 参数列表，每项 `{ type, name }` |

## chip.json（芯片驱动包）

```json
{
  "name": "ch552g",
  "family": "ch55x",
  "package": "SOP16",
  "license": "MIT",
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
    "P1_4": { "capabilities": [{ "type": "gpio_out" }] }
  }
}
```

### 字段说明

| 字段 | 必填 | 说明 |
|------|------|------|
| `name` | 是 | 芯片型号 |
| `family` | 否 | 芯片系列 |
| `package` | 否 | 封装 |
| `license` | 是 | SPDX 标识 |
| `build` | 是 | 构建配置 |
| `toolchain` | 是 | 工具链 |
| `flash` | 是 | 烧录配置 |
| `provides` | 否 | 芯片提供的方法（如 `init`） |
| `pins` | 是 | 引脚能力表 |

### build

```json
"build": {
  "cpu": "cortex-m3",
  "flags": ["-mthumb", "-mcpu=cortex-m3", "-nostdlib", "-nostartfiles"],
  "ldscript": "ld/stm32f103c8t6.ld",
  "startup": "src/startup.c"
}
```

| 字段 | 说明 |
|------|------|
| `cpu` | CPU 类型 |
| `flags` | 编译选项 |
| `ldscript` | 链接脚本相对路径（ARM 必须，8051 留空） |
| `startup` | 启动文件相对路径（ARM 必须，8051 留空） |

### toolchain

```json
"toolchain": {
  "cc": "sdcc",
  "objcopy": "packihx",
  "size": ""
}
```

| 字段 | 说明 |
|------|------|
| `cc` | 编译器 |
| `objcopy` | objcopy 工具 |
| `size` | size 工具（可选） |
| `prefix` | 前缀（可选，如 `arm-none-eabi-`） |
| `name` | 兼容旧字段 |

### flash

```json
"flash": {
  "tool": "st-flash",
  "args": ["write", "{bin}", "0x8000000"]
}
```

占位符：`{bin}`、`{elf}`、`{hex}`。

### pins

```json
"pins": {
  "PA5": {
    "shareable": true,
    "capabilities": [
      { "type": "spi_sck", "instance": "SPI1" },
      { "type": "gpio_out" }
    ]
  }
}
```

| 字段 | 说明 |
|------|------|
| `shareable` | 是否可共享（总线信号设为 `true`） |
| `capabilities` | 能力列表 |

能力类型：
- `gpio_out` / `gpio_in`
- `spi_sck` / `spi_miso` / `spi_mosi`
- `i2c_scl` / `i2c_sda`
- `uart_tx` / `uart_rx`

## 头文件规范

- 头文件必须**自包含**（不缺常量、不缺结构体）
- 共享数据用 `extern` 声明，`.c` 里定义
- 用 `#pragma once`
- **SDCC 兼容**：用 `__sfr __at(...)`，不用 Keil 的 `sfr = ...`
- `bit` 用 `__bit`，`data` 用 `__data`，`code` 用 `__code`

## 源码规范

- 驱动源码用 `HPM_FN` 拼接宏
- 不调 `printf`、`malloc`（裸机无 libc）
- 引脚操作通过 `HPM_CALL` 宏

## 许可证

每个包必须带 `LICENSE` 文件。`manifest.json` / `chip.json` 里 `license` 字段写 SPDX 标识。

## 校验

```bash
hpm-dev validate <驱动包目录>
```

检查：
- `manifest.json` 或 `chip.json` 存在且格式正确
- `LICENSE` 文件存在
- `src/` 和 `include/` 目录存在
- `.c` 文件在 `src/`，`.h` 在 `include/`
- `license` 字段存在