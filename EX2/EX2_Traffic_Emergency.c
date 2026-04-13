#include "SST89x5x4.h"

/*
 * P1.0  南北红灯
 * P1.1  南北黄灯
 * P1.2  南北绿灯
 * P1.3  东西红灯
 * P1.4  东西黄灯
 * P1.5  东西绿灯
 *
 * INT0  急救车中断请求，低电平触发后全红 10 秒，
 *       然后恢复到中断前状态。
 */

#define NS_RED   0x01
#define NS_YEL   0x02
#define NS_GRN   0x04
#define EW_RED   0x08
#define EW_YEL   0x10
#define EW_GRN   0x20

typedef enum
{
    NS_GREEN = 0,
    NS_YELLOW,
    EW_GREEN,
    EW_YELLOW
} traffic_state_t;

/*
 * 用 unsigned int 保存节拍计数，避免调试时把比较阈值改到 255 以上后
 * 由于 8 位溢出而永远达不到条件。
 */
volatile unsigned int tick_50ms = 0;
volatile unsigned char remain_seconds = 8;
volatile traffic_state_t state = NS_GREEN;

volatile bit second_flag = 0;
volatile bit emergency_req = 0;
volatile bit emergency_active = 0;

volatile unsigned char saved_remain = 0;
volatile traffic_state_t saved_state = NS_GREEN;

void show_state(traffic_state_t s)
{
    switch (s)
    {
    case NS_GREEN:
        P1 = NS_GRN | EW_RED;
        break;
    case NS_YELLOW:
        P1 = NS_YEL | EW_RED;
        break;
    case EW_GREEN:
        P1 = NS_RED | EW_GRN;
        break;
    case EW_YELLOW:
        P1 = NS_RED | EW_YEL;
        break;
    default:
        P1 = 0x00;
        break;
    }
}

void all_red(void)
{
    P1 = NS_RED | EW_RED;
}

void next_state(void)
{
    switch (state)
    {
    case NS_GREEN:
        state = NS_YELLOW;
        remain_seconds = 2;
        break;
    case NS_YELLOW:
        state = EW_GREEN;
        remain_seconds = 8;
        break;
    case EW_GREEN:
        state = EW_YELLOW;
        remain_seconds = 2;
        break;
    case EW_YELLOW:
        state = NS_GREEN;
        remain_seconds = 8;
        break;
    default:
        state = NS_GREEN;
        remain_seconds = 8;
        break;
    }
}

void timer0_isr(void) interrupt 1
{
    TH0 = 0x3C;
    TL0 = 0xB0;

    tick_50ms++;
    if (tick_50ms >= 20)
    {
        tick_50ms = 0;
        second_flag = 1;
    }
}

void int0_isr(void) interrupt 0
{
    emergency_req = 1;
}

void main(void)
{
    TMOD = 0x01;
    TH0 = 0x3C;
    TL0 = 0xB0;

    IT0 = 1;
    EX0 = 1;
    ET0 = 1;
    EA = 1;
    TR0 = 1;

    show_state(state);

    while (1)
    {
        if (emergency_req && !emergency_active)
        {
            emergency_req = 0;
            emergency_active = 1;
            saved_state = state;
            saved_remain = remain_seconds;
            /*
             * 触发急救车后把当前 50ms 节拍清零，
             * 这样“全红 10 秒”会从触发时刻开始完整计时，
             * 不会吃掉前一个状态剩余的零头时间。
             */
            tick_50ms = 0;
            second_flag = 0;
            remain_seconds = 10;
            all_red();
        }

        if (second_flag)
        {
            second_flag = 0;

            if (remain_seconds > 0)
            {
                remain_seconds--;
            }

            if (remain_seconds == 0)
            {
                if (emergency_active)
                {
                    emergency_active = 0;
                    state = saved_state;
                    remain_seconds = saved_remain;
                    show_state(state);
                }
                else
                {
                    next_state();
                    show_state(state);
                }
            }
        }
    }
}
