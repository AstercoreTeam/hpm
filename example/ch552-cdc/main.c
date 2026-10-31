#include "driver.h"
#include "hpm.h"

#define PASSWORD "hpm123"

// 简单延时（约 1ms × n，24MHz 下）
static void delay_ms(unsigned int ms) {
    while (ms--) {
        for (unsigned int i = 0; i < 200; i++) {}
    }
}

// 发一个字符串
static void print(const char* s) {
    while (*s) {
        serial_write(*s++);
    }
    CDC_flush();
}

// 读一行（以回车结束），最多 maxLen-1 个字符
static unsigned char read_line(char* buf, unsigned char maxLen) {
    unsigned char len = 0;
    while (1) {
        if (serial_available()) {
            char c = serial_read();
            if (c == '\r' || c == '\n') {
                serial_write('\r');
                serial_write('\n');
                CDC_flush();
                buf[len] = '\0';
                return len;
            }
            if (c == 0x7F || c == 0x08) {   // 退格
                if (len > 0) {
                    len--;
                    print("\b \b");
                }
                continue;
            }
            if (len < maxLen - 1) {
                buf[len++] = c;
                serial_write(c);
                CDC_flush();
            }
        }
    }
}

// 简单字符串比较
static unsigned char str_equal(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

void main(void) {
    char buf[32];

    // 用 hpm.h 的芯片 API 做初始化
    ch552g_init();
    serial_init();

    while (!USB_ENUM_OK) {}
    while (!CDC_getDTR()) {}

    // 用 hpm.h 的引脚 API 闪一下 P1.1
    hpm_pin_P1_1_init();
    hpm_pin_P1_1_write(1);
    delay_ms(200);
    hpm_pin_P1_1_write(0);

    while (1) {
        // 等待回车
        while (1) {
            if (serial_available()) {
                char c = serial_read();
                if (c == '\r' || c == '\n') break;
            }
        }

        // 欢迎信息
        print("\r\n");
        print("=========================\r\n");
        print("  Welcome to HPM Shell\r\n");
        print("=========================\r\n");

        // 提示符
        print("Password: ");

        // 读密码
        unsigned char len = read_line(buf, sizeof(buf));

        if (str_equal(buf, PASSWORD)) {
            print("ok\r\n");
        } else {
            delay_ms(3000);
            print("error\r\n");
        }
    }
}
