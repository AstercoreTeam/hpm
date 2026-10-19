# HPM 芯片驱动包开发指南（给 AI 的技能文档）

## 一、你要做什么

你要为**一颗 MCU 芯片**写一个 HPM 芯片驱动包。芯片驱动包是 HPM 里最底层、唯一直接操作寄存器的驱动，它提供：

- **引脚能力表**：每个引脚能做什么（`gpio_out`、`spi_sck`、`i2c_scl` 等）
- **引脚操作函数**：`hpm_pin_<PIN>_init`、`hpm_pin_<PIN>_write`、`hpm_pin_<PIN>_read`
- **芯片初始化**：时钟、电源、看门狗等
- **工具链声明**：用哪个编译器（`sdcc`、`arm-none-eabi-gcc`、`clang`）
- **烧录配置**：用什么工具、什么地址
- **构建配置**：编译选项、链接脚本、启动文件

**芯片驱动包是唯一接触硬件的层。** 上层总线驱动、设备驱动都建立在它之上。

---

## 二、目录结构

```
<chip>-<version>/
├── chip.json
├── LICENSE
├── README.md
├── src/
│   ├── <chip>.c
│   ├── hpm_pin.c
│   └── startup.c          （ARM 必须，8051 不需要）
├── include/
│   ├── <chip>.h
│   ├── hpm_pin.h
│   └── <chip_vendor>.h    （厂商 SFR 头文件，如 CH552.H、stm32f1xx.h）
└── ld/
    └── <chip>.ld          （ARM 必须，8051 不需要）
```

- `<chip>`：芯片型号，小写，跟目录名一致（如 `stm32f103c8t6`、`ch552g`、`esp32c3`）
- `<version>`：semver（如 `1.0.0`）

---

## 三、`chip.json` 格式

```json
{
  "name": "ch552g",
  "family": "ch55x",
  "package": "SOP16",
  "license": "MIT",
  "build": {
    "cpu": "mcs51",
    "flags": ["-mmcs51", "--model-small", "-DF_CPU=24000000L"],
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
    "P1_4": { "capabilities": [{ "type": "gpio_out" }, { "type": "adc", "channel": 1 }] }
  }
}
```

### 字段说明

| 字段 | 必填 | 说明 |
|------|------|------|
| `name` | 是 | 芯片型号，跟目录名一致 |
| `family` | 否 | 芯片系列（如 `stm32f1`、`ch55x`） |
| `package` | 否 | 封装（如 `LQFP48`、`SOP16`） |
| `license` | 是 | SPDX 标识 |
| `build` | 是 | 构建配置 |
| `toolchain` | 是 | 工具链 |
| `flash` | 是 | 烧录配置 |
| `provides` | 否 | 芯片提供的方法 |
| `pins` | 是 | 引脚能力表 |

### `build`

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
| `cpu` | CPU 类型（`cortex-m3`、`mcs51`、`riscv32` 等） |
| `flags` | 编译选项（**必须包含工具链特有的选项**，如 `-mmcs51 --model-small` 或 `-mthumb -mcpu=cortex-m3`） |
| `ldscript` | 链接脚本相对路径（ARM 必须，8051 留空） |
| `startup` | 启动文件相对路径（ARM 必须，8051 留空） |

**注意**：`flags` 里**必须包含工具链的核心选项**，因为 `Generator` 不会硬编码任何工具链特定选项。

### `toolchain`

```json
"toolchain": {
  "cc": "sdcc",
  "objcopy": "packihx",
  "size": ""
}
```

| 字段 | 说明 |
|------|------|
| `cc` | 编译器（`sdcc`、`arm-none-eabi-gcc`、`clang`、`riscv-none-elf-gcc`） |
| `objcopy` | objcopy 工具（`packihx`、`arm-none-eabi-objcopy`、`llvm-objcopy`） |
| `size` | size 工具（可选） |

