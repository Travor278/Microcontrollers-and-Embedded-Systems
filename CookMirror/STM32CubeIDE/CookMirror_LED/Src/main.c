#include <stdint.h>

#define RCC_APB2ENR  (*(volatile uint32_t *)0x40021018U)

#define GPIOC_CRH    (*(volatile uint32_t *)0x40011004U)
#define GPIOC_BSRR   (*(volatile uint32_t *)0x40011010U)
#define GPIOC_BRR    (*(volatile uint32_t *)0x40011014U)

#define RCC_IOPCEN   (1U << 4)
#define LED_PIN      13U
#define HALF_PERIOD_DELAY 100000U
#define RESET_OFF_DELAY   300000U

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}

void SystemInit(void)
{
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPCEN;

    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x2U << 20);
    GPIOC_BRR = (1U << LED_PIN);
    delay(RESET_OFF_DELAY);

    while (1) {
        GPIOC_BSRR = (1U << LED_PIN);
        delay(HALF_PERIOD_DELAY);

        GPIOC_BRR = (1U << LED_PIN);
        delay(HALF_PERIOD_DELAY);
    }
}
