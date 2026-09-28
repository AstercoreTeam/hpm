// core/src/generator/generator.cpp
#include <hpm/generator/generator.h>

#include <hpm/util/fs_utils.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace hpm {

namespace {

// 收集所有用到的驱动（driver + version），去重
std::set<std::pair<std::string, std::string>>
collectDriverKeys(const DriverState& state) {
    std::set<std::pair<std::string, std::string>> out;
    for (const auto& d : state.userDevices) {
        out.insert({d.driver, d.version.toString()});
    }
    for (const auto& d : state.intermediateDevices) {
        out.insert({d.driver, d.version.toString()});
    }
    return out;
}

} // namespace

// ─────────────────────────────────────────────
// 公共
// ─────────────────────────────────────────────

Result<GenerationPlan> Generator::plan() {
    GenerationPlan p;

    planDriverHeader(p);
    planDriverSource(p);

    // 为每个实例生成目录
    for (const auto& inst : collectInstances()) {
        auto st = planInstance(p, inst.instanceName, inst.driver,
                               inst.version, inst.bindings);
        if (!st.ok()) return st.error();
    }

    // 芯片驱动
    auto st = planChipDriver(p);
    if (!st.ok()) return st.error();

    planMakefile(p);

    return p;
}

Status Generator::write(const GenerationPlan& plan) {
    namespace fs = std::filesystem;

    std::error_code ec;
    fs::create_directories(driverDir_, ec);
    if (ec) {
        return Error(ErrorCode::IoError,
                     "创建目录失败: " + driverDir_.string() + " - " + ec.message());
    }

    for (const auto& f : plan.files) {
        auto path = driverDir_ / f.relativePath;
        auto st = util::writeFileAtomic(path, f.content);
        if (!st.ok()) return st.error();
    }
    return {};
}

Status Generator::generate() {
    auto p = plan();
    if (!p) return p.error();
    return write(*p);
}

// ─────────────────────────────────────────────
// 实例收集
// ─────────────────────────────────────────────

std::vector<Generator::InstanceInfo> Generator::collectInstances() const {
    std::vector<InstanceInfo> out;

    for (const auto& d : state_.userDevices) {
        out.push_back({d.name, d.driver, d.version, d.bindings});
    }
    for (const auto& d : state_.intermediateDevices) {
        out.push_back({d.name, d.driver, d.version, d.bindings});
    }
    return out;
}

// ─────────────────────────────────────────────
// 宏值计算
// ─────────────────────────────────────────────

std::string Generator::macroValue(const std::string& v) const {
    // 如果这个值是某个实例名（用户设备或中间设备），直接用
    for (const auto& d : state_.userDevices) {
        if (d.name == v) return v;
    }
    for (const auto& d : state_.intermediateDevices) {
        if (d.name == v) return v;
    }
    // 否则当作引脚名，加前缀
    return "hpm_pin_" + v;
}

// ─────────────────────────────────────────────
// 用户 API 头
// ─────────────────────────────────────────────

void Generator::planDriverHeader(GenerationPlan& plan) {
    std::ostringstream out;
    out << "// 由 HPM 自动生成，请勿手动修改\n";
    out << "#pragma once\n\n";
    out << "#include <stdint.h>\n";
    out << "#include <stddef.h>\n\n";
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";

    // include 每个驱动实例的头
    for (const auto& dev : state_.userDevices) {
        out << "#include \"" << dev.name << "/" << dev.driver << ".h\"\n";
    }
    out << "\n";

    // 声明每个用户设备的 API
    for (const auto& dev : state_.userDevices) {
        out << "// ---------- " << dev.name << " (" << dev.driver << ") ----------\n";

        const DriverManifest* dm = lib_.find(dev.driver);
        if (!dm) continue;

        // 从 provides 里生成声明
        for (const auto& m : dm->provides) {
            out << "int " << dev.name << "_" << m.name << "(void);\n";
        }
        // 如果没有 provides.init，补一个
        if (!dm->providesMethod("init")) {
            out << "int " << dev.name << "_init(void);\n";
        }
        out << "\n";
    }

    out << "#ifdef __cplusplus\n}\n#endif\n";

    plan.addFile("driver.h", out.str());
}

// ─────────────────────────────────────────────
// 胶水 driver.c
// ─────────────────────────────────────────────

void Generator::planDriverSource(GenerationPlan& plan) {
    std::ostringstream out;
    out << "// 由 HPM 自动生成，请勿手动修改\n";
    out << "#include \"driver.h\"\n\n";

    out << "void hpm_init_all(void) {\n";
    for (const auto& dev : state_.userDevices) {
        out << "    " << dev.name << "_init();\n";
    }
    out << "}\n";

    plan.addFile("driver.c", out.str());
}

// ─────────────────────────────────────────────
// 实例目录
// ─────────────────────────────────────────────

