#include "stm32f10x.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include <stdio.h>

/* K1、K2按键引脚定义：野火指南者板上按键按下为高电平。 */
#define KEY1_GPIO_CLK          RCC_APB2Periph_GPIOA
#define KEY1_GPIO_PORT         GPIOA
#define KEY1_GPIO_PIN          GPIO_Pin_0

#define KEY2_GPIO_CLK          RCC_APB2Periph_GPIOC
#define KEY2_GPIO_PORT         GPIOC
#define KEY2_GPIO_PIN          GPIO_Pin_13

#define KEY_ON                 1
#define KEY_OFF                0

/* 表盘布局参数：屏幕为240x320竖屏，表盘放在屏幕下半部分。 */
#define CLOCK_CENTER_X         120
#define CLOCK_CENTER_Y         205
#define CLOCK_RADIUS           72
#define HOUR_HAND_LENGTH       36
#define MINUTE_HAND_LENGTH     54
#define SECOND_HAND_LENGTH     64
#define KEY_POLL_MS            20

/* 数字时间区域，只局部清除该区域，避免每秒整屏闪烁。 */
#define TIME_TEXT_X            24
#define TIME_TEXT_Y            42
#define TIME_TEXT_W            192
#define TIME_TEXT_H            34

/* 保存当前时、分、秒。 */
typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} ClockTime_t;

/* 60等分圆周的正弦表，数值放大1000倍，避免使用浮点运算。 */
static const int16_t sin60[60] = {
        0,   105,   208,   309,   407,   500,   588,   669,   743,   809,
      866,   914,   951,   978,   995,  1000,   995,   978,   951,   914,
      866,   809,   743,   669,   588,   500,   407,   309,   208,   105,
        0,  -105,  -208,  -309,  -407,  -500,  -588,  -669,  -743,  -809,
     -866,  -914,  -951,  -978,  -995, -1000,  -995,  -978,  -951,  -914,
     -866,  -809,  -743,  -669,  -588,  -500,  -407,  -309,  -208,  -105
};

/* 60等分圆周的余弦表，用于计算刻度和指针终点坐标。 */
static const int16_t cos60[60] = {
     1000,   995,   978,   951,   914,   866,   809,   743,   669,   588,
      500,   407,   309,   208,   105,     0,  -105,  -208,  -309,  -407,
     -500,  -588,  -669,  -743,  -809,  -866,  -914,  -951,  -978,  -995,
    -1000,  -995,  -978,  -951,  -914,  -866,  -809,  -743,  -669,  -588,
     -500,  -407,  -309,  -208,  -105,     0,   105,   208,   309,   407,
      500,   588,   669,   743,   809,   866,   914,   951,   978,   995
};

static void Key_GPIO_Config(void);
static uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
static void DelayMs(uint32_t ms);
static void Clock_Tick(ClockTime_t *time);
static void Clock_AddHour(ClockTime_t *time);
static void Clock_AddMinute(ClockTime_t *time);
static void DrawStaticClockLayout(void);
static void DrawClock(const ClockTime_t *time);
static void DrawTimeText(const ClockTime_t *time);
static void DrawHands(const ClockTime_t *time, uint16_t hourColor, uint16_t minuteColor, uint16_t secondColor);
static void DrawClockFace(void);
static void DrawHand(uint8_t pos, uint8_t length, uint16_t color);
static void DrawTick(uint8_t pos, uint8_t innerRadius, uint8_t outerRadius, uint16_t color);

