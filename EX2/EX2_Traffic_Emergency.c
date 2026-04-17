#include "SST89x5x4.h"

/*
 * 实验二提高题：带急救车优先控制的交通灯系统
 *
 * 这份程序的核心不是“让几盏灯亮起来”，而是把交通灯抽象成一个
 * 按时间推进的有限状态机：
 *
 *   1. 南北绿、东西红
 *   2. 南北黄、东西红
 *   3. 东西绿、南北红
 *   4. 东西黄、南北红
 *
 * 主循环负责：
 *   - 根据定时器产生的“软件秒”推进倒计时
 *   - 当当前状态时间耗尽后切换到下一个状态
 *   - 在急救车请求到来时保存现场并切换到全红保护状态
 *
 * 中断负责：
 *   - Timer0：提供稳定时间基准
 *   - INT0：只提出急救车请求，不在中断里直接做长时间处理
 *
 * 这样设计的原因是：
 *   - 中断服务程序应该尽量短，避免长时间占用 CPU
 *   - 复杂逻辑放在主循环里更容易维护，也更方便恢复原状态
 */

/*
 * 端口分配：
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

/*
 * 交通灯状态枚举。
 * 这里只描述“系统当前处于哪个阶段”，具体输出交给 show_state() 统一转换。
 * 这样状态逻辑和端口输出逻辑分开，代码更清晰。
 */
typedef enum
{
    NS_GREEN = 0,
    NS_YELLOW,
    EW_GREEN,
    EW_YELLOW
} traffic_state_t;

/*
 * tick_50ms：Timer0 每进一次中断就加 1，用来累计基础节拍。
 *
 * 这里用 unsigned int，而不是 unsigned char，是为了避免调试时把
 * 比较阈值改到 255 以上后发生 8 位溢出，导致条件永远达不到。
 */
volatile unsigned int tick_50ms = 0;

/* remain_seconds：当前状态还需要保持多少“软件秒” */
volatile unsigned char remain_seconds = 8;

/* state：当前交通灯所处状态，初始为南北绿 */
volatile traffic_state_t state = NS_GREEN;

/* second_flag：定时器累计满 1 秒后置位，通知主循环处理 */
volatile bit second_flag = 0;

/* emergency_req：INT0 触发后置 1，表示有急救车请求等待处理 */
volatile bit emergency_req = 0;

/* emergency_active：当前是否正在执行急救车优先的全红保护阶段 */
volatile bit emergency_active = 0;

/* 保存急救车打断前的状态和剩余时间，保护结束后用于恢复 */
volatile unsigned char saved_remain = 0;
volatile traffic_state_t saved_state = NS_GREEN;

/*
 * show_state()
 * 把抽象状态转换成实际的 P1 口输出。
 *
 * 如果后续改接线，只需要改这里，不用改主逻辑。
 */
void show_state(traffic_state_t s)
{
    switch (s)
    {
    case NS_GREEN:
        /* 南北通行，东西等待 */
        P1 = NS_GRN | EW_RED;
        break;
    case NS_YELLOW:
        /* 南北由通行切换到禁行前的过渡阶段 */
        P1 = NS_YEL | EW_RED;
        break;
    case EW_GREEN:
        /* 东西通行，南北等待 */
        P1 = NS_RED | EW_GRN;
        break;
    case EW_YELLOW:
        /* 东西由通行切换到禁行前的过渡阶段 */
        P1 = NS_RED | EW_YEL;
        break;
    default:
        P1 = 0x00;
        break;
    }
}

/* 急救车优先时，两个方向都禁止通行，因此统一全红 */
void all_red(void)
{
    P1 = NS_RED | EW_RED;
}

/*
 * next_state()
 * 按既定顺序推进交通灯状态机，并为新状态装入持续时间。
 *
 * 顺序为：
 *   南北绿(8s) -> 南北黄(2s) -> 东西绿(8s) -> 东西黄(2s) -> 南北绿...
 */
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

/*
 * Timer0 中断：
 * 1. 重装 TH0/TL0，保证下一次中断仍保持同样节拍
 * 2. 累加 tick_50ms
 * 3. 当累计到设定阈值时，近似认为过了 1 秒，通知主循环处理
 *
 * 这里不直接在中断中切换交通灯，只置位 second_flag。
 * 原因是中断服务程序越短越稳定，复杂状态切换交给主循环更安全。
 */
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

/*
 * INT0 外部中断：
 * 只做一件事——置位 emergency_req。
 *
 * 为什么不在这里直接执行“全红 10 秒”？
 * 因为那样会让中断服务程序很长，既不利于实时性，
 * 也会让后续恢复原状态变得混乱。
 */
void int0_isr(void) interrupt 0
{
    emergency_req = 1;
}

void main(void)
{
    /* Timer0 工作在方式 1（16 位定时器） */
    TMOD = 0x01;

    /* 装入定时器初值，形成稳定基础节拍 */
    TH0 = 0x3C;
    TL0 = 0xB0;

    /* INT0 配置为下降沿触发 */
    IT0 = 1;

    /* 打开外部中断 0、Timer0 中断和总中断 */
    EX0 = 1;
    ET0 = 1;
    EA = 1;

    /* 启动定时器 0 */
    TR0 = 1;

    /* 上电后先显示初始状态：南北绿、东西红 */
    show_state(state);

    while (1)
    {
        /*
         * 检测急救车请求。
         * 只有当前不在急救车保护阶段时，才允许进入全红状态。
         */
        if (emergency_req && !emergency_active)
        {
            /* 请求一旦被主循环接收，就立刻清零，避免重复处理 */
            emergency_req = 0;

            /* 标记当前正在执行急救车优先阶段 */
            emergency_active = 1;

            /* 保存中断前的状态和剩余时间，供保护结束后恢复 */
            saved_state = state;
            saved_remain = remain_seconds;

            /*
             * 触发急救车后把当前节拍清零，
             * 这样“全红 10 秒”会从触发时刻开始完整计时，
             * 不会吃掉前一个状态剩余的零头时间。
             */
            tick_50ms = 0;
            second_flag = 0;

            /* 急救车优先阶段规定保持全红 10 秒 */
            remain_seconds = 10;
            all_red();
        }

        /*
         * second_flag 由定时器中断置位，表示“软件上过去了 1 秒”。
         * 主循环每次只处理一次，然后立刻清零。
         */
        if (second_flag)
        {
            second_flag = 0;

            /* 剩余时间还没用完，就继续倒计时 */
            if (remain_seconds > 0)
            {
                remain_seconds--;
            }

            /*
             * 当前状态时间耗尽后，有两种情况：
             * 1. 如果正在执行急救车优先，就结束全红并恢复现场
             * 2. 否则按正常交通灯顺序切换到下一状态
             */
            if (remain_seconds == 0)
            {
                if (emergency_active)
                {
                    /* 全红保护结束，退出急救车模式 */
                    emergency_active = 0;

                    /* 恢复到中断前状态和剩余时间，而不是重新开始 */
                    state = saved_state;
                    remain_seconds = saved_remain;
                    show_state(state);
                }
                else
                {
                    /* 正常运行时，进入下一个交通灯状态 */
                    next_state();
                    show_state(state);
                }
            }
        }
    }
}
