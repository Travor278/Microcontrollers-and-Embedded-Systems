#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f10x.h"

/* RGB LED on EmbedFire STM32 F103 Guide board.
 * The LED pins are active-low.
 */
#define LED1_GPIO_PORT        GPIOB
#define LED1_GPIO_CLK         RCC_APB2Periph_GPIOB
#define LED1_GPIO_PIN         GPIO_Pin_5

#define LED2_GPIO_PORT        GPIOB
#define LED2_GPIO_CLK         RCC_APB2Periph_GPIOB
#define LED2_GPIO_PIN         GPIO_Pin_0

#define LED3_GPIO_PORT        GPIOB
#define LED3_GPIO_CLK         RCC_APB2Periph_GPIOB
#define LED3_GPIO_PIN         GPIO_Pin_1

#define digitalHi(p, i)       ((p)->BSRR = (i))
#define digitalLo(p, i)       ((p)->BRR = (i))
#define digitalToggle(p, i)   ((p)->ODR ^= (i))

#define LED1_ON               digitalLo(LED1_GPIO_PORT, LED1_GPIO_PIN)
#define LED1_OFF              digitalHi(LED1_GPIO_PORT, LED1_GPIO_PIN)
#define LED1_TOGGLE           digitalToggle(LED1_GPIO_PORT, LED1_GPIO_PIN)

#define LED2_ON               digitalLo(LED2_GPIO_PORT, LED2_GPIO_PIN)
#define LED2_OFF              digitalHi(LED2_GPIO_PORT, LED2_GPIO_PIN)
#define LED2_TOGGLE           digitalToggle(LED2_GPIO_PORT, LED2_GPIO_PIN)

#define LED3_ON               digitalLo(LED3_GPIO_PORT, LED3_GPIO_PIN)
#define LED3_OFF              digitalHi(LED3_GPIO_PORT, LED3_GPIO_PIN)
#define LED3_TOGGLE           digitalToggle(LED3_GPIO_PORT, LED3_GPIO_PIN)

#define LED_RED               do { LED1_ON;  LED2_OFF; LED3_OFF; } while (0)
#define LED_GREEN             do { LED1_OFF; LED2_ON;  LED3_OFF; } while (0)
#define LED_BLUE              do { LED1_OFF; LED2_OFF; LED3_ON;  } while (0)
#define LED_YELLOW            do { LED1_ON;  LED2_ON;  LED3_OFF; } while (0)
#define LED_PURPLE            do { LED1_ON;  LED2_OFF; LED3_ON;  } while (0)
#define LED_CYAN              do { LED1_OFF; LED2_ON;  LED3_ON;  } while (0)
#define LED_WHITE             do { LED1_ON;  LED2_ON;  LED3_ON;  } while (0)
#define LED_RGBOFF            do { LED1_OFF; LED2_OFF; LED3_OFF; } while (0)

typedef enum
{
    LED_COLOR_OFF = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_YELLOW,
    LED_COLOR_PURPLE,
    LED_COLOR_CYAN,
    LED_COLOR_WHITE
} LedColor_t;

void LED_GPIO_Config(void);
void LED_SetColor(LedColor_t color);

#endif /* __BSP_LED_H */
