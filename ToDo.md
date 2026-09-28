hpm/
│
├── CMakeLists.txt[OK]                          顶层构建：定义子目录、全局编译选项、C++20
├── README.md                               项目说明、快速开始、架构概览
├── LICENSE                                 开源协议
├── .gitignore[OK]                              忽略 build/、driver/、.hpm/ 等
├── .clang-format                           代码风格配置
├── .clang-tidy                             静态检查配置
│
├── cmake/                                  CMake 辅助模块
│   ├── FindLibCurl.cmake                   查找 libcurl
│   ├── FindQt6.cmake                       查找 Qt6（GUI 用）
│   └── HpmOptions.cmake[OK]                    编译选项、警告等级、sanitizer 开关
│
├── third_party/                            第三方依赖
│   ├── CMakeLists.txt                      用 FetchContent 或 add_subdirectory 引入
│   ├── json/                               nlohmann/json（单头文件）
│   │   └── json.hpp
│   ├── fmt/                                fmt 格式化库
│   ├── cli11/                              CLI11 命令行解析
│   └── catch2/                             Catch2 测试框架
│
├── core/                                   共享核心库 hpm_core
│   ├── CMakeLists.txt[OK]                      构建 hpm_core 静态库
│   │
│   ├── include/hpm/[OK]                        对外公共头文件（安装用）
│   │   ├── hpm.h                           总入口，include 所有公共 API
│   │   ├── types.h                         基础类型别名：PinName、DeviceName、DriverName
│   │   ├── result.h                        Result<T> / Error，统一错误处理
│   │   └── version.h                       HPM 自身版本号
│   │
│   └── src/
│       │
│       ├── json/                           JSON 读写封装
│       │   ├── json_value.h/.cpp           JsonValue 类型，包装 nlohmann
│       │   ├── json_parser.h/.cpp          从字符串/文件解析
│       │   ├── json_writer.h/.cpp          序列化为字符串/文件（带缩进）
│       │   └── json_path.h/.cpp            路径查询，如 "devices.rc522.cs"
│       │
│       ├── model/                          数据模型（内存对象）
│       │   ├── version.h/.cpp              semver：解析、比较、约束匹配（^、~、*）
│       │   ├── pin.h/.cpp                  引脚：名字、能力列表、shareable
│       │   ├── pin_capability.h/.cpp       引脚能力：类型 + 实例（spi_sck/SPI1）
│       │   ├── pin_assignment.h/.cpp       一次引脚分配：引脚 → 资源 key
│       │   ├── device.h/.cpp               设备实例：设备名、驱动名、引脚分配、版本
│       │   ├── driver_manifest.h/.cpp      驱动清单：requires、provides、元数据
│       │   ├── requirement.h/.cpp          一条需求：资源类型 + 需要的方法 + 可选性
│       │   ├── provided_method.h/.cpp      提供的方法：名字、签名、返回类型
│       │   ├── resource.h/.cpp             资源抽象：引脚资源、总线资源、方法资源
│       │   ├── project.h/.cpp              项目模型：mcu + 设备列表
│       │   ├── driver_state.h/.cpp         driver.json 的内存表示
│       │   ├── intermediate_device.h/.cpp  中间虚拟设备（__spi_a3f2）
│       │   ├── chip.h/.cpp                 芯片模型：型号、引脚表、工具链、烧录配置
│       │   └── dependency.h/.cpp           依赖边：谁 requires 谁的什么
│       │
│       ├── manifest/                       清单解析与校验
│       │   ├── manifest_loader.h/.cpp      从文件加载各类清单为 model 对象
│       │   ├── manifest_validator.h/.cpp   字段校验：必填、类型、枚举、引用
│       │   ├── manifest_schema.h/.cpp      各清单的 schema 定义（字段表）
│       │   ├── project_loader.h/.cpp       加载 project.json
│       │   ├── driver_loader.h/.cpp        加载驱动清单
│       │   ├── state_loader.h/.cpp         加载 driver.json
│       │   └── test_config_loader.h/.cpp   加载 test.json
│       │
│       ├── capability/                     能力检查（核心价值）
│       │   ├── capability_checker.h/.cpp   总调度：检查整个项目
│       │   ├── pin_allocator.h/.cpp        引脚分配表：谁占了哪个引脚
│       │   ├── pin_matcher.h/.cpp          引脚能力匹配：要 spi_sck，引脚支不支持
│       │   ├── conflict_detector.h/.cpp    冲突检测：引脚冲突、名字冲突
│       │   ├── name_registry.h/.cpp        名字注册表：设备名全局唯一
│       │   ├── method_resolver.h/.cpp      方法能力匹配：要 transfer，有没有
│       │   ├── resource_graph.h/.cpp       资源依赖图：逐层向上申请
│       │   └── check_report.h/.cpp         检查报告：错误、警告、位置信息
│       │
│       ├── resolver/                       依赖解析
│       │   ├── dependency_graph.h/.cpp     从 project.json 构建依赖图
│       │   ├── resolver.h/.cpp             拓扑排序、逐层解析
│       │   ├── version_solver.h/.cpp       版本约束求解
│       │   ├── intermediate_creator.h/.cpp 自动创建中间虚拟设备
│       │   ├── id_generator.h/.cpp         随机 ID 生成 + 从 driver.json 复用
│       │   ├── lock_file.h/.cpp            driver.json 的读、写、更新
│       │   └── resolve_result.h/.cpp       解析结果：完整 DriverState
│       │
│       ├── generator/                      代码生成
│       │   ├── generator.h/.cpp            总调度：生成整个 driver/ 目录
│       │   ├── template_engine.h/.cpp      简单模板：{{var}} 替换
│       │   ├── macro_generator.h/.cpp      生成每个驱动的 *_macros.h
│       │   ├── driver_h_generator.h/.cpp   生成 driver.h（用户 API）
│       │   ├── driver_c_generator.h/.cpp   生成 driver.c（胶水代码）
│       │   ├── hpm_h_generator.h/.cpp      生成 hpm.h（底层逃生舱）
│       │   ├── makefile_generator.h/.cpp   生成 Makefile
│       │   ├── source_copier.h/.cpp        把驱动源码复制到 driver/ 下
│       │   ├── file_writer.h/.cpp          写文件：目录创建、幂等、原子写
│       │   └── generation_plan.h/.cpp      生成计划：要写哪些文件、内容是什么
│       │
│       ├── toolchain/                      工具链管理
│       │   ├── toolchain_spec.h/.cpp       工具链描述：名字、版本、来源 URL
│       │   ├── toolchain_manager.h/.cpp    确保工具链可用（查缓存或下载）
│       │   ├── toolchain_cache.h/.cpp      全局缓存 ~/.hpm/toolchains/
│       │   ├── toolchain_installer.h/.cpp  下载、解压、校验工具链
│       │   └── toolchain_runner.h/.cpp     调用 gcc/make，捕获输出
│       │
│       ├── registry/                       仓库客户端
│       │   ├── registry_client.h/.cpp      抽象接口
│       │   ├── registry_backend.h/.cpp     后端基类
│       │   ├── github_backend.h/.cpp       GitHub 后端（拉 release、raw 文件）
│       │   ├── http_backend.h/.cpp         通用 HTTP 后端（自建镜像）
│       │   ├── index.h/.cpp                索引：包列表、版本、元数据
│       │   ├── index_fetcher.h/.cpp        拉取 + 解析索引
│       │   ├── package_downloader.h/.cpp   下载驱动包 tarball
│       │   ├── package_extractor.h/.cpp    解压 tarball 到缓存
│       │   ├── package_verifier.h/.cpp     校验 hash、可选签名
│       │   └── mirror_config.h/.cpp        镜像配置：项目/用户/全局优先级
│       │
│       ├── cache/                          本地缓存
│       │   ├── cache_manager.h/.cpp        缓存根目录管理
│       │   ├── package_cache.h/.cpp        驱动包缓存：查、存、清理
│       │   ├── index_cache.h/.cpp          索引缓存 + TTL
│       │   ├── hash.h/.cpp                 内容哈希（SHA256）
│       │   └── cache_key.h/.cpp            缓存 key：包名 + 版本
│       │
│       ├── build/                          构建系统
│       │   ├── build_driver.h/.cpp         构建总调度：make 或直接调 gcc
│       │   ├── makefile_runner.h/.cpp      调 make，传参数，捕获输出
│       │   ├── build_config.h/.cpp         构建配置：优化等级、宏定义、include 路径
│       │   └── build_artifact.h/.cpp       产物管理：.elf/.hex/.bin/.map 路径
│       │
│       ├── flash/                          烧录
│       │   ├── flasher.h/.cpp              抽象接口
│       │   ├── flasher_factory.h/.cpp      根据芯片驱动选 flasher
│       │   ├── stlink_flasher.h/.cpp       st-flash 实现
│       │   ├── openocd_flasher.h/.cpp      openocd 实现
│       │   ├── esptool_flasher.h/.cpp      esptool 实现
│       │   └── flash_config.h/.cpp         烧录配置：工具、参数、地址
│       │
│       ├── device/                         设备枚举（真机探测，规划）
│       │   ├── device_scanner.h/.cpp       扫描连接的调试器/串口
│       │   └── serial_port.h/.cpp          串口列表、波特率配置
│       │
│       └── util/                           工具函数
│           ├── fs_utils.h/.cpp             路径、目录、临时文件、原子写
│           ├── string_utils.h/.cpp         分割、trim、大小写、格式化
│           ├── process.h/.cpp              子进程调用，捕获 stdout/stderr
│           ├── log.h/.cpp                  日志：等级、颜色、文件输出
│           ├── error.h/.cpp                错误码、错误消息、错误上下文
│           ├── env.h/.cpp                  环境变量、HOME、XDG 路径
│           ├── random.h/.cpp               随机 ID 生成
│           └── timer.h/.cpp                计时、进度条
│
├── cli/                                    hpm 命令行工具
│   ├── CMakeLists.txt                      构建 hpm_cli 可执行
│   └── src/
│       ├── main.cpp                        入口：解析 argv，分发到命令
│       ├── cmd_context.h/.cpp              命令上下文：项目路径、配置、日志器
│       ├── command.h                       命令基类：name()、run()、help()
│       ├── command_registry.h/.cpp         注册所有命令
│       ├── help.cpp                        帮助文本生成
│       ├── output.h/.cpp                   用户输出：进度、成功、错误（带颜色）
│       └── commands/
│           ├── init_cmd.h/.cpp             hpm init：生成 project.json 模板
│           ├── sync_cmd.h/.cpp             hpm sync：解析+拉取+检查+生成
│           ├── build_cmd.h/.cpp            hpm build：生成 Makefile+调 make
│           ├── flash_cmd.h/.cpp            hpm flash：读配置+调烧录器
│           ├── clean_cmd.h/.cpp            hpm clean：删 build/
│           └── dev_export_cmd.h/.cpp       hpm dev --export：生成 hpm.h
│
├── dev/                                    hpm-dev 命令行工具
│   ├── CMakeLists.txt                      构建 hpm_dev 可执行
│   └── src/
│       ├── main.cpp                        入口
│       ├── dev_context.h/.cpp              上下文：驱动包路径、配置
│       └── commands/
│           ├── init_cmd.h/.cpp             hpm-dev init：建驱动包骨架
│           ├── test_cmd.h/.cpp             hpm-dev test：多芯片测试
│           ├── validate_cmd.h/.cpp         hpm-dev validate：校验清单和接口
│           ├── publish_cmd.h/.cpp          hpm-dev publish：打包+推送
│           └── import_cmd.h/.cpp           hpm-dev import：AI 导入（规划）
│       ├── test_runner/
│       │   ├── test_discovery.h/.cpp       发现 test/*.c 用例
│       │   ├── test_project_gen.h/.cpp     实时生成测试项目
│       │   ├── test_builder.h/.cpp         编译测试固件
│       │   ├── qemu_runner.h/.cpp          在 QEMU 里执行
│       │   ├── real_runner.h/.cpp          真机烧录+执行
│       │   ├── result_parser.h/.cpp        解析串口/退出码
│       │   ├── test_reporter.h/.cpp        汇总报告
│       │   └── test_config.h/.cpp          test.json 的模型
│       └── publisher/
│           ├── packager.h/.cpp             打包成 tarball
│           ├── manifest_writer.h/.cpp      生成发布用清单
│           └── publish_backend.h/.cpp      推送到 GitHub / 镜像
│
├── gui/                                    hpm-gui（Qt6）
│   ├── CMakeLists.txt                      构建 hpm_gui 可执行
│   └── src/
│       ├── main.cpp                        入口，启动 QApplication
│       ├── main_window.h/.cpp              主窗口，菜单、工具栏、布局
│       ├── app_state.h/.cpp                全局状态：当前项目、选中的芯片
│       ├── chip_selector.h/.cpp            芯片选择器（列表 + 搜索）
│       ├── pin_view.h/.cpp                 引脚图：按封装绘制，带状态着色
│       ├── pin_item.h/.cpp                 单个引脚图元（QGraphicsItem）
│       ├── device_panel.h/.cpp             已添加设备列表
│       ├── device_item.h/.cpp              单个设备条目
│       ├── driver_picker.h/.cpp            驱动选择器弹窗
│       ├── pin_assigner.h/.cpp             引脚分配向导，逐步引导
│       ├── conflict_highlighter.h/.cpp     冲突可视化：红/黄/灰
│       ├── project_model.h/.cpp            GUI 内部项目模型
│       ├── project_writer.h/.cpp           输出 project.json
│       ├── registry_client.h/.cpp          拉取芯片/驱动列表（复用 core）
│       ├── sync_runner.h/.cpp              子进程调 hpm sync
│       ├── settings_dialog.h/.cpp          镜像、缓存路径等设置
│       └── kicad_import/
│           ├── kicad_parser.h/.cpp         解析 .kicad_sch
│           ├── symbol_mapper.h/.cpp        符号 → 驱动映射
│           ├── net_mapper.h/.cpp           网络 → 引脚分配
│           └── import_dialog.h/.cpp        导入向导
│
├── tests/                                  测试
│   ├── CMakeLists.txt
│   ├── unit/
│   │   ├── test_version.cpp                semver 解析、比较、约束
│   │   ├── test_json.cpp                   JSON 读写
│   │   ├── test_manifest.cpp               清单加载、校验
│   │   ├── test_capability.cpp             能力检查、引脚冲突
│   │   ├── test_resolver.cpp               依赖解析、中间设备创建
│   │   ├── test_generator.cpp              模板替换、宏生成
│   │   ├── test_toolchain.cpp              工具链查找
│   │   └── test_registry.cpp               索引解析、缓存
│   ├── integration/
│   │   ├── test_sync_flow.cpp              完整 sync 流程
│   │   ├── test_build_flow.cpp             sync + build
│   │   └── test_multi_device.cpp           多设备、多总线场景
│   └── fixtures/
│       ├── projects/                       测试用 project.json
│       ├── drivers/                        测试用驱动包
│       └── chips/                          测试用芯片能力数据
│
├── docs/                                   文档
│   ├── architecture.md                     架构说明
│   ├── project_json.md                     project.json 格式规范
│   ├── driver_json.md                      driver.json 格式规范
│   ├── driver_manifest.md                  驱动清单规范
│   ├── test_json.md                        test.json 格式规范
│   ├── chip_driver.md                      如何写芯片驱动
│   ├── device_driver.md                    如何写设备驱动
│   └── cli_reference.md                    命令参考
│
└── examples/                               示例项目
    ├── blinky_stm32/                       最小示例：点灯
    │   ├── project.json
    │   └── main.c
    ├── rc522_stm32/                        RC522 读卡
    │   ├── project.json
    │   └── main.c
    └── multi_device/                       多设备组合
        ├── project.json
        └── main.c