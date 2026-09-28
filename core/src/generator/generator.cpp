// core/src/generator/generator.cpp
#include <hpm/generator/generator.h>

#include <hpm/generator/template_engine.h>
#include <hpm/util/fs_utils.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

namespace hpm {

namespace {

// 收集所有用到的驱动（去重）
std::set<std::pair<std::string, std::string>>
collectDrivers(const DriverState& state) {
    std::set<std::pair<std::string, std::string>> out;
    for (const auto& d : state.userDevices) {
        out.insert({d.driver, d.version.toString()});
    }
    for (const auto& d : state.intermediateDevices) {
        out.insert({d.driver, d.version.toString()});
    }
    return out;
}

// 设备名首字母大写转驼峰，用于宏名
std::string toUpperSnake(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else {
            out += '_';
        }
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
    planDeviceMacros(p);

    auto st = planDriverSources(p);
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
// driver.h：用户 API
// ─────────────────────────────────────────────

void Generator::planDriverHeader(GenerationPlan& plan) {
    std::ostringstream out;
    out << "// 由 HPM 自动生成，请勿手动修改\n";
    out << "#pragma once\n\n";
    out << "#include <stdint.h>\n";
    out << "#include <stddef.h>\n\n";
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";

    // 每个用户设备：先 include 它的驱动头
    std::set<std::string> includedHeaders;
    for (const auto& dev : state_.userDevices) {
        if (includedHeaders.count(dev.driver)) continue;
        includedHeaders.insert(dev.driver);
        out << "#include \"" << dev.driver << "/" << dev.driver << ".h\"\n";
    }
    out << "\n";

    // 每个用户设备的宏 + API 声明
    for (const auto& dev : state_.userDevices) {
        out << "// ---------- 设备: " << dev.name << " (驱动 " << dev.driver << ") ----------\n";

        // 声明 init 和驱动提供的方法
        // 方法签名暂时简单处理：返回 int，无参数
        // 真实场景应从驱动清单的 provides 里读签名
        out << "int " << dev.name << "_init(void);\n";

        const DriverManifest* dm = lib_.find(dev.driver);
        if (dm) {
            for (const auto& m : dm->provides) {
                if (m.name == "init") continue;   // 已经有
                out << "int " << dev.name << "_" << m.name << "(void);\n";
            }
        }
        out << "\n";
    }

    out << "#ifdef __cplusplus\n}\n#endif\n";

    plan.addFile("driver.h", out.str());
}

// ─────────────────────────────────────────────
// driver.c：胶水
// ─────────────────────────────────────────────

void Generator::planDriverSource(GenerationPlan& plan) {
    std::ostringstream out;
    out << "// 由 HPM 自动生成，请勿手动修改\n";
    out << "#include \"driver.h\"\n\n";

    // 目前 driver.c 只做一件事：把各设备的初始化串起来
    // 具体调用的实现来自各驱动的源码，这里暂时留空
    // 真实场景会生成 「door_reader_init() → rc522_init_with_macros(...)」的胶水

    out << "// 初始化所有设备（可选）\n";
    out << "void hpm_init_all(void) {\n";
    for (const auto& dev : state_.userDevices) {
        out << "    " << dev.name << "_init();\n";
    }
    out << "}\n";

    plan.addFile("driver.c", out.str());
}

// ─────────────────────────────────────────────
// 每个设备的 *_macros.h
// ─────────────────────────────────────────────

void Generator::planDeviceMacros(GenerationPlan& plan) {
    // 用户设备：DEVICE_NAME + 直接引脚 + bus 引用
    for (const auto& dev : state_.userDevices) {
        std::ostringstream out;
        out << "// 由 HPM 自动生成，请勿手动修改\n";
        out << "#pragma once\n\n";
        out << "#define DEVICE_NAME  " << dev.name << "\n";
        out << "#define DEVICE_DRIVER " << dev.driver << "\n\n";

        for (const auto& [k, v] : dev.bindings) {
            out << "#define " << k << "  " << v << "\n";
        }

        std::string fname = dev.driver + "/" + dev.name + "_macros.h";
        plan.addFile(fname, out.str());
    }

    // 中间设备：同样生成一份，宏名统一为 DEVICE_NAME
    for (const auto& idev : state_.intermediateDevices) {
        std::ostringstream out;
        out << "// 由 HPM 自动生成，请勿手动修改\n";
        out << "#pragma once\n\n";
        out << "#define DEVICE_NAME  " << idev.name << "\n";
        out << "#define DEVICE_DRIVER " << idev.driver << "\n\n";

        for (const auto& [k, v] : idev.bindings) {
            out << "#define " << k << "  " << v << "\n";
        }

        std::string fname = idev.driver + "/" + idev.name + "_macros.h";
        plan.addFile(fname, out.str());
    }
}

// ─────────────────────────────────────────────
// 复制驱动源码
// ─────────────────────────────────────────────

Status Generator::planDriverSources(GenerationPlan& plan) {
    for (const auto& [driver, version] : collectDrivers(state_)) {
        auto st = copyDriverPackage(plan, driver, Version::parse(version).value());
        if (!st.ok()) return st.error();
    }
    return {};
}

Status Generator::copyDriverPackage(GenerationPlan& plan,
                                    const std::string& driver,
                                    const Version& version) {
    namespace fs = std::filesystem;

    auto pkgDir = packageCacheDir_ / (driver + "-" + version.toString());
    if (!fs::exists(pkgDir)) {
        return Error(ErrorCode::NotFound,
                     "驱动包未找到: " + pkgDir.string());
    }

    // 复制 src/ 和 include/ 下的所有文件
    for (const char* sub : {"src", "include"}) {
        auto dir = pkgDir / sub;
        if (!fs::exists(dir)) continue;

        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;

            auto rel = fs::relative(entry.path(), pkgDir);
            auto target = fs::path(driver) / rel;

            // 读文件内容
            std::ifstream f(entry.path(), std::ios::binary);
            if (!f) {
                return Error(ErrorCode::IoError,
                             "读取失败: " + entry.path().string());
            }
            std::ostringstream ss;
            ss << f.rdbuf();

            plan.addFile(target, ss.str());
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
    out << "CC ?= arm-none-eabi-gcc\n";
    out << "OBJCOPY ?= arm-none-eabi-objcopy\n\n";

    // 源文件：所有驱动目录下的 .c
    out << "SRCS := \\\n";
    out << "    driver.c \\\n";

    std::set<std::string> seenDrivers;
    for (const auto& [driver, version] : collectDrivers(state_)) {
        if (seenDrivers.count(driver)) continue;
        seenDrivers.insert(driver);
        out << "    " << driver << "/" << driver << ".c \\\n";
    }
    // 去掉最后一个续行
    std::string s = out.str();
    if (s.size() >= 2 && s.substr(s.size() - 2) == "\\\n") {
        s = s.substr(0, s.size() - 2) + "\n";
    }
    out.str(s);
    out.clear();

    out << "\n";
    out << "INCLUDES := -I. -I..\n";
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