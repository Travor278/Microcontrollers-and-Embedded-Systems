#include "SST89x5x4.H"

/* =====================================================
   直流电机 PWM 调速提高题

   功能：
   1. 按键控制启动、停止、加速、减速
   2. 速度档位 0~4
   3. 0 档也有速度，不是停止
   4. 4 档不是满速，避免电机过快
   5. 数码管 COM5 显示当前速度档位
   6. 越界操作报警：蜂鸣器 + 报警灯 + 串口发送 error
   7. 上位机串口可以实时查看和控制状态

   串口：
   波特率 9600
   数据位 8
   停止位 1
   无校验
   ===================================================== */


/* ================= 电机 PWM 输出 ================= */
sbit MOTOR_PWM = P1^7;


/* ================= 键盘 X 线：P1 口 ================= */
sbit X1 = P1^0;
sbit X2 = P1^1;
sbit X3 = P1^2;
sbit X4 = P1^3;


/* ================= 键盘 Y 线：P2 口 ================= */
sbit Y1 = P2^0;
sbit Y2 = P2^1;
sbit Y3 = P2^2;
sbit Y4 = P2^3;


/* ================= 数码管 COM5 ================= */
sbit DIG5 = P2^4;


/* ================= 报警输出 ================= */
sbit ALARM_LED = P3^4;
sbit BEEP      = P3^5;


/* =====================================================
   有效电平设置

   如果你的实验箱显示或报警逻辑反了，
   优先修改这里。
   ===================================================== */

/* 数码管 COM5：默认低电平选通 */
#define DIGIT_ON_LEVEL     0
#define DIGIT_OFF_LEVEL    1

/* 蜂鸣器：无源蜂鸣器，由报警函数输出方波 */
#define BEEP_ON_LEVEL      1
#define BEEP_OFF_LEVEL     0

/* 报警灯：默认高电平亮 */
#define LED_ON_LEVEL       1
#define LED_OFF_LEVEL      0


/* =====================================================
   数码管段码

   当前默认：共阳极数码管，低电平点亮
   P0.0~P0.7 对应 a b c d e f g dp

   如果显示完全反了，把 DisplaySpeed() 里面：
   P0 = seg_code_ca[speed_level];
   改成：
   P0 = ~seg_code_ca[speed_level];
   ===================================================== */
unsigned char code seg_code_ca[10] =
{
    0xC0,   /* 0 */
    0xF9,   /* 1 */
    0xA4,   /* 2 */
    0xB0,   /* 3 */
    0x99,   /* 4 */
    0x92,   /* 5 */
    0x82,   /* 6 */
    0xF8,   /* 7 */
    0x80,   /* 8 */
    0x90    /* 9 */
};


/* =====================================================
   速度档位对应 PWM 占空比

   重点：
   speed_level = 0 时，电机也有速度
   speed_level = 4 时，不是 100% 满速

   0 -> 20%
   1 -> 35%
   2 -> 50%
   3 -> 65%
   4 -> 80%
   ===================================================== */
unsigned char code duty_table[5] =
{
    20,
    35,
    50,
    65,
    80
};


/* ================= 全局变量 ================= */
volatile unsigned char pwm_count = 0;
volatile unsigned int  send_count = 0;
volatile bit send_flag = 0;

bit motor_run = 0;              /* 0：停止，1：运行 */
unsigned char speed_level = 0;  /* 速度档位 0~4 */

bit key_lock = 0;


/* =====================================================
   简单延时
   ===================================================== */
void DelayMs(unsigned int ms)
{
    unsigned int i;
    unsigned char j;

    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 120; j++)
        {
            ;
        }
    }
}


/* =====================================================
   串口初始化
   11.0592MHz 晶振，9600bps
   ===================================================== */
void UART_Init(void)
{
    SCON = 0x50;       /* 串口方式1，允许接收 */

    TMOD &= 0x0F;      /* 清除 Timer1 设置 */
    TMOD |= 0x20;      /* Timer1 方式2，8位自动重装 */

    TH1 = 0xFD;        /* 9600bps @ 11.0592MHz */
    TL1 = 0xFD;

    TR1 = 1;

    TI = 0;
    RI = 0;
}


/* =====================================================
   串口发送一个字符
   ===================================================== */
void UART_SendChar(unsigned char dat)
{
    TI = 0;
    SBUF = dat;

    while(TI == 0)
    {
        ;
    }

    TI = 0;
}


/* =====================================================
   串口发送字符串
   ===================================================== */
void UART_SendString(unsigned char *s)
{
    while(*s != '\0')
    {
        UART_SendChar(*s);
        s++;
    }
}


/* =====================================================
   串口发送一位数字
   ===================================================== */
void UART_SendDigit(unsigned char num)
{
    UART_SendChar(num + '0');
}


