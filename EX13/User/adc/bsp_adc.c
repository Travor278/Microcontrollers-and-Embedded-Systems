#include "./adc/bsp_adc.h"

void ADC_PC1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(ADC_PC1_GPIO_CLK | ADC_PC1_CLK, ENABLE);

    /* ADC clock must not exceed 14 MHz. PCLK2/8 = 9 MHz when SYSCLK is 72 MHz. */
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    GPIO_InitStructure.GPIO_Pin = ADC_PC1_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ADC_PC1_GPIO_PORT, &GPIO_InitStructure);

    ADC_DeInit(ADC_PC1);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC_PC1, &ADC_InitStructure);

    /* Longer sample time makes the potentiometer input more stable. */
    ADC_RegularChannelConfig(ADC_PC1, ADC_PC1_CHANNEL, 1, ADC_SampleTime_239Cycles5);

    ADC_Cmd(ADC_PC1, ENABLE);

    ADC_ResetCalibration(ADC_PC1);
    while (ADC_GetResetCalibrationStatus(ADC_PC1) == SET)
    {
    }

    ADC_StartCalibration(ADC_PC1);
    while (ADC_GetCalibrationStatus(ADC_PC1) == SET)
    {
    }
}

uint16_t ADC_PC1_ReadRaw(void)
{
    ADC_SoftwareStartConvCmd(ADC_PC1, ENABLE);

    while (ADC_GetFlagStatus(ADC_PC1, ADC_FLAG_EOC) == RESET)
    {
    }

    return ADC_GetConversionValue(ADC_PC1);
}

uint16_t ADC_RawToMilliVolt(uint16_t rawValue)
{
    return (uint16_t)(((uint32_t)rawValue * 3300U) / 4095U);
}
