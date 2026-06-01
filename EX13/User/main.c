#include "stm32f10x.h"
#include "./adc/bsp_adc.h"
#include "./led/bsp_led.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./usart/bsp_usart.h"
#include <stdio.h>

#define SAMPLE_PERIOD_MS        50U
#define FILTER_LENGTH           8U
#define ALARM_THRESHOLD_MV      1500U
#define ADC_FULL_SCALE_MV       3300U

#define SCOPE_X                 18U
#define SCOPE_Y                 102U
#define SCOPE_W                 204U
#define SCOPE_H                 150U

#define TEXT_X                  18U
#define TEXT_Y                  36U
#define TEXT_W                  204U
#define TEXT_H                  54U

static void DelayMs(uint32_t ms);
static void DrawStaticLayout(void);
static void DrawScopeGrid(void);
static void DrawRealtimeText(uint16_t rawValue, uint16_t voltageMv, uint8_t alarm);
static void DrawErrorMessage(char *message);
static void Scope_Plot(uint16_t voltageMv);
static uint16_t MovingAverage_Update(uint16_t value);
static uint16_t VoltageToY(uint16_t voltageMv);

int main(void)
{
    uint16_t rawValue;
    uint16_t voltageMv;
    uint16_t filteredMv;
    uint8_t alarm;

    LED_GPIO_Config();
    USART_Config();

    /* 先把LCD点亮，后续如果ADC初始化失败，屏幕能显示错误而不是白屏。 */
    ILI9341_Init();
    ILI9341_GramScan(6);
    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(WHITE);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    LCD_SetFont(&Font16x24);
    LCD_SetTextColor(CYAN);
    ILI9341_DispString_EN(34, 8, "EX3 ADC Scope");
    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(YELLOW);
    ILI9341_DispString_EN(18, 42, "LCD OK, init ADC...");

    if (ADC_PC1_Config() != ADC_PC1_OK)
    {
        LED_SetColor(LED_COLOR_RED);
        DrawErrorMessage("ADC init timeout");
        printf("\r\nADC init timeout. Check project and ADC clock.\r\n");

        while (1)
        {
        }
    }

    DrawStaticLayout();

    printf("\r\nSTM32 EX13 ADC fire alarm and LCD scope\r\n");
    printf("PC1 -> ADC channel 11, threshold = %u.%03u V\r\n",
           (unsigned int)(ALARM_THRESHOLD_MV / 1000U),
           (unsigned int)(ALARM_THRESHOLD_MV % 1000U));

    while (1)
    {
        if (ADC_PC1_ReadRaw(&rawValue) != ADC_PC1_OK)
        {
            LED_SetColor(LED_COLOR_RED);
            DrawErrorMessage("ADC read timeout");
            printf("ADC read timeout\r\n");
            DelayMs(200);
            continue;
        }

        voltageMv = ADC_RawToMilliVolt(rawValue);
        filteredMv = MovingAverage_Update(voltageMv);

        alarm = (filteredMv >= ALARM_THRESHOLD_MV) ? 1U : 0U;
        LED_SetColor(alarm ? LED_COLOR_RED : LED_COLOR_OFF);

        DrawRealtimeText(rawValue, filteredMv, alarm);
        Scope_Plot(filteredMv);

        printf("ADC=%4u, Voltage=%u.%03u V, State=%s\r\n",
               (unsigned int)rawValue,
               (unsigned int)(filteredMv / 1000U),
               (unsigned int)(filteredMv % 1000U),
               alarm ? "ALARM" : "NORMAL");

        DelayMs(SAMPLE_PERIOD_MS);
    }
}

static void DrawStaticLayout(void)
{
    LCD_SetBackColor(BLACK);

    LCD_SetFont(&Font16x24);
    LCD_SetTextColor(CYAN);
    ILI9341_DispString_EN(34, 8, "EX3 ADC Scope");

    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(WHITE);
    ILI9341_DispString_EN(18, 270, "PC1 input, range 0-3.3V");
    ILI9341_DispString_EN(18, 288, "Red LED on when V >= 1.5V");

    DrawScopeGrid();
}