/* =====================================================
   发送当前状态到上位机
   ===================================================== */
void SendStatus(void)
{
    UART_SendString("Motor:");

    if(motor_run)
    {
        UART_SendString("RUN");
    }
    else
    {
        UART_SendString("STOP");
    }

    UART_SendString("  Speed:");
    UART_SendDigit(speed_level);

    UART_SendString("  Duty:");
    UART_SendDigit(duty_table[speed_level] / 10);
    UART_SendDigit(duty_table[speed_level] % 10);
    UART_SendString("%\r\n");
}


/* =====================================================
   报警灯和蜂鸣器控制
   ===================================================== */
void Alarm_Output_On(void)
{
    ALARM_LED = LED_ON_LEVEL;
    BEEP = BEEP_ON_LEVEL;
}


void Alarm_Output_Off(void)
{
    ALARM_LED = LED_OFF_LEVEL;
    BEEP = BEEP_OFF_LEVEL;
}


/* =====================================================
   越界报警函数
   ===================================================== */
void Alarm_Error(void)
{
    unsigned int i;

    UART_SendString("error\r\n");

    ALARM_LED = LED_ON_LEVEL;

    for(i = 0; i < 400; i++)
    {
        BEEP = BEEP_ON_LEVEL;
        DelayMs(1);
        BEEP = BEEP_OFF_LEVEL;
        DelayMs(1);
    }

    Alarm_Output_Off();
}


/* =====================================================
   数码管显示当前速度档位
   只打开 COM5
   ===================================================== */
void DisplaySpeed(void)
{
    DIG5 = DIGIT_OFF_LEVEL;

    /*
       当前按共阳极段码处理。
       如果你的实验箱显示反了，改成：
       P0 = ~seg_code_ca[speed_level];
    */
    P0 = ~seg_code_ca[speed_level];

    DIG5 = DIGIT_ON_LEVEL;
}


/* =====================================================
   Timer0 初始化
   用于产生 PWM 和定时发送状态

   约 100us 中断一次
   PWM周期 = 100 * 100us = 10ms
   PWM频率约 100Hz
   ===================================================== */
void Timer0_Init(void)
{
    TMOD &= 0xF0;      /* 清除 Timer0 设置 */
    TMOD |= 0x01;      /* Timer0 方式1 */

    /*
       11.0592MHz 下，100us 约 92 个机器周期
       65536 - 92 = 65444 = 0xFFA4
    */
    TH0 = 0xFF;
    TL0 = 0xA4;

    ET0 = 1;
    EA  = 1;
    TR0 = 1;
}


/* =====================================================
   Timer0 中断服务函数
   ===================================================== */
void Timer0_ISR(void) interrupt 1
{
    TH0 = 0xFF;
    TL0 = 0xA4;

    pwm_count++;

    if(pwm_count >= 100)
    {
        pwm_count = 0;
    }

    if(motor_run)
    {
        if(pwm_count < duty_table[speed_level])
        {
            MOTOR_PWM = 0;
        }
        else
        {
            MOTOR_PWM = 1;
        }
    }
    else
    {
        MOTOR_PWM = 1;
    }

    /*
       约 500ms 发送一次状态
       100us * 5000 = 500ms
    */
    send_count++;

    if(send_count >= 5000)
    {
        send_count = 0;
        send_flag = 1;
    }
}


/* =====================================================
   键盘扫描函数

   接线：
   X1 -> P1.0
   X2 -> P1.1
   X3 -> P1.2
   X4 -> P1.3

   Y1 -> P2.0
   Y2 -> P2.1
   Y3 -> P2.2
   Y4 -> P2.3

   返回值：
   0~15 对应 OM0~OMF
   0xFF 表示无按键

   按键编号：
          X1   X2   X3   X4
   Y1     OM0  OM1  OM2  OM3
   Y2     OM4  OM5  OM6  OM7
   Y3     OM8  OM9  OMA  OMB
   Y4     OMC  OMD  OME  OMF

   本程序使用：
   OM0：启动
   OM1：停止
   OM2：减速
   OM3：加速
   ===================================================== */
