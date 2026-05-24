#include "stm32f10x.h"
#include "./led/bsp_led.h"
#include "./key/bsp_key.h"

#define PURE_COLOR_COUNT       3
#define MIXED_COLOR_COUNT      4
#define PURE_HOLD_TIME_MS      1000
#define KEY_POLL_TIME_MS       10

typedef enum
{
    MODE_PURE = 0,
    MODE_MIXED
} DisplayMode_t;

static const LedColor_t pureColors[PURE_COLOR_COUNT] = {
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE
};

static const LedColor_t mixedColors[MIXED_COLOR_COUNT] = {
    LED_COLOR_YELLOW,
    LED_COLOR_PURPLE,
    LED_COLOR_CYAN,
    LED_COLOR_WHITE
};

static void DelayMs(uint32_t ms);
static void HandlePureMode(DisplayMode_t *mode, uint8_t *pureIndex, int8_t *pureStep);
static void HandleMixedMode(DisplayMode_t *mode, uint8_t *mixedIndex);

int main(void)
{
    DisplayMode_t mode = MODE_PURE;
    uint8_t pureIndex = 0;
    uint8_t mixedIndex = 0;
    int8_t pureStep = 1;

    LED_GPIO_Config();
    Key_GPIO_Config();

    while (1)
    {
        if (mode == MODE_PURE)
        {
            HandlePureMode(&mode, &pureIndex, &pureStep);
        }
        else
        {
            HandleMixedMode(&mode, &mixedIndex);
        }
    }
}

static void HandlePureMode(DisplayMode_t *mode, uint8_t *pureIndex, int8_t *pureStep)
{
    uint16_t elapsed;

    LED_SetColor(pureColors[*pureIndex]);

    for (elapsed = 0; elapsed < PURE_HOLD_TIME_MS; elapsed += KEY_POLL_TIME_MS)
    {
        if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON)
        {
            *pureStep = -*pureStep;
        }

        if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON)
        {
            *mode = MODE_MIXED;
            return;
        }

        DelayMs(KEY_POLL_TIME_MS);
    }

    if (*pureStep > 0)
    {
        *pureIndex = (uint8_t)((*pureIndex + 1) % PURE_COLOR_COUNT);
    }
    else
    {
        *pureIndex = (uint8_t)((*pureIndex + PURE_COLOR_COUNT - 1) % PURE_COLOR_COUNT);
    }
}

static void HandleMixedMode(DisplayMode_t *mode, uint8_t *mixedIndex)
{
    LED_SetColor(mixedColors[*mixedIndex]);

    if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON)
    {
        *mixedIndex = (uint8_t)((*mixedIndex + 1) % MIXED_COLOR_COUNT);
        LED_SetColor(mixedColors[*mixedIndex]);
    }

    if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON)
    {
        *mode = MODE_PURE;
    }

    DelayMs(KEY_POLL_TIME_MS);
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
