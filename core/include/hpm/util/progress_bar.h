/*
 * Copyright 2026 XueZicheng <xuezicheng842@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// core/include/hpm/util/progress_bar.h
#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include "hpm/config.h"

namespace hpm::util {

// 简单的终端进度条
// 用法：
//   ProgressBar bar("ch552g@1.0.0", 100);
//   bar.update(50);     // 50%
//   bar.done();
class ProgressBar {
public:
    ProgressBar(std::string label, uint64_t total)
        : label_(std::move(label)), total_(total) {}

    // 更新进度
    //   current: 已下载字节
    //   total:   总字节（0 表示未知）
    void update(uint64_t current, uint64_t total = 0) {
        if (total == 0) total = total_;
        if (total == 0) {
            // 未知总量，只显示已下载
            std::cout << "\r    " << label_
                      << "  " << formatSize(current) << "   " << std::flush;
            return;
        }

        int pct = static_cast<int>(current * 100 / total);
        int barWidth = HPM_PROGRESS_BAR_WIDTH;
        int filled = pct * barWidth / 100;

        std::cout << "\r    " << label_ << "  [";
        for (int i = 0; i < barWidth; ++i) {
            if (i < filled) std::cout << "█";
            else            std::cout << "░";
        }
        std::cout << "] " << pct << "%  "
                  << formatSize(current) << " / " << formatSize(total)
                  << "   " << std::flush;
    }

    // 完成
    void done() {
        std::cout << "\r    " << label_ << "  [";
        for (int i = 0; i < 30; ++i) std::cout << "█";
        std::cout << "] 100%  完成" << std::string(10, ' ') << "\n";
    }

    // 失败
    void fail() {
        std::cout << "\r    " << label_ << "  失败" << std::string(20, ' ') << "\n";
    }

private:
    static std::string formatSize(uint64_t bytes) {
        if (bytes < 1024) {
            return std::to_string(bytes) + " B";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024) + " KB";
        } else {
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        }
    }

    std::string label_;
    uint64_t    total_;
};

} // namespace hpm::util