// core/include/hpm/generator/generator.h
#pragma once

#include <hpm/capability/driver_library.h>
#include <hpm/generator/generation_plan.h>
#include <hpm/model/driver_state.h>
#include <hpm/model/project.h>
#include <hpm/result.h>

#include <filesystem>

namespace hpm {

// 代码生成器
// 输入：DriverState + DriverLibrary + 驱动包缓存目录
// 输出：GenerationPlan，可通过 writePlan 写到 driver/ 目录
class Generator {
public:
    Generator(const DriverState& state,
              const DriverLibrary& lib,
              std::filesystem::path packageCacheDir,
              std::filesystem::path driverDir)
        : state_(state),
          lib_(lib),
          packageCacheDir_(std::move(packageCacheDir)),
          driverDir_(std::move(driverDir)) {}

    // 构建生成计划
    Result<GenerationPlan> plan();

    // 执行：把计划写入 driverDir_
    Status write(const GenerationPlan& plan);

    // 便捷：plan + write
    Status generate();

private:
    // 生成 driver.h（用户 API 声明）
    void planDriverHeader(GenerationPlan& plan);

    // 生成 driver.c（胶水：驱动初始化）
    void planDriverSource(GenerationPlan& plan);

    // 生成每个设备的 *_macros.h
    void planDeviceMacros(GenerationPlan& plan);

    // 复制驱动源码到 driver/<driver>/
    Status planDriverSources(GenerationPlan& plan);

    // 生成 Makefile
    void planMakefile(GenerationPlan& plan);

    // 把驱动包目录里的 src/include 复制到计划里
    Status copyDriverPackage(GenerationPlan& plan,
                             const std::string& driver,
                             const Version& version);

    const DriverState&         state_;
    const DriverLibrary&       lib_;
    std::filesystem::path      packageCacheDir_;
    std::filesystem::path      driverDir_;
};

} // namespace hpm