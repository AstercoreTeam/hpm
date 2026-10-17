// core/src/generator/generator.cpp
#include <hpm/generator/generator.h>

#include <hpm/util/fs_utils.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

namespace hpm {

namespace fs = std::filesystem;

// ─────────────────────────────────────────────
// 公共
// ─────────────────────────────────────────────

Result<GenerationPlan> Generator::plan() {
    GenerationPlan p;

    planDriverHeader(p);
    planDriverSource(p);

    for (const auto& inst : collectInstances()) {
        auto st = planInstance(p, inst.instanceName, inst.driver,
                               inst.version, inst.bindings);
        if (!st.ok()) return st.error();
    }

    auto st = planChipDriver(p);
    if (!st.ok()) return st.error();

    planMakefile(p);

    return p;
}

Status Generator::write(const GenerationPlan& plan) {
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
// 实例收集 / 宏值
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

std::string Generator::macroValue(const std::string& v) const {
    for (const auto& d : state_.userDevices) {
        if (d.name == v) return v;
    }
    for (const auto& d : state_.intermediateDevices) {
        if (d.name == v) return v;
    }
    return "hpm_pin_" + v;
}

// ─────────────────────────────────────────────
// 查找芯片包目录
// ─────────────────────────────────────────────

fs::path Generator::findChipPackageDir() const {
    if (!fs::exists(packageCacheDir_)) return {};
    for (const auto& entry : fs::directory_iterator(packageCacheDir_)) {
        if (!entry.is_directory()) continue;
        auto name = entry.path().filename().string();
        if (name.rfind(chip_.name + "-", 0) != 0) continue;
        if (fs::exists(entry.path() / "chip.json")) return entry.path();
    }
    return {};
}

// ─────────────────────────────────────────────
// driver.h
// ─────────────────────────────────────────────

void Generator::planDriverHeader(GenerationPlan& plan) {
    std::ostringstream out;
    out << "// 由 HPM 自动生成，请勿手动修改\n";
    out << "#pragma once\n\n";
    out << "#include <stdint.h>\n";
    out << "#include <stddef.h>\n\n";
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";

    // include 每个用户设备驱动的头
    for (const auto& dev : state_.userDevices) {
        out << "#include \"" << dev.name << "/" << dev.driver << ".h\"\n";
    }
    out << "\n";

    // 声明每个用户设备的 API
    for (const auto& dev : state_.userDevices) {
        out << "// ---------- " << dev.name << " (" << dev.driver << ") ----------\n";
        const DriverManifest* dm = lib_.find(dev.driver);
        if (!dm) continue;

        bool hasInit = false;
        for (const auto& m : dm->provides) {
            if (m.name == "init") hasInit = true;
            out << m.declaration(dev.name + "_") << "\n";
        }
        if (!hasInit) {
            out << "int " << dev.name << "_init(void);\n";
        }
        out << "\n";
    }

    out << "#ifdef __cplusplus\n}\n#endif\n";

    plan.addFile("driver.h", out.str());
}

// ─────────────────────────────────────────────
// driver.c
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
    fs::path instDir = instanceName;

    // hpm_dev.h
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

    // 拷贝驱动源码
    auto st = copyPackageSources(plan, driverName, version, instDir);
    if (!st.ok()) return st.error();

    return {};
}

// ─────────────────────────────────────────────
// 拷贝包源码（src + include + ld）
// ─────────────────────────────────────────────

Status Generator::copyPackageSources(GenerationPlan& plan,
                                     const std::string& driver,
                                     const Version& version,
                                     const std::filesystem::path& targetDir) {
    auto pkgDir = packageCacheDir_ / (driver + "-" + version.toString());
    if (!fs::exists(pkgDir)) {
        return Error(ErrorCode::NotFound,
                     "驱动包未找到: " + pkgDir.string());
    }

    for (const char* sub : {"src", "include", "ld"}) {
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
    auto chipPkgDir = findChipPackageDir();
    if (chipPkgDir.empty()) {
        return Error(ErrorCode::NotFound,
                     "未找到芯片驱动包: " + chip_.name +
                         "（在 " + packageCacheDir_.string() + " 下）");
    }

    fs::path chipTargetDir = chip_.name;

    for (const char* sub : {"src", "include", "ld"}) {
        auto dir = chipPkgDir / sub;
        if (!fs::exists(dir)) continue;

        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;

            auto rel = fs::relative(entry.path(), dir);
            auto target = chipTargetDir / rel;

            auto content = util::readFile(entry.path());
            if (!content) return content.error();

            plan.addFile(target, *content);
        }
    }

    return {};
}

// ─────────────────────────────────────────────
// Makefile
// ─────────────────────────────────────────────

void Generator::planMakefile(GenerationPlan& plan) {
    std::ostringstream out;

    out << "# 由 HPM 自动生成\n\n";
    out << "CC := " << chip_.toolchain.resolveCc() << "\n";
    out << "OBJCOPY := " << chip_.toolchain.resolveObjcopy() << "\n";
    out << "SIZE := " << chip_.toolchain.resolveSize() << "\n\n";

    // ── SRCS ──
    // 1. driver.c（胶水）
    // 2. 每个实例目录下的所有 .c（从包扫描）
    // 3. 芯片驱动包 src/ 下的所有 .c
    // 4. 用户 main.c
    std::vector<std::string> srcs;
    srcs.push_back("driver.c");

    // 实例源文件：按包里的 src/ 内容决定，但写到实例目录下
    for (const auto& inst : collectInstances()) {
        auto pkgSrcDir = packageCacheDir_ /
                         (inst.driver + "-" + inst.version.toString()) /
                         "src";
        if (!fs::exists(pkgSrcDir)) continue;

        std::set<std::string> seen;
        for (const auto& entry : fs::recursive_directory_iterator(pkgSrcDir)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".c") continue;

            auto rel = fs::relative(entry.path(), pkgSrcDir);
            auto target = inst.instanceName + "/" + rel.string();
            if (seen.count(target)) continue;
            seen.insert(target);
            srcs.push_back(target);
        }
    }