static void DrawScopeGrid(void)
{
    uint8_t i;
    uint16_t y;

    ILI9341_Clear(SCOPE_X, SCOPE_Y, SCOPE_W, SCOPE_H);

    LCD_SetTextColor(WHITE);
    ILI9341_DrawRectangle(SCOPE_X, SCOPE_Y, SCOPE_W, SCOPE_H, 0);

    LCD_SetTextColor(BLUE2);
    for (i = 1; i < 4; i++)
    {
        y = (uint16_t)(SCOPE_Y + (SCOPE_H * i) / 4U);
        ILI9341_DrawLine(SCOPE_X + 1U, y, SCOPE_X + SCOPE_W - 2U, y);
    }

    LCD_SetFont(&Font8x16);
    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(WHITE);
    ILI9341_DispString_EN(2, SCOPE_Y - 8U, "3.3");
    ILI9341_DispString_EN(2, VoltageToY(ALARM_THRESHOLD_MV) - 8U, "1.5");
    ILI9341_DispString_EN(6, SCOPE_Y + SCOPE_H - 12U, "0");

    LCD_SetTextColor(RED);
    y = VoltageToY(ALARM_THRESHOLD_MV);
    ILI9341_DrawLine(SCOPE_X + 1U, y, SCOPE_X + SCOPE_W - 2U, y);
}

static void DrawRealtimeText(uint16_t rawValue, uint16_t voltageMv, uint8_t alarm)
{
    char line[32];

    ILI9341_Clear(TEXT_X, TEXT_Y, TEXT_W, TEXT_H);

    LCD_SetBackColor(BLACK);
    LCD_SetFont(&Font8x16);

    LCD_SetTextColor(WHITE);
    sprintf(line, "ADC: %4u", (unsigned int)rawValue);
    ILI9341_DispString_EN(TEXT_X, TEXT_Y, line);

    sprintf(line, "Volt: %u.%03u V",
            (unsigned int)(voltageMv / 1000U),
            (unsigned int)(voltageMv % 1000U));
    ILI9341_DispString_EN(TEXT_X, TEXT_Y + 18U, line);

    LCD_SetTextColor(alarm ? RED : GREEN);
    sprintf(line, "State: %s", alarm ? "ALARM" : "NORMAL");
    ILI9341_DispString_EN(TEXT_X, TEXT_Y + 36U, line);
}

static void DrawErrorMessage(char *message)
{
    ILI9341_Clear(TEXT_X, TEXT_Y, TEXT_W, TEXT_H);
    LCD_SetBackColor(BLACK);
    LCD_SetFont(&Font8x16);
    LCD_SetTextColor(RED);
    ILI9341_DispString_EN(TEXT_X, TEXT_Y, message);
}

static void Scope_Plot(uint16_t voltageMv)
{
    static uint16_t scopeX = 0;
    static uint16_t lastX = 0;
    static uint16_t lastY = 0;
    static uint8_t hasLast = 0;
    uint16_t x;
    uint16_t y;

    if (scopeX == 0U)
    {
        DrawScopeGrid();
        hasLast = 0;
    }

    x = (uint16_t)(SCOPE_X + scopeX);
    y = VoltageToY(voltageMv);

    LCD_SetTextColor(GREEN);
    if (hasLast)
    {
        ILI9341_DrawLine(lastX, lastY, x, y);
    }
    else
    {
        ILI9341_SetPointPixel(x, y);
        hasLast = 1;
    }

    lastX = x;
    lastY = y;

    scopeX++;
    if (scopeX >= SCOPE_W)
    {
        scopeX = 0;
    }
}

static uint16_t MovingAverage_Update(uint16_t value)
{
    static uint16_t buffer[FILTER_LENGTH];
    static uint32_t sum = 0;
    static uint8_t index = 0;
    static uint8_t count = 0;

    sum -= buffer[index];
    buffer[index] = value;
    sum += value;

    index++;
    if (index >= FILTER_LENGTH)
    {
        index = 0;
    }

    if (count < FILTER_LENGTH)
    {
        count++;
    }

    return (uint16_t)(sum / count);
}

static uint16_t VoltageToY(uint16_t voltageMv)
{
    uint32_t yOffset;

    if (voltageMv > ADC_FULL_SCALE_MV)
    {
        voltageMv = ADC_FULL_SCALE_MV;
    }

    yOffset = ((uint32_t)voltageMv * (SCOPE_H - 1U)) / ADC_FULL_SCALE_MV;
    return (uint16_t)(SCOPE_Y + SCOPE_H - 1U - yOffset);
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
