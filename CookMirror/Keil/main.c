#include <stdint.h>

#define RCC_APB2ENR  (*(volatile uint32_t *)0x40021018U)

#define GPIOC_CRH    (*(volatile uint32_t *)0x40011004U)
#define GPIOC_BSRR   (*(volatile uint32_t *)0x40011010U)
#define GPIOC_BRR    (*(volatile uint32_t *)0x40011014U)

#define RCC_IOPCEN   (1U << 4)
#define LED_PIN      13U

static void delay(volatile uint32_t count)
{
    while (count--) {
    }
}

int main(void)
{
    RCC_APB2ENR |= RCC_IOPCEN;

    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x2U << 20);

    while (1) {
        GPIOC_BSRR = (1U << LED_PIN);
        delay(500000U);

        GPIOC_BRR = (1U << LED_PIN);
        delay(500000U);
    }
}
