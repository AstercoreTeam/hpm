// core/include/hpm/generator/generator.h
#pragma once

#include <hpm/capability/driver_library.h>
#include <hpm/generator/generation_plan.h>
#include <hpm/model/driver_state.h>
#include <hpm/model/project.h>
#include <hpm/result.h>

#include <filesystem>
#include <string>

namespace hpm {

// 代码生成器
//
// 输出结构：
//   driver/
//   ├── driver.h              用户 API
//   ├── driver.c              胶水
//   ├── Makefile
//   ├── <instance>/           每个实例一个目录
//   │   ├── hpm_dev.h         实例宏
//   │   ├── <driver>.c        驱动源码拷贝
//   │   ├── <driver>.h
//   │   └── ...
//   └── <chip>/               芯片驱动（全局一份）
//       ├── <chip>.c
//       ├── hpm_pin_*.c
//       └── ...
class Generator {
public:
    Generator(const DriverState& state,
              const DriverLibrary& lib,
              const Project& project,
              std::filesystem::path packageCacheDir,
              std::filesystem::path driverDir,
              std::string chipDriverName)
        : state_(state),
          lib_(lib),
          project_(project),
          packageCacheDir_(std::move(packageCacheDir)),
          driverDir_(std::move(driverDir)),
          chipDriverName_(std::move(chipDriverName)) {}

    // 构建生成计划
    Result<GenerationPlan> plan();

    // 写计划到 driverDir_
    Status write(const GenerationPlan& plan);

    // plan + write
    Status generate();

private:
    // 生成用户 API 头
    void planDriverHeader(GenerationPlan& plan);

    // 生成胶水
    void planDriverSource(GenerationPlan& plan);

    // 生成 Makefile
    void planMakefile(GenerationPlan& plan);

    // 为每个实例生成目录
    Status planInstance(GenerationPlan& plan,
                        const std::string& instanceName,
                        const std::string& driverName,
                        const Version& version,
                        const std::map<ResourceKey, std::string>& bindings);

    // 拷贝芯片驱动
    Status planChipDriver(GenerationPlan& plan);

    // 拷贝驱动包里的源码到目标目录
    Status copyPackageSources(GenerationPlan& plan,
                              const std::string& driver,
                              const Version& version,
                              const std::filesystem::path& targetDir);

    // 计算宏值：绑定 key → 宏值
    //   - 如果值是一个设备名（在 state_ 里能找到）→ 直接用
    //   - 否则是引脚名 → "hpm_pin_" + 引脚名
    std::string macroValue(const std::string& bindingValue) const;

    // 收集所有实例（用户设备 + 中间设备）
    struct InstanceInfo {
        std::string instanceName;
        std::string driver;
        Version     version;
        std::map<ResourceKey, std::string> bindings;
    };
    std::vector<InstanceInfo> collectInstances() const;

    const DriverState&   state_;
    const DriverLibrary& lib_;
    const Project&       project_;
    std::filesystem::path packageCacheDir_;
    std::filesystem::path driverDir_;
    std::string           chipDriverName_;
};

} // namespace hpm