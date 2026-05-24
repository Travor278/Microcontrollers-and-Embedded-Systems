#include "bsp_led.h"

void LED_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LED1_GPIO_CLK | LED2_GPIO_CLK | LED3_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = LED1_GPIO_PIN;
    GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LED2_GPIO_PIN;
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LED3_GPIO_PIN;
    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);

    LED_RGBOFF;
}

void LED_SetColor(LedColor_t color)
{
    switch (color)
    {
    case LED_COLOR_RED:
        LED_RED;
        break;

    case LED_COLOR_GREEN:
        LED_GREEN;
        break;

    case LED_COLOR_BLUE:
        LED_BLUE;
        break;

    case LED_COLOR_YELLOW:
        LED_YELLOW;
        break;

    case LED_COLOR_PURPLE:
        LED_PURPLE;
        break;

    case LED_COLOR_CYAN:
        LED_CYAN;
        break;

    case LED_COLOR_WHITE:
        LED_WHITE;
        break;

    case LED_COLOR_OFF:
    default:
        LED_RGBOFF;
        break;
    }
}
