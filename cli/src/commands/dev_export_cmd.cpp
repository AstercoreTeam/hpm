// cli/src/commands/dev_export_cmd.cpp
#include "dev_export_cmd.h"

#include "../cmd_context.h"
#include "../output.h"

#include <hpm/util/fs_utils.h>

#include <filesystem>
#include <sstream>

namespace hpm::cli {

void DevExportCmd::setup(CLI::App& app) {
    app.add_flag("--export", export_, "生成 hpm.h");
}

int DevExportCmd::run(CmdContext& ctx) {
    namespace fs = std::filesystem;

    if (!export_) {
        Output::error("请指定 --export");
        return 1;
    }

    auto driverDir = ctx.driverDir();
    if (!fs::exists(driverDir)) {
        Output::error("driver/ 不存在，先运行 hpm sync");
        return 1;
    }

    // 生成最小 hpm.h：暴露 hpm_init_all + 所有设备的 *_init
    std::ostringstream out;
    out << "// 由 HPM 自动生成（hpm dev --export）\n";
    out << "#pragma once\n\n";
    out << "#include <stdint.h>\n\n";
    out << "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n";
    out << "// 底层逃生舱：初始化所有设备\n";
    out << "void hpm_init_all(void);\n\n";
    out << "// 引脚操作（芯片驱动提供）\n";
    out << "void hpm_pin_write(int pin, int level);\n";
    out << "int  hpm_pin_read(int pin);\n\n";
    out << "#ifdef __cplusplus\n}\n#endif\n";

    auto path = driverDir / "hpm.h";
    auto st = util::writeFileAtomic(path, out.str());
    if (!st.ok()) {
        Output::error(st.error().message());
        return 1;
    }
    Output::success("已生成 " + path.string());
    return 0;
}

} // namespace hpm::cli