int main(void)
{
    /* 初始时间设为12:00:00。 */
    ClockTime_t time = {12, 0, 0};
    uint16_t elapsed;

    /* 初始化LCD、设置竖屏扫描方向，并初始化按键GPIO。 */
    ILI9341_Init();
    ILI9341_GramScan(6);
    Key_GPIO_Config();

    /* 开机只清一次全屏，后续刷新采用局部更新，避免闪屏。 */
    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(WHITE);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);

    /* 固定内容只画一次：标题、按键提示、表盘外圈和刻度。 */
    DrawStaticClockLayout();
    DrawClock(&time);

    while (1)
    {
        /* 把1秒拆成多个20ms小延时，这样等待走秒时也能及时响应按键。 */
        for (elapsed = 0; elapsed < 1000; elapsed += KEY_POLL_MS)
        {
            if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON)
            {
                /* K1：小时加1，并立即刷新显示。 */
                Clock_AddHour(&time);
                DrawClock(&time);
            }

            if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON)
            {
                /* K2：分钟加1，秒清零，并立即刷新显示。 */
                Clock_AddMinute(&time);
                DrawClock(&time);
            }

            DelayMs(KEY_POLL_MS);
        }

        /* 1秒到达后，时钟自动走一秒。 */
        Clock_Tick(&time);
        DrawClock(&time);
    }
}

static void Key_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 同时打开K1、K2所在GPIO端口的时钟。 */
    RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK | KEY2_GPIO_CLK, ENABLE);

    /* 野火板按键电路已有上下拉/消抖处理，这里使用浮空输入。 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;
    GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN;
    GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);
}

static uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
    {
        /* 简单软件消抖，防止一次按下被识别成多次。 */
        DelayMs(15);

        if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
        {
            /* 等待按键释放，保证每按一次只触发一次。 */
            while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
            {
            }

            return KEY_ON;
        }
    }

    return KEY_OFF;
}

static void 


Clock_Tick(ClockTime_t *time)
{
    /* 秒加1，并处理秒、分、时的进位。 */
    time->second++;

    if (time->second >= 60)
    {
        time->second = 0;
        time->minute++;

        if (time->minute >= 60)
        {
            time->minute = 0;
            time->hour = (uint8_t)((time->hour + 1) % 24);
        }
    }
}

static void Clock_AddHour(ClockTime_t *time)
{
    /* K1调小时：23点后回到0点。 */
    time->hour = (uint8_t)((time->hour + 1) % 24);
}

static void Clock_AddMinute(ClockTime_t *time)
{
    /* K2调分钟：分钟满60后小时进位，同时秒清零。 */
    time->minute++;

    if (time->minute >= 60)
    {
        time->minute = 0;
        time->hour = (uint8_t)((time->hour + 1) % 24);
    }

    time->second = 0;
}

static void DrawStaticClockLayout(void)
{
    LCD_SetBackColor(BLACK);

    /* 顶部标题。 */
    LCD_SetFont(&Font16x24);
    LCD_SetTextColor(CYAN);
    ILI9341_DispString_EN(64, 8, "jinyifan");

    /* 底部按键说明。 */
    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(YELLOW);
    ILI9341_DispString_EN(20, 292, "K1:+hour  K2:+minute");

    /* 表盘属于静态背景，正常情况下不需要每秒整屏重画。 */
    DrawClockFace();
}

static void DrawClock(const ClockTime_t *time)
{
    static ClockTime_t lastTime = {0xFF, 0xFF, 0xFF};

    if (lastTime.hour != 0xFF)
    {
        /* 先用黑色擦掉旧指针，再补回被旧指针覆盖的刻度和外圈。 */
        DrawHands(&lastTime, BLACK, BLACK, BLACK);
        DrawClockFace();
    }

    /* 数字时间和三根指针是动态内容，每次时间变化后局部刷新。 */
    DrawTimeText(time);
    DrawHands(time, YELLOW, CYAN, RED);

    lastTime = *time;
}

static void DrawTimeText(const ClockTime_t *time)
{
    char timeText[16];

    /* 格式化成“时:分:秒”。 */
    sprintf(timeText, "%02d:%02d:%02d", time->hour, time->minute, time->second);

    /* 只清除数字时间区域，不清除整屏。 */
    LCD_SetBackColor(BLACK);
    ILI9341_Clear(TIME_TEXT_X, TIME_TEXT_Y, TIME_TEXT_W, TIME_TEXT_H);

    LCD_SetFont(&Font24x32);
    LCD_SetTextColor(GREEN);
    ILI9341_DispString_EN(TIME_TEXT_X, TIME_TEXT_Y, timeText);
}