**`Generator` 根据 `cc` 判断 Makefile 风格**：`sdcc` → SDCC 风格（`.rel`、`.ihx`），其他 → GCC 风格（`.o`、`.elf`）。

### `flash`

```json
"flash": {
  "tool": "st-flash",
  "args": ["write", "{bin}", "0x8000000"]
}
```

**占位符**：`{bin}`、`{elf}`、`{hex}`。

### `provides`

```json
"provides": {
  "methods": [
    { "name": "init", "return": "void", "params": [] }
  ]
}
```

**芯片初始化方法**，会生成 `<chip>_init()`，被 `hpm_init_all()` 调用。

**注意**：芯片的 `provides` 里只放 `init` 这类全局初始化，**不要放引脚操作**（引脚操作由 `hpm_pin.c` 提供，通过 `hpm_pin_<PIN>_<method>` 命名）。

### `pins`

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
| `shareable` | 是否可共享（总线信号如 SCK/MISO/MOSI 设为 `true`） |
| `capabilities` | 能力列表 |

**能力类型**：

| 类型 | 说明 | 典型 `shareable` |
|------|------|-----------------|
| `gpio_out` | 推挽输出 | `false` |
| `gpio_in` | 输入 | `false` |
| `spi_sck` | SPI 时钟 | `true` |
| `spi_miso` | SPI 主入从出 | `true` |
| `spi_mosi` | SPI 主出从入 | `true` |
| `spi_cs` | SPI 片选（可选，通常用 gpio_out） | `false` |
| `i2c_scl` | I2C 时钟 | `true` |
| `i2c_sda` | I2C 数据 | `true` |
| `uart_tx` | UART 发送 | `false` |
| `uart_rx` | UART 接收 | `false` |
| `pwm` | PWM 输出 | `false` |
| `adc` | ADC 输入 | `false` |

**引脚命名**：
- ARM（STM32）：`PA0`、`PA1`、`PB0` 等
- 8051（CH552）：`P1_1`、`P3_0` 等（**不能用 `P1.1`，`.` 不能做 C 宏名**）
- RISC-V（ESP32-C3）：`GPIO0`、`GPIO1` 等
- 其他：按数据手册命名

---

## 四、源码写法

### 4.1 `include/<chip>.h`

```c
#pragma once

#include <stdint.h>
#include "<vendor_header>.h"   // 厂商 SFR 头文件

void <chip>_init(void);
```

**示例（CH552）**：

```c
#pragma once

#include <stdint.h>
#include "CH552.H"

void ch552g_init(void);
```

**示例（STM32）**：

```c
#pragma once

#include <stdint.h>
#include "stm32f1xx.h"

void stm32f103c8t6_init(void);
```

### 4.2 `src/<chip>.c`

```c
#include "<chip>.h"

void <chip>_init(void) {
    // 时钟配置
    // 电源配置
    // 外设初始化（可选）
}
```

**示例（CH552）**：

```c
#include "ch552g.h"

static void clock_24m(void) {
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_SEL) | 0x06;   // 24MHz
    SAFE_MOD = 0x00;
}

void ch552g_init(void) {
    clock_24m();
}
```

**示例（STM32）**：

```c
#include "stm32f103c8t6.h"

static void clock_72m(void) {
    // HSE 8MHz → PLL ×9 → 72MHz
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

void stm32f103c8t6_init(void) {
    clock_72m();
}
```

### 4.3 `include/hpm_pin.h`

```c
#pragma once
#include <stdint.h>

// 每个引脚提供三个函数
void hpm_pin_<PIN>_init(void);
void hpm_pin_<PIN>_write(int level);
int  hpm_pin_<PIN>_read(void);
```

**示例（CH552）**：

```c
#pragma once
#include <stdint.h>

void hpm_pin_P1_1_init(void);
void hpm_pin_P1_1_write(int level);
int  hpm_pin_P1_1_read(void);

void hpm_pin_P3_0_init(void);
void hpm_pin_P3_0_write(int level);
int  hpm_pin_P3_0_read(void);

// ...
```