unsigned char KeyScan(void)
{
    unsigned char key;

    key = 0xFF;

    X1 = 1;
    X2 = 1;
    X3 = 1;
    X4 = 1;

    /* 扫描 X1 列 */
    X1 = 0;
    X2 = 1;
    X3 = 1;
    X4 = 1;
    DelayMs(1);

    if(Y1 == 0) key = 0;
    if(Y2 == 0) key = 4;
    if(Y3 == 0) key = 8;
    if(Y4 == 0) key = 12;

    /* 扫描 X2 列 */
    X1 = 1;
    X2 = 0;
    X3 = 1;
    X4 = 1;
    DelayMs(1);

    if(Y1 == 0) key = 1;
    if(Y2 == 0) key = 5;
    if(Y3 == 0) key = 9;
    if(Y4 == 0) key = 13;

    /* 扫描 X3 列 */
    X1 = 1;
    X2 = 1;
    X3 = 0;
    X4 = 1;
    DelayMs(1);

    if(Y1 == 0) key = 2;
    if(Y2 == 0) key = 6;
    if(Y3 == 0) key = 10;
    if(Y4 == 0) key = 14;

    /* 扫描 X4 列 */
    X1 = 1;
    X2 = 1;
    X3 = 1;
    X4 = 0;
    DelayMs(1);

    if(Y1 == 0) key = 3;
    if(Y2 == 0) key = 7;
    if(Y3 == 0) key = 11;
    if(Y4 == 0) key = 15;

    X1 = 1;
    X2 = 1;
    X3 = 1;
    X4 = 1;

    return key;
}


/* =====================================================
   执行按键功能
   ===================================================== */
void DoKey(unsigned char key)
{
    switch(key)
    {
        case 0:     /* OM0：启动 */
            motor_run = 1;
            UART_SendString("Start by key\r\n");
            SendStatus();
            break;

        case 1:     /* OM1：停止 */
            motor_run = 0;
            MOTOR_PWM = 0;
            UART_SendString("Stop by key\r\n");
            SendStatus();
            break;

        case 2:     /* OM2：减速 */
            if(speed_level == 0)
            {
                Alarm_Error();
            }
            else
            {
                speed_level--;
                UART_SendString("Speed down by key\r\n");
                SendStatus();
            }
            break;

        case 3:     /* OM3：加速 */
            if(speed_level == 4)
            {
                Alarm_Error();
            }
            else
            {
                speed_level++;
                UART_SendString("Speed up by key\r\n");
                SendStatus();
            }
            break;

        default:
            break;
    }
}


/* =====================================================
   按键处理，带消抖和松手检测
   ===================================================== */
void CheckKey(void)
{
    unsigned char key;

    key = KeyScan();

    if(key != 0xFF)
    {
        if(key_lock == 0)
        {
            DelayMs(20);

            key = KeyScan();

            if(key != 0xFF)
            {
                key_lock = 1;
                DoKey(key);
            }
        }
    }
    else
    {
        key_lock = 0;
    }
}


/* =====================================================
   串口命令处理

   上位机发送：
   r：启动
   t：停止
   +：加速
   -：减速
   0~4：直接设置速度档位
   s：立即发送一次状态
   ===================================================== */
void CheckUART(void)
{
    unsigned char ch;

    if(RI)
    {
        RI = 0;
        ch = SBUF;

        if(ch == 'r' || ch == 'R')
        {
            motor_run = 1;
            UART_SendString("Start by PC\r\n");
            SendStatus();
        }
        else if(ch == 't' || ch == 'T')
        {
            motor_run = 0;
            MOTOR_PWM = 0;
            UART_SendString("Stop by PC\r\n");
            SendStatus();
        }
        else if(ch == '+')
        {
            if(speed_level == 4)
            {
                Alarm_Error();
            }
            else
            {
                speed_level++;
                UART_SendString("Speed up by PC\r\n");
                SendStatus();
            }
        }
        else if(ch == '-')
        {
            if(speed_level == 0)
            {
                Alarm_Error();
            }
            else
            {
                speed_level--;
                UART_SendString("Speed down by PC\r\n");
                SendStatus();
            }
        }
        else if(ch >= '0' && ch <= '4')
        {
            speed_level = ch - '0';
            UART_SendString("Set speed by PC\r\n");
            SendStatus();
        }
        else if(ch == 's' || ch == 'S')
        {
            SendStatus();
        }
        else
        {
            UART_SendString("Unknown command\r\n");
        }
    }
}


/* =====================================================
   主函数
   ===================================================== */
void main(void)
{
    P0 = 0xFF;
    P1 = 0xFF;
    P2 = 0xFF;
    P3 = 0xFF;

    MOTOR_PWM = 1;
    DIG5 = DIGIT_OFF_LEVEL;

    Alarm_Output_Off();

    speed_level = 0;
    motor_run = 0;

    UART_Init();
    Timer0_Init();

    UART_SendString("\r\nDC Motor Control Experiment Start\r\n");
    UART_SendString("Key: OM0=start, OM1=stop, OM2=down, OM3=up\r\n");
    UART_SendString("PC : r=start, t=stop, +=up, -=down, 0~4=set speed, s=status\r\n");

    SendStatus();

    while(1)
    {
        DisplaySpeed();

        CheckKey();

        CheckUART();

        if(send_flag)
        {
            send_flag = 0;
            SendStatus();
        }
    }
}
