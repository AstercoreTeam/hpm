// Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// core/src/generator/generator.cpp
#include <hpm/generator/generator.h>

#include <hpm/util/fs_utils.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace hpm {

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
// 实例
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
// driver.h
// ─────────────────────────────────────────────

void Generator::planDriverHeader(GenerationPlan& plan) {
    std::ostringstream out;
    out << "// 由 HPM 自动生成，请勿手动修改\n";
    out << "#pragma once\n\n";
    out << "#include <stdint.h>\n";
    out << "#include <stddef.h>\n\n";
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";

    for (const auto& dev : state_.userDevices) {
        out << "#include \"" << dev.name << "/" << dev.driver << ".h\"\n";
    }
    out << "\n";

    for (const auto& dev : state_.userDevices) {
        out << "// ---------- " << dev.name << " (" << dev.driver << ") ----------\n";
        const DriverManifest* dm = lib_.find(dev.driver);
        if (!dm) continue;

        for (const auto& m : dm->provides) {
            out << "int " << dev.name << "_" << m.name << "(void);\n";
        }
        if (!dm->providesMethod("init")) {
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
    namespace fs = std::filesystem;

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
// 拷贝包源码
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
    namespace fs = std::filesystem;

    // 在 packageCacheDir_ 下找 <chipName>-* 目录
    if (!fs::exists(packageCacheDir_)) {
        return Error(ErrorCode::NotFound,
                     "包目录不存在: " + packageCacheDir_.string());
    }

    fs::path chipPkgDir;
    for (const auto& entry : fs::directory_iterator(packageCacheDir_)) {
        if (!entry.is_directory()) continue;
        auto name = entry.path().filename().string();
        if (name.rfind(chip_.name + "-", 0) != 0) continue;

        if (fs::exists(entry.path() / "chip.json")) {
            chipPkgDir = entry.path();
            break;
        }
    }

    if (chipPkgDir.empty()) {
        return Error(ErrorCode::NotFound,
                     "未找到芯片驱动包: " + chip_.name +
                         "（在 " + packageCacheDir_.string() + " 下）");
    }

    fs::path chipTargetDir = chip_.name;

    // 拷贝 src/ 和 include/
    for (const char* sub : {"src", "include","ld"}) {
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

    // 工具链前缀从芯片驱动推导
    std::string prefix;
    {
        const auto& tn = chip_.toolchain.name;   // 如 "arm-none-eabi-gcc"
        auto pos = tn.find("gcc");
        if (pos != std::string::npos && pos > 0) {
            prefix = tn.substr(0, pos);          // "arm-none-eabi-"
        }
    }

    out << "PREFIX ?= " << prefix << "\n";
    out << "CC := $(PREFIX)gcc\n";
    out << "OBJCOPY ?= $(PREFIX)objcopy\n\n";

    // ── SRCS ──
    std::vector<std::string> srcs;
    srcs.push_back("driver.c");
    for (const auto& inst : collectInstances()) {
        srcs.push_back(inst.instanceName + "/" + inst.driver + ".c");
    }
    // 芯片驱动源：<chip>/<chip>.c、<chip>/hpm_pin.c、startup
    srcs.push_back(chip_.name + "/" + chip_.name + ".c");
    srcs.push_back(chip_.name + "/hpm_pin.c");
    if (!chip_.build.startup.empty()) {
        // startup 的路径里可能带 src/ 前缀，取出文件名
        auto s = chip_.build.startup;
        auto slash = s.find_last_of('/');
        std::string fname = (slash == std::string::npos) ? s : s.substr(slash + 1);
        srcs.push_back(chip_.name + "/" + fname);
    }
    // 用户 main.c
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

    // ── CFLAGS：芯片 flags + 通用 ──
    out << "CFLAGS := -std=gnu11 -Wall -Wextra -Wno-implicit-function-declaration -Os";
    for (const auto& f : chip_.build.flags) {
        out << " " << f;
    }
    out << " $(INCLUDES)\n";

    // ── LDFLAGS：链接脚本 ──
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

    out << ".PHONY: clean\n";
    out << "clean:\n";
    out << "\trm -rf $(BUILD_DIR)\n";

    plan.addFile("Makefile", out.str());
}

} // namespace hpm