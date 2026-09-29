# HPM 驱动包开发指南

本文档面向驱动包开发者，介绍如何为 HPM 编写芯片驱动和外设驱动。

---

## 目录

- [一、驱动包的基本约定](#一驱动包的基本约定)
- [二、宏体系](#二宏体系)
- [三、芯片驱动](#三芯片驱动)
- [四、外设驱动](#四外设驱动)
- [五、模板化机制](#五模板化机制)
- [六、测试与发布](#六测试与发布)
- [七、常见错误](#七常见错误)

---

## 一、驱动包的基本约定

### 1.1 包目录结构

**芯片驱动包**：

```
<chip>-<version>/
├── chip.json               芯片清单（必须）
├── src/
│   ├── <chip>.c            芯片初始化
│   ├── hpm_pin.c           引脚函数实现
│   └── startup.c           启动文件（可选）
├── include/
│   ├── <chip>.h
│   └── hpm_pin.h
└── ld/
    └── <chip>.ld           链接脚本（可选）
```

**外设驱动包**：

```
<driver>-<version>/
├── manifest.json           驱动清单（必须）
├── src/
│   └── <driver>.c
└── include/
    └── <driver>.h
```

### 1.2 包名规则

- 全小写，只能包含字母、数字、下划线
- 芯片驱动用芯片型号：`stm32f103c8t6`
- 外设驱动用器件名或总线名：`rc522`、`bmp280`、`spi`、`i2c`

### 1.3 版本号

遵循 semver：`major.minor.patch`。

- 接口不兼容变更 → major
- 新增功能 → minor
- bug 修复 → patch

---

## 二、宏体系

理解宏体系是写驱动的前提。HPM 用两套宏把「驱动模板」和「具体实例」解耦。

### 2.1 `hpm_dev.h`（工具生成，每个实例一份）

编译驱动源码时，工具会为每个实例生成一份 `hpm_dev.h`，内容只有宏定义：

```c
#pragma once

#define DEVICE_NAME  door_reader
#define bus          __spi_a3f2
#define cs           hpm_pin_PA4
#define rst          hpm_pin_PA3
```

**这些宏名由驱动清单的 `requires` 决定**，工具按 key 生成。

| 宏 | 值 | 说明 |
|----|----|------|
| `DEVICE_NAME` | 用户起的设备名 | 自动生成 |
| `bus` | 中间设备名 | requires 里 key=bus 的值 |
| `cs` | `hpm_pin_PA4` | 引脚，工具加前缀 |
| `rst` | `hpm_pin_PA3` | 同上 |

**驱动包本身不包含 `hpm_dev.h`**。它由工具生成到实例目录，编译时通过 `-I<实例目录>` 找到。

### 2.2 拼接宏（驱动源码自带）

**关键规则：C 预处理器只匹配独立 token。**

`cs` 宏不会匹配 `cs_init` 里的 `cs`。所以驱动源码**不能写** `cs_init()`——它不会被展开。

必须用拼接宏：

```c
#define HPM_CAT_(a, b) a##b
#define HPM_CAT(a, b)  HPM_CAT_(a, b)
#define HPM_FN(name)   HPM_CAT(DEVICE_NAME, HPM_CAT(_, name))
#define HPM_CALL(prefix, method)  HPM_CAT(prefix, HPM_CAT(_, method))
#define HPM_STR_(x)    #x
#define HPM_STR(x)     HPM_STR_(x)
```

**每个驱动源码开头都要写这组宏**（或包含一个公共头）。

用法：

| 写法 | 展开结果 |
|------|---------|
| `HPM_FN(read_card)` | `door_reader_read_card` |
| `HPM_CALL(cs, init)` | `hpm_pin_PA4_init` |
| `HPM_CALL(bus, transfer)` | `__spi_a3f2_transfer` |
| `HPM_STR(DEVICE_NAME)` | `"door_reader"` |

---

## 三、芯片驱动

芯片驱动是**唯一直接接触寄存器的层**。它负责：

1. 声明芯片引脚能力（`chip.json` 的 `pins`）
2. 提供引脚操作函数（`hpm_pin_<PIN>_<method>`）
3. 声明工具链、烧录方式、链接脚本
4. 提供启动文件

### 3.1 `chip.json` 格式

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
    "PA5": {
      "shareable": true,
      "capabilities": [{ "type": "spi_sck", "instance": "SPI1" }]
    },
    "PA6": {
      "shareable": true,
      "capabilities": [{ "type": "spi_miso", "instance": "SPI1" }]
    },
    "PA7": {
      "shareable": true,
      "capabilities": [{ "type": "spi_mosi", "instance": "SPI1" }]
    }
  }
}
```

### 3.2 字段说明

| 字段 | 必填 | 说明 |
|------|------|------|
| `name` | ✅ | 芯片型号，跟包名一致 |
| `family` | ❌ | 芯片系列 |
| `package` | ❌ | 封装 |
| `build` | ❌ | 编译配置 |
| `build.cpu` | ❌ | CPU 类型 |
| `build.flags` | ❌ | 额外编译选项 |
| `build.ldscript` | ❌ | 链接脚本相对路径 |
| `build.startup` | ❌ | 启动文件相对路径 |
| `toolchain.name` | ✅ | 工具链名，如 `arm-none-eabi-gcc` |
| `flash.tool` | ✅ | 烧录器命令 |
| `flash.args` | ❌ | 参数数组，支持 `{bin}`/`{elf}`/`{hex}` 占位符 |
| `pins` | ✅ | 引脚表 |

### 3.3 引脚能力类型

| 类型 | 含义 | 典型 shareable |
|------|------|---------------|
| `gpio_out` | 数字输出 | false |
| `gpio_in` | 数字输入 | false |
| `spi_sck` | SPI 时钟 | true |
| `spi_miso` | SPI 主入从出 | true |
| `spi_mosi` | SPI 主出从入 | true |
| `i2c_scl` | I2C 时钟 | true |
| `i2c_sda` | I2C 数据 | true |
| `uart_tx` | UART 发送 | false |
| `uart_rx` | UART 接收 | false |

**shareable** 的含义：该引脚被一个设备占用后，是否还能被其他设备共享。总线信号（SCK/MISO/MOSI/SCL/SDA）应设为 `true`，其余 `false`。

### 3.4 引脚函数实现

芯片驱动必须为每个引脚提供 `<prefix>_init`、`<prefix>_write`、`<prefix>_read` 三组函数。`<prefix>` 是工具生成的 `hpm_dev.h` 里 `cs` 宏的值（如 `hpm_pin_PA4`）。

**命名约定**：`hpm_pin_<PIN>_<method>`。

```c
// src/hpm_pin.c
#include "hpm_pin.h"
#include <stdint.h>

// STM32F103 GPIOA 寄存器（真实芯片驱动）
#define GPIOA_BASE   0x40010800UL
#define GPIOA_CRL    (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_BSRR   (*(volatile uint32_t*)(GPIOA_BASE + 0x10))

// 配置引脚为推挽输出
static void pin_config_output(int pin) {
    int shift = (pin % 8) * 4;
    uint32_t val = GPIOA_CRL;
    val &= ~(0xFUL << shift);
    val |=  (0x2UL << shift);
    GPIOA_CRL = val;
}

// 写引脚
static void pin_write(int pin, int level) {
    if (level) GPIOA_BSRR = (1UL << pin);
    else       GPIOA_BSRR = (1UL << (pin + 16));
}

// 读引脚（输入模式才能读，这里简化）
static int pin_read(int pin) {
    return 0;
}

// 用宏批量生成
#define DEF_PIN(name, port, pin_num) \
    void hpm_pin_##name##_init(void)       { pin_config_output(pin_num); } \
    void hpm_pin_##name##_write(int level) { pin_write(pin_num, level); } \
    int  hpm_pin_##name##_read(void)       { return pin_read(pin_num); }

DEF_PIN(PA4, A, 4)
DEF_PIN(PA5, A, 5)
DEF_PIN(PA6, A, 6)
DEF_PIN(PA7, A, 7)
DEF_PIN(PA8, A, 8)
```

**注意**：

- 不要用 `printf`、`malloc` 等 libc 函数
- 不要依赖中断（除非芯片驱动自己处理）
- 引脚函数应该**只有硬件操作**，不含业务逻辑

### 3.5 `hpm_pin.h`

为每个引脚声明函数：

```c
#pragma once
#include <stdint.h>

void hpm_pin_PA4_init(void);
void hpm_pin_PA4_write(int level);
int  hpm_pin_PA4_read(void);

void hpm_pin_PA5_init(void);
void hpm_pin_PA5_write(int level);
int  hpm_pin_PA5_read(void);

// ...
```

### 3.6 芯片初始化

```c
// src/stm32f103c8t6.c
#include "stm32f103c8t6.h"

void stm32f103c8t6_init(void) {
    // 开时钟、配置 PLL、初始化必要外设
    // 如果引脚函数自己处理，这里可以空
}
```

### 3.7 启动文件

最小 Cortex-M 启动文件（`src/startup.c`）：

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
    // ... 完整中断向量
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

### 3.8 链接脚本

最小 Cortex-M3 链接脚本（`ld/stm32f103c8t6.ld`）：

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
    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
```

---

## 四、外设驱动

外设驱动建立在芯片驱动之上，**不直接碰寄存器**，只调用：

- 引脚函数（`HPM_CALL(cs, write)(1)`）
- 总线驱动（`HPM_CALL(bus, transfer)(...)`）

### 4.1 `manifest.json` 格式

```json
{
  "name": "rc522",
  "version": "1.2.0",
  "description": "RC522 RFID reader",

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

### 4.2 字段说明

| 字段 | 说明 |
|------|------|
| `name` | 驱动名，跟包名一致 |
| `version` | semver |
| `requires` | 需求表，key 是资源名，会成为 `hpm_dev.h` 里的宏名 |
| `requires.<key>.type` | 资源类型 |
| `requires.<key>.methods` | 需要下层驱动提供的方法 |
| `requires.<key>.optional` | 是否可选 |
| `provides.methods` | 本驱动对外提供的方法列表 |

### 4.3 `type` 的两种语义

**这是最重要的区分**：

1. **`type` 是一个驱动名**（如 `"spi"`、`"i2c"`）→ 工具递归创建中间设备。
   - 中间设备的引脚从**顶层用户设备**的 bindings 里按 key 取。
   - 例如 `spi` 驱动 requires `sck`/`miso`/`mosi`，工具会从用户设备里找同名字段。

2. **`type` 是引脚能力**（如 `"gpio_out"`）→ 工具直接从用户设备的 bindings 里按 key 取值当引脚。
   - `cs` key 对应的值必须是引脚名（`PA4`）。

### 4.4 驱动源码模板

```c
// src/rc522.c
#include "rc522.h"
#include <stddef.h>

// ── 拼接宏（每个驱动自带）──
#define HPM_CAT_(a, b) a##b
#define HPM_CAT(a, b)  HPM_CAT_(a, b)
#define HPM_FN(name)   HPM_CAT(DEVICE_NAME, HPM_CAT(_, name))
#define HPM_CALL(prefix, method)  HPM_CAT(prefix, HPM_CAT(_, method))

// ── init ──
int HPM_FN(init)(void) {
    // 初始化 CS
    HPM_CALL(cs, init)();
    HPM_CALL(cs, write)(1);

    // 初始化 RST（可选）
#ifdef rst
    HPM_CALL(rst, init)();
    HPM_CALL(rst, write)(1);
#endif

    // 初始化总线
    HPM_CALL(bus, init)();

    return 0;
}

// ── read_card ──
int HPM_FN(read_card)(void) {
    unsigned char tx[2] = {0x01, 0x02};
    unsigned char rx[2] = {0};
    HPM_CALL(bus, transfer)(tx, rx, sizeof(tx));
    return (int)rx[0];
}
```

### 4.5 `rc522.h`

```c
#pragma once

#include "hpm_dev.h"
```

**注意**：`hpm_dev.h` 由工具生成，驱动包里没有。

### 4.6 可选资源的处理

`optional: true` 的 requires，用户可以不提供。

驱动源码里用 `#ifdef` 检查宏是否存在：

```c
#ifdef rst
    HPM_CALL(rst, init)();
    HPM_CALL(rst, write)(1);
#endif
```

用户没配 `rst` 时，`hpm_dev.h` 里不会有 `#define rst`，`#ifdef` 就为假。

### 4.7 总线驱动的写法

总线驱动（spi/i2c/uart）跟外设驱动**写法一样**，只是它 requires 的是引脚能力：

```json
{
  "name": "spi",
  "version": "1.0.0",
  "requires": {
    "sck":  { "type": "spi_sck" },
    "miso": { "type": "spi_miso" },
    "mosi": { "type": "spi_mosi" }
  },
  "provides": {
    "methods": ["init", "transfer"]
  }
}
```

```c
// src/spi.c
#include "spi.h"
#include <stddef.h>

#define HPM_CAT_(a, b) a##b
#define HPM_CAT(a, b)  HPM_CAT_(a, b)
#define HPM_FN(name)   HPM_CAT(DEVICE_NAME, HPM_CAT(_, name))
#define HPM_CALL(prefix, method)  HPM_CAT(prefix, HPM_CAT_(method))

int HPM_FN(init)(void) {
    HPM_CALL(sck, init)();
    HPM_CALL(miso, init)();
    HPM_CALL(mosi, init)();
    return 0;
}

int HPM_FN(transfer)(const unsigned char* tx,
                     unsigned char* rx,
                     size_t len) {
    for (size_t i = 0; i < len; ++i) {
        rx[i] = tx[i];
    }
    return 0;
}
```

**注意**：这里 `sck`/`miso`/`mosi` 宏的值是 `hpm_pin_PA5` 等，`HPM_CALL(sck, init)` 展开成 `hpm_pin_PA5_init`——芯片驱动提供。

---

## 五、模板化机制

### 5.1 一个驱动，多个实例

用户可能声明多个同驱动的设备：

```json
{
  "devices": {
    "door_reader": { "driver": "rc522", "cs": "PA4", "sck": "PA5", ... },
    "gate_reader": { "driver": "rc522", "cs": "PA8", "sck": "PA5", ... }
  }
}
```

工具会：
1. 为每个实例建目录：`driver/door_reader/`、`driver/gate_reader/`
2. 每个目录里拷一份驱动源码 `rc522.c`
3. 每个目录里生成一份 `hpm_dev.h`，`DEVICE_NAME` 不同
4. 编译时 `-Idoor_reader` 和 `-Igate_reader` 分别编译
5. 生成的符号：`door_reader_read_card`、`gate_reader_read_card`

**驱动源码只写一份**，靠 `-I` 切换实例。

### 5.2 中间设备自动创建

用户写：

```json
{
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

`rc522` requires `bus`（type=spi），工具：
1. 识别 `spi` 是一个驱动
2. 找 `spi` 驱动的 requires：`sck`/`miso`/`mosi`
3. 从 `door_reader` 的 bindings 里取 `sck`/`miso`/`mosi` 的值
4. 创建中间设备 `__spi_xxxx`，绑定这三个引脚
5. `door_reader.bus` 指向 `__spi_xxxx`
6. 生成两个 `hpm_dev.h`：
   - `door_reader/hpm_dev.h`：`#define bus __spi_xxxx`
   - `__spi_xxxx/hpm_dev.h`：`#define sck hpm_pin_PA5`

### 5.3 同名 key 冲突

如果两个驱动 requires 同名 key（比如都叫 `sck`），且都从**同一个用户设备**取，会报错。

解决：用不同的 key 名，或把其中一个放到中间设备。

---

## 六、测试与发布

### 6.1 本地测试

把包放到 `~/.hpm/packages/<name>-<version>/`，然后：

```bash
cd /tmp/test-project
cat > project.json <<'EOF'
{
  "mcu": "stm32f103c8t6",
  "devices": {
    "my_dev": {
      "driver": "mydevice",
      "cs": "PA4",
      "sck": "PA5",
      "miso": "PA6",
      "mosi": "PA7"
    }
  }
}
EOF

hpm sync
hpm build
```

### 6.2 检查清单

- [ ] `manifest.json` / `chip.json` 字段完整
- [ ] 所有 `requires` key 在用户设备里都有对应值（或用 `optional`）
- [ ] `provides.methods` 里的方法在源码里都有 `HPM_FN` 实现
- [ ] 驱动源码只用 `HPM_CALL` 调下层，不用裸 `cs_init`
- [ ] 不调 `printf`、`malloc`、`free`
- [ ] 芯片驱动提供所有声明的引脚的函数
- [ ] 版本号符合 semver

### 6.3 未来的发布流程

`hpm-dev` 工具（未实现）会支持：
- `hpm-dev init`：建包骨架
- `hpm-dev test`：多芯片测试（QEMU + 真机）
- `hpm-dev validate`：校验清单和接口
- `hpm-dev publish`：发布到 registry

---

## 七、常见错误

### 7.1 `undefined reference to cs_init`

**原因**：驱动源码写了 `cs_init()`，没有用 `HPM_CALL`。

**修**：改成 `HPM_CALL(cs, init)()`。

**根本原因**：C 预处理器不展开 `cs_init` 里的 `cs`。

### 7.2 `undefined reference to hpm_pin_PA4_init`

**原因**：芯片驱动没提供 `hpm_pin_PA4_init`。

**修**：在芯片驱动的 `hpm_pin.c` 里加 `DEF_PIN(PA4, A, 4)`。

### 7.3 引脚冲突

**原因**：两个设备用同一个不可共享的引脚。

**修**：检查 `project.json`，把冲突的引脚改掉；或把芯片驱动里该引脚的 `shareable` 改为 `true`（如果是总线信号）。

### 7.4 方法能力不匹配

**原因**：A 驱动 requires `bus.transfer`，但 `bus` 指向的驱动没提供 `transfer`。

**修**：检查被引用驱动的 `manifest.json` 里的 `provides.methods`。

### 7.5 循环依赖

**原因**：A requires B，B requires A。

**修**：重新设计依赖关系，引入一个中间层。

### 7.6 `build.ldscript` 找不到

**原因**：芯片驱动包的 `ld/` 目录没被拷贝到实例目录。

**修**：确认 `chip.json` 里 `build.ldscript` 的相对路径正确，工具会拷 `ld/` 目录。

---

## 附录：HPM 内部处理流程

```
project.json
    ↓ 读
Project
    ↓ resolver
DriverState（user_devices + intermediate_devices）
    ↓ capability check
    ↓ generator
driver/
├── driver.h
├── driver.c
├── Makefile
├── <instance>/          （每个实例一份）
│   ├── hpm_dev.h
│   ├── <driver>.c
│   └── ...
└── <chip>/              （芯片驱动一份）
    ├── <chip>.c
    ├── hpm_pin.c
    ├── startup.c
    └── <chip>.ld
```

编译时：
- `-I<instance>` 让驱动源码找到自己的 `hpm_dev.h`
- `-I<chip>` 让驱动源码找到芯片头
- 芯片驱动的 `build.flags` 和 `build.ldscript` 写入 Makefile

---

**文档版本**：v0.1
**对应 HPM 版本**：0.1.0
**反馈**：驱动包规范还在演进，欢迎提供反馈。