    // 芯片驱动源文件
    {
        auto chipPkgDir = findChipPackageDir();
        if (!chipPkgDir.empty()) {
            auto srcDir = chipPkgDir / "src";
            if (fs::exists(srcDir)) {
                for (const auto& entry : fs::recursive_directory_iterator(srcDir)) {
                    if (!entry.is_regular_file()) continue;
                    if (entry.path().extension() != ".c") continue;

                    auto rel = fs::relative(entry.path(), srcDir);
                    srcs.push_back(chip_.name + "/" + rel.string());
                }
            }
        }
    }

    srcs.push_back("../main.c");

    out << "SRCS := \\\n";
    for (size_t i = 0; i < srcs.size(); ++i) {
        out << "    " << srcs[i];
        if (i + 1 < srcs.size()) out << " \\";
        out << "\n";
    }
    out << "\n";

    // ── INCLUDES ──
    std::vector<std::string> includes;
    includes.push_back(".");
    includes.push_back(chip_.name);
    for (const auto& inst : collectInstances()) {
        includes.push_back(inst.instanceName);
    }

    out << "INCLUDES := \\\n";
    for (size_t i = 0; i < includes.size(); ++i) {
        out << "    -I" << includes[i];
        if (i + 1 < includes.size()) out << " \\";
        out << "\n";
    }
    out << "\n";

    // ── CFLAGS ──
    out << "CFLAGS := -std=gnu11 -Wall -Wextra -Wno-implicit-function-declaration -Os";
    for (const auto& f : chip_.build.flags) {
        out << " " << f;
    }
    out << " $(INCLUDES)\n";

    // ── LDFLAGS ──
    out << "LDFLAGS :=";
    if (!chip_.build.ldscript.empty()) {
        auto s = chip_.build.ldscript;
        auto slash = s.find_last_of('/');
        std::string fname = (slash == std::string::npos) ? s : s.substr(slash + 1);
        out << " -T " << chip_.name << "/" << fname;
    }
    out << " -Wl,--gc-sections\n\n";

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

    out << ".PHONY: clean size\n";
    out << "size: $(TARGET)\n";
    out << "\t$(SIZE) $<\n\n";
    out << "clean:\n";
    out << "\trm -rf $(BUILD_DIR)\n";

    plan.addFile("Makefile", out.str());
}

} // namespace hpm