static void DrawHands(const ClockTime_t *time, uint16_t hourColor, uint16_t minuteColor, uint16_t secondColor)
{
    uint8_t hourPos;

    /* 表盘一圈60格，1小时占5格；minute/12让时针随分钟缓慢移动。 */
    hourPos = (uint8_t)(((time->hour % 12) * 5) + (time->minute / 12));
    DrawHand(hourPos, HOUR_HAND_LENGTH, hourColor);
    DrawHand(time->minute, MINUTE_HAND_LENGTH, minuteColor);
    DrawHand(time->second, SECOND_HAND_LENGTH, secondColor);

    /* 重画中心圆点，遮住三根指针在中心的交汇毛边。 */
    LCD_SetTextColor(WHITE);
    ILI9341_DrawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, 3, 1);
}

static void DrawClockFace(void)
{
    uint8_t i;

    /* 画双层外圆，让表盘边界更清楚。 */
    LCD_SetTextColor(WHITE);
    ILI9341_DrawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, 0);
    ILI9341_DrawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS - 1, 0);

    /* 画60个刻度：整点刻度更长，普通分钟刻度更短。 */
    for (i = 0; i < 60; i++)
    {
        if ((i % 5) == 0)
        {
            DrawTick(i, CLOCK_RADIUS - 10, CLOCK_RADIUS, YELLOW);
        }
        else
        {
            DrawTick(i, CLOCK_RADIUS - 4, CLOCK_RADIUS, BLUE2);
        }
    }

    /* 只标出12、3、6、9四个数字，画面更简洁。 */
    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(WHITE);
    ILI9341_DispString_EN(CLOCK_CENTER_X - 4, CLOCK_CENTER_Y - CLOCK_RADIUS + 10, "12");
    ILI9341_DispString_EN(CLOCK_CENTER_X + CLOCK_RADIUS - 18, CLOCK_CENTER_Y - 8, "3");
    ILI9341_DispString_EN(CLOCK_CENTER_X - 4, CLOCK_CENTER_Y + CLOCK_RADIUS - 25, "6");
    ILI9341_DispString_EN(CLOCK_CENTER_X - CLOCK_RADIUS + 12, CLOCK_CENTER_Y - 8, "9");
}

static void DrawTick(uint8_t pos, uint8_t innerRadius, uint8_t outerRadius, uint16_t color)
{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;

    pos %= 60;
    /* 根据刻度序号计算线段内外端点坐标。LCD的Y轴向下，所以Y坐标用减法。 */
    x1 = (int16_t)(CLOCK_CENTER_X + (sin60[pos] * innerRadius) / 1000);
    y1 = (int16_t)(CLOCK_CENTER_Y - (cos60[pos] * innerRadius) / 1000);
    x2 = (int16_t)(CLOCK_CENTER_X + (sin60[pos] * outerRadius) / 1000);
    y2 = (int16_t)(CLOCK_CENTER_Y - (cos60[pos] * outerRadius) / 1000);

    LCD_SetTextColor(color);
    ILI9341_DrawLine((uint16_t)x1, (uint16_t)y1, (uint16_t)x2, (uint16_t)y2);
}

static void DrawHand(uint8_t pos, uint8_t length, uint16_t color)
{
    int16_t x;
    int16_t y;

    pos %= 60;
    /* 指针从圆心出发，终点由当前位置和指针长度计算得到。 */
    x = (int16_t)(CLOCK_CENTER_X + (sin60[pos] * length) / 1000);
    y = (int16_t)(CLOCK_CENTER_Y - (cos60[pos] * length) / 1000);

    LCD_SetTextColor(color);
    ILI9341_DrawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, (uint16_t)x, (uint16_t)y);
}

static void DelayMs(uint32_t ms)
{
    uint32_t i;

    /* 简单软件延时，用于按键消抖和主循环定时。 */
    while (ms--)
    {
        i = 9000;
        while (i--)
        {
            __NOP();
        }
    }
}
