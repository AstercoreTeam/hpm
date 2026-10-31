#include "driver.h"

/* 角度补偿算法 */
static unsigned int angle_to_permille(int deg) {
    if (deg > 120) deg = 120;
    if (deg < 0)   deg = 0;
    return 165 - ((deg * 115) / 120);
}

int main(void) {
    unsigned char i = 0;
    const int angles[5] = { 0, 30, 60, 90, 120 };

    hpm_init_all();
    servo_start();

    while (1) {
        servo_set_duty(angle_to_permille(angles[i]));

        /* 用独立的 Timer1 精确延时 1.5 秒 */
        app_delay_delay_ms(1500);

        i++;
        if (i >= 5) i = 0;
    }
    return 0;
}
