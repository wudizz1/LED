/**
 * STC89C51 LED 闪烁 + 按键控制程序
 * 功能：按键切换 LED 工作模式（闪烁 → 常亮 → 常灭 → 闪烁...）
 * 晶振：11.0592MHz（如需 12MHz，调整 DELAY_MS 宏注释）
 *
 * 硬件连接：
 *   LED：阳极接 VCC（5V），阴极通过 470Ω 限流电阻接 P1.0（低电平点亮）
 *   按键：一端接 P3.2，另一端接 GND（按下为低电平，需开启内部上拉或外接上拉）
 *         STC89C51 P3.2 默认有内部弱上拉，无需外接电阻
 */

#include <reg51.h>

// ==================== 引脚定义 ====================
sbit LED = P1 ^ 0;  // LED 连接至 P1.0
sbit KEY = P3 ^ 2;  // 按键连接至 P3.2（外部中断0引脚，此处用作普通IO）

// ==================== 晶振频率配置 ====================
#define FOSC 11059200UL // 晶振频率 11.0592MHz
// #define FOSC 12000000UL   // 如果使用 12MHz 晶振，取消此行注释

// ==================== LED 工作模式定义 ====================
#define MODE_BLINK   0  // 闪烁模式（500ms 间隔）
#define MODE_ON      1  // 常亮模式
#define MODE_OFF     2  // 常灭模式
#define MODE_MAX     3  // 模式总数

// ==================== 全局变量 ====================
unsigned char led_mode = MODE_BLINK;  // 当前 LED 工作模式

/**
 * @brief  毫秒级延时函数（软件延时，非精确）
 * @param  ms  延时的毫秒数
 * @note   适用于 11.0592MHz 晶振，12T 模式（默认）
 *         如果使用 12MHz 晶振，延时会略有偏差
 */
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
    {
        // 11.0592MHz / 12 = 921.6kHz，每机器周期 ≈ 1.085μs
        // 内层循环约 114 次 ≈ 1ms（粗略估算，非精确延时）
        for (j = 0; j < 114; j++)
        {
            // 空操作，消耗机器周期
        }
    }
}

/**
 * @brief  按键扫描函数（带消抖）
 * @retval 1  检测到按键按下（松开时返回）
 * @retval 0  无按键按下
 * @note   采用松开检测方式，消抖时间约 20ms
 */
unsigned char key_scan(void)
{
    if (KEY == 0)              // 检测到按键按下（低电平）
    {
        delay_ms(20);          // 消抖延时
        if (KEY == 0)          // 再次确认按下
        {
            while (KEY == 0);  // 等待按键松开
            return 1;          // 返回有效按键
        }
    }
    return 0;
}

/**
 * @brief  按键处理函数（切换 LED 模式）
 * @note   按下按键在三种模式间循环切换
 *         MODE_BLINK → MODE_ON → MODE_OFF → MODE_BLINK ...
 */
void key_handle(void)
{
    led_mode++;
    if (led_mode >= MODE_MAX)
    {
        led_mode = MODE_BLINK;  // 回到闪烁模式
    }
}

/**
 * @brief  定时器0初始化（精确延时方案，可选）
 * @note   使用定时器0工作模式1，50ms定时
 *         如果不需要精确延时，可以不调用此函数
 */
void timer0_init(void)
{
    TMOD &= 0xF0; // 清除定时器0的设置位
    TMOD |= 0x01; // 定时器0，工作模式1（16位定时器）
    // 50ms 定时初值（11.0592MHz，12T）
    // 65536 - 50000 * 11.0592 / 12 = 65536 - 46080 = 19456 = 0x4C00
    TH0 = 0x4C; // 高8位
    TL0 = 0x00; // 低8位
    ET0 = 1;    // 允许定时器0中断
    EA = 1;     // 开启总中断
    TR0 = 1;    // 启动定时器0
}

/**
 * @brief  主函数
 */
void main(void)
{
    LED = 1; // 初始化：熄灭 LED（高电平）

    // 如果使用定时器精确延时，取消下面这行注释
    // timer0_init();

    while (1)
    {
        // ---------- 按键扫描 ----------
        if (key_scan())      // 检测到有效按键
        {
            key_handle();    // 切换 LED 模式
        }

        // ---------- LED 控制（根据当前模式）----------
        switch (led_mode)
        {
        case MODE_BLINK:
            LED = 0;         // 点亮 LED
            delay_ms(500);   // 延时 500ms
            LED = 1;         // 熄灭 LED
            delay_ms(500);   // 延时 500ms
            break;

        case MODE_ON:
            LED = 0;         // 常亮
            break;

        case MODE_OFF:
            LED = 1;         // 常灭
            break;

        default:
            led_mode = MODE_BLINK;  // 非法模式，复位到闪烁
            break;
        }
    }
}

/**
 * @brief  定时器0中断服务函数（精确延时方案用）
 * @note   如果使用 timer0_init()，在此实现 LED 闪烁逻辑
 *         当前保留为占位函数
 */
// void timer0_isr(void) interrupt 1
// {
//     static unsigned int count = 0;
//     TH0 = 0x4C;  // 重装初值（50ms）
//     TL0 = 0x00;
//     count++;
//     if (count >= 10)  // 50ms * 10 = 500ms
//     {
//         count = 0;
//         LED = ~LED;  // 翻转 LED 状态
//     }
// }