### 4.4 `src/hpm_pin.c`

**为每个引脚实现 `hpm_pin_<PIN>_init`、`hpm_pin_<PIN>_write`、`hpm_pin_<PIN>_read`。**

**示例（CH552）**：

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

**示例（STM32）**：

```c
#include "hpm_pin.h"
#include "stm32f103c8t6.h"

#define DEF_PIN(port, pin) \
    void hpm_pin_##port##pin##_init(void) { \
        RCC->APB2ENR |= RCC_APB2ENR_IOP##port##EN; \
        volatile uint32_t* cr = (pin < 8) ? &GPIO##port->CRL : &GPIO##port->CRH; \
        uint32_t shift = (pin % 8) * 4; \
        *cr &= ~(0xF << shift); \
        *cr |= (0x2 << shift);   /* 输出 2MHz */ \
    } \
    void hpm_pin_##port##pin##_write(int level) { \
        if (level) GPIO##port->BSRR = (1 << pin); \
        else       GPIO##port->BSRR = (1 << (pin + 16)); \
    } \
    int hpm_pin_##port##pin##_read(void) { \
        return (GPIO##port->IDR >> pin) & 1; \
    }

DEF_PIN(A, 4)
DEF_PIN(A, 5)
DEF_PIN(A, 6)
DEF_PIN(A, 7)
DEF_PIN(B, 0)
```

**注意**：STM32 的 `GPIOA` 等是宏，`DEF_PIN(A, 4)` 展开成 `GPIOA`、`hpm_pin_A4_init`。**引脚名 `PA4` 对应 `DEF_PIN(A, 4)`，生成 `hpm_pin_A4_init`，但工具生成的宏是 `hpm_pin_PA4`，所以要改成 `hpm_pin_PA4_init`。**

**正确写法**：

```c
#define DEF_PIN(name, port, pin) \
    void hpm_pin_##name##_init(void) { \
        RCC->APB2ENR |= RCC_APB2ENR_IOP##port##EN; \
        volatile uint32_t* cr = (pin < 8) ? &GPIO##port->CRL : &GPIO##port->CRH; \
        uint32_t shift = (pin % 8) * 4; \
        *cr &= ~(0xF << shift); \
        *cr |= (0x2 << shift); \
    } \
    void hpm_pin_##name##_write(int level) { \
        if (level) GPIO##port->BSRR = (1 << pin); \
        else       GPIO##port->BSRR = (1 << (pin + 16)); \
    } \
    int hpm_pin_##name##_read(void) { \
        return (GPIO##port->IDR >> pin) & 1; \
    }

DEF_PIN(PA4, A, 4)
DEF_PIN(PA5, A, 5)
DEF_PIN(PA6, A, 6)
DEF_PIN(PA7, A, 7)
```

### 4.5 `src/startup.c`（ARM 必须）

**最小 Cortex-M 启动文件**：

```c
#include <stdint.h>

extern int main(void);
extern uint32_t _estack;
extern uint32_t _sdata, _edata, _sidata;
extern uint32_t _sbss, _ebss;

void Reset_Handler(void);
void Default_Handler(void);

__attribute__((section(".isr_vector"), used))
void (*const g_vectors[])(void) = {
    (void (*)(void))&_estack,
    Reset_Handler,
    Default_Handler,    // NMI
    Default_Handler,    // HardFault
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
};

void Reset_Handler(void) {
    uint32_t* src = &_sidata;
    uint32_t* dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;

    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;

    main();
    while (1) {}
}

void Default_Handler(void) {
    while (1) {}
}
```

### 4.6 `ld/<chip>.ld`（ARM 必须）

**最小链接脚本**：

```
ENTRY(Reset_Handler)

MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS {
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text : { *(.text*) *(.rodata*) } > FLASH

    _sidata = LOADADDR(.data);
    .data : { _sdata = .; *(.data*) _edata = .; } > RAM AT > FLASH
    .bss : { _sbss = .; *(.bss*) *(COMMON) _ebss = .; } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
```