Status Generator::planInstance(GenerationPlan& plan,
                               const std::string& instanceName,
                               const std::string& driverName,
                               const Version& version,
                               const std::map<ResourceKey, std::string>& bindings) {
    namespace fs = std::filesystem;

    fs::path instDir = instanceName;

    // 1. 生成 hpm_dev.h
    {
        std::ostringstream out;
        out << "// 由 HPM 自动生成，请勿手动修改\n";
        out << "#pragma once\n\n";
        out << "#define DEVICE_NAME  " << instanceName << "\n\n";
        for (const auto& [k, v] : bindings) {
            out << "#define " << k << "  " << macroValue(v) << "\n";
        }
        plan.addFile(instDir / "hpm_dev.h", out.str());
    }

    // 2. 拷贝驱动源码到实例目录
    auto st = copyPackageSources(plan, driverName, version, instDir);
    if (!st.ok()) return st.error();

    return {};
}

// ─────────────────────────────────────────────
// 拷贝驱动包源码
// ─────────────────────────────────────────────

Status Generator::copyPackageSources(GenerationPlan& plan,
                                     const std::string& driver,
                                     const Version& version,
                                     const std::filesystem::path& targetDir) {
    namespace fs = std::filesystem;

    auto pkgDir = packageCacheDir_ / (driver + "-" + version.toString());
    if (!fs::exists(pkgDir)) {
        return Error(ErrorCode::NotFound,
                     "驱动包未找到: " + pkgDir.string());
    }

    // 拷贝 src/ 和 include/ 下的所有文件到 targetDir/
    for (const char* sub : {"src", "include"}) {
        auto dir = pkgDir / sub;
        if (!fs::exists(dir)) continue;

        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;

            auto rel = fs::relative(entry.path(), dir);
            auto target = targetDir / rel;

            auto content = util::readFile(entry.path());
            if (!content) return content.error();

            plan.addFile(target, *content);
        }
    }

    return {};
}

// ─────────────────────────────────────────────
// 芯片驱动
// ─────────────────────────────────────────────

Status Generator::planChipDriver(GenerationPlan& plan) {
    namespace fs = std::filesystem;

    // 芯片驱动名 = project_.mcu（约定）
    // 版本：从驱动库里找
    const DriverManifest* dm = lib_.find(chipDriverName_);
    if (!dm) {
        return Error(ErrorCode::NotFound,
                     "找不到芯片驱动: " + chipDriverName_);
    }

    fs::path chipDir = chipDriverName_;

    // 拷贝芯片驱动源码
    auto st = copyPackageSources(plan, chipDriverName_, dm->version, chipDir);
    if (!st.ok()) return st.error();

    return {};
}

// ─────────────────────────────────────────────
// Makefile
// ─────────────────────────────────────────────

void Generator::planMakefile(GenerationPlan& plan) {
    std::ostringstream out;

    out << "# 由 HPM 自动生成\n\n";
    out << "CC ?= arm-none-eabi-gcc\n";
    out << "OBJCOPY ?= arm-none-eabi-objcopy\n\n";

    // 源文件列表
    std::vector<std::string> srcs;
    srcs.push_back("driver.c");

    for (const auto& inst : collectInstances()) {
        // 实例目录里的 .c 文件
        // 约定：驱动包里 src/<driver>.c 被拷到 <instance>/<driver>.c
        srcs.push_back(inst.instanceName + "/" + inst.driver + ".c");
    }

    // 芯片驱动的 .c 文件（假设 <chip>/<chip>.c + <chip>/hpm_pin_*.c）
    srcs.push_back(chipDriverName_ + "/" + chipDriverName_ + ".c");

    out << "SRCS := \\\n";
    for (size_t i = 0; i < srcs.size(); ++i) {
        out << "    " << srcs[i];
        if (i + 1 < srcs.size()) out << " \\";
        out << "\n";
    }
    out << "\n";

    // include 路径
    out << "INCLUDES := -I. -I" << chipDriverName_ << " \\\n";
    for (const auto& inst : collectInstances()) {
        out << "    -I" << inst.instanceName << " \\\n";
    }
    // 去掉最后一个续行
    {
        std::string s = out.str();
        if (s.size() >= 3 && s.substr(s.size() - 3) == " \\\n") {
            s = s.substr(0, s.size() - 3) + "\n";
        }
        out.str(s);
        out.clear();
    }
    out << "\n";

    out << "CFLAGS := -std=c11 -Wall -Wextra -Os $(INCLUDES)\n";
    out << "LDFLAGS :=\n\n";

    out << "BUILD_DIR := ../build\n";
    out << "OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))\n\n";

    out << "TARGET := $(BUILD_DIR)/firmware.elf\n";
    out << "BIN    := $(BUILD_DIR)/firmware.bin\n\n";

    out << "all: $(TARGET) $(BIN)\n\n";

    out << "$(TARGET): $(OBJS)\n";
    out << "\t$(CC) $(OBJS) -o $@ $(LDFLAGS)\n\n";

    out << "$(BIN): $(TARGET)\n";
    out << "\t$(OBJCOPY) -O binary $< $@\n\n";

    out << "$(BUILD_DIR)/%.o: %.c\n";
    out << "\t@mkdir -p $(dir $@)\n";
    out << "\t$(CC) $(CFLAGS) -c $< -o $@\n\n";

    out << ".PHONY: clean\n";
    out << "clean:\n";
    out << "\trm -rf $(BUILD_DIR)\n";

    plan.addFile("Makefile", out.str());
}

} // namespace hpm