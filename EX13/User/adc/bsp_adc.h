#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f10x.h"

/* PC1 is ADC channel 11 on STM32F103VE. */
#define ADC_PC1_GPIO_CLK        RCC_APB2Periph_GPIOC
#define ADC_PC1_GPIO_PORT       GPIOC
#define ADC_PC1_GPIO_PIN        GPIO_Pin_1

#define ADC_PC1                 ADC1
#define ADC_PC1_CLK             RCC_APB2Periph_ADC1
#define ADC_PC1_CHANNEL         ADC_Channel_11

void ADC_PC1_Config(void);
uint16_t ADC_PC1_ReadRaw(void);
uint16_t ADC_RawToMilliVolt(uint16_t rawValue);

#endif /* __BSP_ADC_H */