**地址和大小按芯片数据手册改。**

---

## 五、关键规则

**1. 引脚命名不能有 `.`**

`P1.1` → `P1_1`，`PA.5` → `PA5`。C 宏名不能有 `.`。

**2. 引脚函数命名必须 `hpm_pin_<PIN>_<method>`**

工具生成的 `hpm_dev.h` 里 `#define cs hpm_pin_PA4`，驱动源码 `HPM_CALL(cs, init)` 展开成 `hpm_pin_PA4_init`。所以引脚函数必须是 `hpm_pin_<PIN>_init`、`hpm_pin_<PIN>_write`、`hpm_pin_<PIN>_read`。

**3. `build.flags` 必须包含工具链核心选项**

`sdcc` → `-mmcs51 --model-small`，`arm-none-eabi-gcc` → `-mthumb -mcpu=cortex-m3`。**`Generator` 不硬编码任何工具链选项。**

**4. 厂商 SFR 头文件放 `include/`**

如 `CH552.H`、`stm32f1xx.h`。**用 SDCC 兼容版**（`__sfr __at(...)`），不用 Keil 版（`sfr = ...`）。

**5. 共享数据用 `extern`**

头文件 `extern` 声明，`.c` 里定义。避免多个 `.c` include 导致重复符号。

**6. 芯片初始化只做全局配置**

时钟、电源、看门狗。**不要初始化具体引脚**，引脚初始化由 `hpm_pin.c` 的 `hpm_pin_<PIN>_init` 负责。

**7. `shareable` 只给总线信号**

`spi_sck`、`spi_miso`、`spi_mosi`、`i2c_scl`、`i2c_sda` 设为 `true`，其他 `false`。

**8. 引脚能力要完整**

每个引脚列出所有能力。如 `PA5` 能做 `spi_sck`、`gpio_out`、`tim_ch`，都列出来。

---

## 六、完整示例：CH552G 芯片驱动

### `chip.json`

```json
{
  "name": "ch552g",
  "family": "ch55x",
  "package": "SOP16",
  "license": "MIT",
  "build": {
    "cpu": "mcs51",
    "flags": ["-mmcs51", "--model-small", "-DF_CPU=24000000L"],
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
    "P1_1": { "capabilities": [{ "type": "gpio_out" }, { "type": "adc", "channel": 0 }] },
    "P1_4": { "capabilities": [{ "type": "gpio_out" }, { "type": "adc", "channel": 1 }] },
    "P1_5": { "capabilities": [{ "type": "gpio_out" }, { "type": "pwm" }] },
    "P3_0": { "capabilities": [{ "type": "gpio_out" }, { "type": "uart_rx" }] },
    "P3_1": { "capabilities": [{ "type": "gpio_out" }, { "type": "uart_tx" }] },
    "P3_2": { "capabilities": [{ "type": "gpio_out" }, { "type": "adc", "channel": 3 }] },
    "P3_3": { "capabilities": [{ "type": "gpio_out" }] },
    "P3_4": { "capabilities": [{ "type": "gpio_out" }, { "type": "pwm" }] }
  }
}
```

### `include/ch552g.h`

```c
#pragma once

#include <stdint.h>
#include "CH552.H"

void ch552g_init(void);
```

### `include/hpm_pin.h`

```c
#pragma once
#include <stdint.h>

void hpm_pin_P1_1_init(void);
void hpm_pin_P1_1_write(int level);
int  hpm_pin_P1_1_read(void);

void hpm_pin_P1_4_init(void);
void hpm_pin_P1_4_write(int level);
int  hpm_pin_P1_4_read(void);

// ... 其他引脚
```

### `src/ch552g.c`

```c
#include "ch552g.h"

static void clock_24m(void) {
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_SEL) | 0x06;
    SAFE_MOD = 0x00;
}

void ch552g_init(void) {
    clock_24m();
}
```

