#include "stm32f10x.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include <stdio.h>

#define KEY1_GPIO_CLK          RCC_APB2Periph_GPIOA
#define KEY1_GPIO_PORT         GPIOA
#define KEY1_GPIO_PIN          GPIO_Pin_0

#define KEY2_GPIO_CLK          RCC_APB2Periph_GPIOC
#define KEY2_GPIO_PORT         GPIOC
#define KEY2_GPIO_PIN          GPIO_Pin_13

#define KEY_ON                 1
#define KEY_OFF                0

#define CLOCK_CENTER_X         120
#define CLOCK_CENTER_Y         205
#define CLOCK_RADIUS           72
#define HOUR_HAND_LENGTH       36
#define MINUTE_HAND_LENGTH     54
#define SECOND_HAND_LENGTH     64
#define KEY_POLL_MS            20

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} ClockTime_t;

static const int16_t sin60[60] = {
        0,   105,   208,   309,   407,   500,   588,   669,   743,   809,
      866,   914,   951,   978,   995,  1000,   995,   978,   951,   914,
      866,   809,   743,   669,   588,   500,   407,   309,   208,   105,
        0,  -105,  -208,  -309,  -407,  -500,  -588,  -669,  -743,  -809,
     -866,  -914,  -951,  -978,  -995, -1000,  -995,  -978,  -951,  -914,
     -866,  -809,  -743,  -669,  -588,  -500,  -407,  -309,  -208,  -105
};

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
static void DrawClock(const ClockTime_t *time);
static void DrawClockFace(void);
static void DrawHand(uint8_t pos, uint8_t length, uint16_t color);
static void DrawTick(uint8_t pos, uint8_t innerRadius, uint8_t outerRadius, uint16_t color);

int main(void)
{
    ClockTime_t time = {12, 0, 0};
    uint16_t elapsed;

    ILI9341_Init();
    ILI9341_GramScan(6);
    Key_GPIO_Config();

    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(WHITE);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);

    DrawClock(&time);

    while (1)
    {
        for (elapsed = 0; elapsed < 1000; elapsed += KEY_POLL_MS)
        {
            if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON)
            {
                Clock_AddHour(&time);
                DrawClock(&time);
            }

            if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON)
            {
                Clock_AddMinute(&time);
                DrawClock(&time);
            }

            DelayMs(KEY_POLL_MS);
        }

        Clock_Tick(&time);
        DrawClock(&time);
    }
}

static void Key_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK | KEY2_GPIO_CLK, ENABLE);

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
        DelayMs(15);

        if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
        {
            while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
            {
            }

            return KEY_ON;
        }
    }

    return KEY_OFF;
}

static void Clock_Tick(ClockTime_t *time)
{
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
    time->hour = (uint8_t)((time->hour + 1) % 24);
}

static void Clock_AddMinute(ClockTime_t *time)
{
    time->minute++;

    if (time->minute >= 60)
    {
        time->minute = 0;
        time->hour = (uint8_t)((time->hour + 1) % 24);
    }

    time->second = 0;
}

static void DrawClock(const ClockTime_t *time)
{
    char timeText[16];
    uint8_t hourPos;

    LCD_SetBackColor(BLACK);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);

    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(CYAN);
    ILI9341_DispString_EN(56, 8, "EX2 LCD Clock");

    LCD_SetTextColor(YELLOW);
    ILI9341_DispString_EN(20, 292, "K1:+hour  K2:+minute");

    sprintf(timeText, "%02d:%02d:%02d", time->hour, time->minute, time->second);
    LCD_SetFont(&Font24x32);
    LCD_SetTextColor(GREEN);
    ILI9341_DispString_EN(24, 42, timeText);

    DrawClockFace();

    hourPos = (uint8_t)(((time->hour % 12) * 5) + (time->minute / 12));
    DrawHand(hourPos, HOUR_HAND_LENGTH, YELLOW);
    DrawHand(time->minute, MINUTE_HAND_LENGTH, CYAN);
    DrawHand(time->second, SECOND_HAND_LENGTH, RED);

    LCD_SetTextColor(WHITE);
    ILI9341_DrawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, 3, 1);
}

static void DrawClockFace(void)
{
    uint8_t i;

    LCD_SetTextColor(WHITE);
    ILI9341_DrawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, 0);
    ILI9341_DrawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS - 1, 0);

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
    x = (int16_t)(CLOCK_CENTER_X + (sin60[pos] * length) / 1000);
    y = (int16_t)(CLOCK_CENTER_Y - (cos60[pos] * length) / 1000);

    LCD_SetTextColor(color);
    ILI9341_DrawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, (uint16_t)x, (uint16_t)y);
}

static void DelayMs(uint32_t ms)
{
    uint32_t i;

    while (ms--)
    {
        i = 9000;
        while (i--)
        {
            __NOP();
        }
    }
}
