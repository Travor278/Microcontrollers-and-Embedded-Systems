// 实验三提高题（1）：简易火警报警器
// PC1(ADC_Channel_11) 采集可变电阻/烟雾传感器输出电压。
// 电压升高到 1.50 V 及以上时，红灯点亮报警；
// 电压降低到 1.40 V 及以下时，报警解除。

#include "stm32f10x.h"
#include "./usart/bsp_usart.h"
#include "./adc/bsp_adc.h"
#include <stdio.h>

extern __IO uint16_t ADC_ConvertedValue;

#define ADC_VREF                3.3f
#define ADC_FULL_SCALE          4096.0f

// 使用两个阈值形成“迟滞”：
// 进入报警阈值高一点，退出报警阈值低一点，避免电压在 1.5V 附近抖动时红灯反复闪烁。
#define ALARM_ON_VOLTAGE        1.50f
#define ALARM_OFF_VOLTAGE       1.40f

// 野火指南者板载 RGB 灯中，红灯接 PB5，且为低电平点亮。
#define RED_LED_GPIO_CLK        RCC_APB2Periph_GPIOB
#define RED_LED_GPIO_PORT       GPIOB
#define RED_LED_GPIO_PIN        GPIO_Pin_5

static void Delay(__IO uint32_t nCount)
{
    // 简单软件延时，用于降低串口打印频率，方便观察。
    while (nCount--);
}

static void RedLed_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RED_LED_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = RED_LED_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RED_LED_GPIO_PORT, &GPIO_InitStructure);

    GPIO_SetBits(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN);
}

static void RedLed_On(void)
{
    // 板载红灯低电平有效，所以 ResetBits 是点亮。
    GPIO_ResetBits(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN);
}

static void RedLed_Off(void)
{
    // SetBits 输出高电平，红灯熄灭。
    GPIO_SetBits(RED_LED_GPIO_PORT, RED_LED_GPIO_PIN);
}

static float ADC_ToVoltage(uint16_t adcValue)
{
    // 12 位 ADC 的原始值范围为 0~4095，对应 0~3.3V。
    return (float)adcValue * ADC_VREF / ADC_FULL_SCALE;
}

int main(void)
{
    float voltage;
    uint8_t alarmState = 0;

    USART_Config();
    RedLed_Init();
    ADCx_Init();

    printf("\r\n ---- ADC fire alarm experiment ----\r\n");
    printf("Input: PC1 / ADC_Channel_11\r\n");
    printf("Alarm on : %.2f V\r\n", ALARM_ON_VOLTAGE);
    printf("Alarm off: %.2f V\r\n\r\n", ALARM_OFF_VOLTAGE);

    while (1)
    {
        // ADC_ConvertedValue 在 ADC 中断服务函数中更新，这里只负责换算和判断。
        voltage = ADC_ToVoltage(ADC_ConvertedValue);

        // 未报警时，电压达到进入阈值才点亮红灯。
        if (!alarmState && voltage >= ALARM_ON_VOLTAGE)
        {
            alarmState = 1;
            RedLed_On();
        }
        // 已报警时，电压降到退出阈值才关闭红灯。
        else if (alarmState && voltage <= ALARM_OFF_VOLTAGE)
        {
            alarmState = 0;
            RedLed_Off();
        }

        // 串口输出当前 ADC 原始值、电压值和报警状态。
        printf("ADC = %4u, Voltage = %.3f V, State = %s\r\n",
               ADC_ConvertedValue,
               voltage,
               alarmState ? "ALARM" : "NORMAL");

        Delay(0x2FFFFF);
    }
}