### `src/hpm_pin.c`

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

---

## 七、完整示例：STM32F103C8T6 芯片驱动

### `chip.json`

```json
{
  "name": "stm32f103c8t6",
  "family": "stm32f1",
  "package": "LQFP48",
  "license": "MIT",
  "build": {
    "cpu": "cortex-m3",
    "flags": ["-mthumb", "-mcpu=cortex-m3", "-nostdlib", "-nostartfiles"],
    "ldscript": "ld/stm32f103c8t6.ld",
    "startup": "src/startup.c"
  },
  "toolchain": {
    "cc": "arm-none-eabi-gcc",
    "objcopy": "arm-none-eabi-objcopy",
    "size": "arm-none-eabi-size"
  },
  "flash": {
    "tool": "st-flash",
    "args": ["write", "{bin}", "0x8000000"]
  },
  "provides": {
    "methods": [
      { "name": "init", "return": "void", "params": [] }
    ]
  },
  "pins": {
    "PA4": { "capabilities": [{ "type": "gpio_out" }] },
    "PA5": { "shareable": true, "capabilities": [{ "type": "spi_sck", "instance": "SPI1" }] },
    "PA6": { "shareable": true, "capabilities": [{ "type": "spi_miso", "instance": "SPI1" }] },
    "PA7": { "shareable": true, "capabilities": [{ "type": "spi_mosi", "instance": "SPI1" }] },
    "PB0": { "capabilities": [{ "type": "gpio_out" }] }
  }
}
```

### 文件清单

- `include/stm32f103c8t6.h`：include `stm32f1xx.h`，声明 `stm32f103c8t6_init`
- `include/hpm_pin.h`：引脚函数声明
- `include/stm32f1xx.h`：厂商头文件（从 ST 或开源项目复制）
- `src/stm32f103c8t6.c`：时钟配置
- `src/hpm_pin.c`：引脚操作
- `src/startup.c`：启动文件
- `ld/stm32f103c8t6.ld`：链接脚本

---

## 八、开发流程

### 1. 初始化

```bash
hpm-dev init mychip --type chip
```

生成：

```
mychip-0.1.0/
├── chip.json
├── LICENSE
├── README.md
├── src/
│   ├── mychip.c
│   ├── hpm_pin.c
│   └── startup.c
├── include/
│   ├── mychip.h
│   └── hpm_pin.h
└── ld/
    └── mychip.ld
```

### 2. 编辑 `chip.json`

填 `build`、`toolchain`、`flash`、`provides`、`pins`。

### 3. 复制厂商头文件

`CH552.H`、`stm32f1xx.h` 等，从厂商或开源项目下载，放到 `include/`。

### 4. 实现 `src/<chip>.c`

时钟、电源、看门狗。

### 5. 实现 `src/hpm_pin.c`

每个引脚的 `init`、`write`、`read`。

### 6. 校验

```bash
hpm-dev validate mychip-0.1.0
```

### 7. 复制到 `~/.hpm/packages/`

```bash
cp -r mychip-0.1.0 ~/.hpm/packages/
```

### 8. 测试

`project.json`：

```json
{
  "mcu": "mychip",
  "devices": {}
}
```

```bash
hpm sync --offline
hpm build
```

### 9. 发布

```bash
hpm-dev publish mychip-0.1.0 --key alice.key
```

---

## 九、常见错误

| 错误 | 原因 | 解决 |
|------|------|------|
| `unrecognized command-line option '-mthumb'` | `build.flags` 里有 ARM 选项，但工具链是 `sdcc` | 换芯片，或用 `arm-none-eabi-gcc` |
| `Could not get N consecutive bytes in internal RAM` | 局部变量太大 | 加 `__xdata`（SDCC） |
| `undefined reference to hpm_pin_PA4_init` | 引脚函数没实现 | 在 `hpm_pin.c` 里加 `DEF_PIN(PA4, A, 4)` |
| `Initializer element is not a constant expression` | `__code` 数组用了运行时值 | 用字面量 |
| `cannot open linker script` | `ld/<chip>.ld` 不存在 | 创建链接脚本，或从开源项目复制 |
| `undefined reference to Reset_Handler` | `startup.c` 没加 | 创建启动文件 |
| `sfr P1 = 0x90;` 报错 | Keil 格式的厂商头文件 | 用 SDCC 版（`__sfr __at(0x90) P1;`） |

---

## 十、检查清单

写芯片驱动时逐条检查：

- [ ] `chip.json` 有 `name`、`family`、`package`、`license`
- [ ] `build.flags` 包含工具链核心选项（`-mmcs51 --model-small` 或 `-mthumb -mcpu=cortex-m3`）
- [ ] `build.ldscript` 和 `build.startup` 路径正确（ARM 必须，8051 留空）
- [ ] `toolchain.cc` 是 `sdcc` 或 `arm-none-eabi-gcc` 或 `clang`
- [ ] `flash.tool` 和 `flash.args` 正确
- [ ] `provides.methods` 有 `init`
- [ ] `pins` 里每个引脚有 `capabilities`
- [ ] `shareable: true` 只给总线信号
- [ ] 引脚命名不能有 `.`（`P1_1` 不是 `P1.1`）
- [ ] `hpm_pin.h` 每个引脚有 `init`、`write`、`read`
- [ ] `hpm_pin.c` 每个引脚有实现
- [ ] `src/<chip>.c` 有时钟配置
- [ ] `src/startup.c` 有向量表（ARM）
- [ ] `ld/<chip>.ld` 有 MEMORY 和 SECTIONS（ARM）
- [ ] 厂商头文件是 SDCC 兼容版
- [ ] 共享数据用 `extern`
- [ ] 有 `LICENSE` 文件
- [ ] `hpm-dev validate` 通过

---

## 十一、给 AI 的提示

当你收到「写一个 XXX 芯片的 HPM 芯片驱动」时：

1. **查芯片数据手册**：CPU 架构、时钟、引脚复用、Flash/RAM 大小
2. **确定工具链**：
   - 8051 → `sdcc`
   - ARM Cortex-M → `arm-none-eabi-gcc`
   - RISC-V → `riscv-none-elf-gcc` 或 `clang`
3. **确定 `build.flags`**：
   - SDCC：`-mmcs51 --model-small`
   - ARM：`-mthumb -mcpu=cortex-m3 -nostdlib -nostartfiles`
4. **确定 `flash`**：
   - STM32 → `st-flash write {bin} 0x8000000`
   - CH552 → `wchisp flash {hex}`
   - ESP32 → `esptool.py write_flash 0x0 {bin}`
5. **写 `pins` 表**：按数据手册的引脚复用表
6. **写 `hpm_pin.c`**：每个引脚的 `init`、`write`、`read`
7. **写 `<chip>.c`**：时钟、电源
8. **写 `startup.c` 和 `ld/<chip>.ld`**（ARM）
9. **复制厂商头文件**（SDCC 兼容版）
10. **`hpm-dev validate`**

**不要问用户「用什么引脚」，引脚由用户 `project.json` 配。**

**不要写 `main.c`，只写芯片驱动包。**

**不要硬编码任何工具链选项到 `Generator`，全部放 `build.flags`。**

---

## 十二、参考

- HPM 主仓库：`https://github.com/xzc842/hpm`
- 芯片驱动格式规范：`docs/driver_package.md`
- 示例：
  - `ch552g-1.0.0`（8051，SDCC）
  - `stm32f103c8t6-1.0.0`（ARM，GCC）
- SDCC 兼容头文件：`https://github.com/Blinkinlabs/ch554_sdcc`

**写完芯片驱动后，用 `hpm-dev validate` 检查，然后用 `hpm sync --offline` + `hpm build` 测试。**