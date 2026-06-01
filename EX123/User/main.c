// 实验三提高题（2）：简易 LCD 示波器
// PC1(ADC2 Channel 11) 采集可变电阻/传感器电压。
// 程序把 ADC 采样值换算成电压，并在 ILI9341 LCD 上绘制实时曲线。
//
// 硬件接线：
// 可变电阻/传感器中间脚 -> PC1
// 可变电阻/传感器两端 -> 3.3V 和 GND

#include "stm32f10x.h"
#include "./usart/bsp_usart.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include <stdio.h>

__IO uint16_t ADC_ConvertedValue = 0;

// ADC 参考电压为 3.3V，12 位 ADC 满量程约为 4096 个量化等级。
#define ADC_VREF                3.3f
#define ADC_FULL_SCALE          4096.0f

// K1 切换显示量程：3.3V -> 2.0V -> 1.0V。
// K2 打开/关闭软件低通滤波。
static const float g_scopeRanges[] = {3.3f, 2.0f, 1.0f};
#define SCOPE_RANGE_COUNT       (sizeof(g_scopeRanges) / sizeof(g_scopeRanges[0]))

// 曲线绘图区的位置和尺寸。横屏时屏幕尺寸为 320x240。
#define PLOT_X0                 20
#define PLOT_Y0                 40
#define PLOT_WIDTH              280
#define PLOT_HEIGHT             160
#define PLOT_Y_BOTTOM           (PLOT_Y0 + PLOT_HEIGHT)

// 一阶低通滤波系数：越小越平滑，但响应越慢。
#define FILTER_ALPHA_NEW        0.10f

#define KEY1_GPIO_CLK           RCC_APB2Periph_GPIOA
#define KEY1_GPIO_PORT          GPIOA
#define KEY1_GPIO_PIN           GPIO_Pin_0

#define KEY2_GPIO_CLK           RCC_APB2Periph_GPIOC
#define KEY2_GPIO_PORT          GPIOC
#define KEY2_GPIO_PIN           GPIO_Pin_13

#define KEY_ON                  1
#define KEY_OFF                 0

/* 1/3/5/7 都是横屏方向。
 * 这块 LCD 使用 7 会出现左右镜像，所以这里用 5 保持横屏且文字正常。
 */
#define LCD_LANDSCAPE_SCAN      5

static void Delay(__IO uint32_t nCount)
{
    // 简单空循环延时，用于控制曲线刷新速度。
    while (nCount--);
}

static void DelayMs(uint32_t ms)
{
    uint32_t i;

    // 粗略毫秒延时，用在按键消抖中。
    while (ms--)
    {
        i = 9000;
        while (i--)
        {
            __NOP();
        }
    }
}

static void Key_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // K1 接 PA0，K2 接 PC13，按下为高电平。
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
        // 简单软件消抖，避免机械按键抖动造成误触发。
        DelayMs(15);

        if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
        {
            // 等待按键释放，保证一次按下只触发一次。
            while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
            {
            }

            return KEY_ON;
        }
    }

    return KEY_OFF;
}

static void ADC_PC1_Init(void)                                           // 初始化 PC1 对应的 ADC2 通道 11。
{                                                                        // ADC 初始化函数开始。
    GPIO_InitTypeDef GPIO_InitStructure;                                 // 定义 GPIO 初始化结构体，用来配置 PC1 引脚。
    ADC_InitTypeDef ADC_InitStructure;                                   // 定义 ADC 初始化结构体，用来配置 ADC2 工作模式。
    NVIC_InitTypeDef NVIC_InitStructure;                                 // 定义 NVIC 初始化结构体，用来配置 ADC 中断。

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC2, ENABLE); // 打开 GPIOC 和 ADC2 的外设时钟。
                                                                        // 外部晶振 HSE -> PLL 倍频 -> 系统时钟 SYSCLK -> APB2 总线时钟 -> GPIOC / ADC2

    RCC_ADCCLKConfig(RCC_PCLK2_Div8);                                    // 设置 ADC 时钟为 PCLK2/8，72MHz/8=9MHz。

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;                            // 选择 PC1 引脚。
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                        // 设置 PC1 为模拟输入模式，供 ADC 采样。
    GPIO_Init(GPIOC, &GPIO_InitStructure);                               // 按上述参数初始化 GPIOC 的 PC1。

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                   // ADC2 使用独立模式，不与其他 ADC 联动。
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                        // 关闭扫描模式，因为这里只采一个通道。
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                   // 开启连续转换，启动后 ADC 会一直采样。
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 不使用外部触发，改用软件启动转换。
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               // ADC 结果右对齐，读出的值范围为 0~4095。
                                                                         // ADC 数据寄存器通常是 16 位的，但有效数据只有 12 位
    ADC_InitStructure.ADC_NbrOfChannel = 1;                              // 规则序列中只有 1 个转换通道。
    ADC_Init(ADC2, &ADC_InitStructure);                                  // 把上述配置写入 ADC2。

    ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5); // 配置 ADC2 的第 1 个规则通道为 PC1 对应的通道 11。
                                                                         // PC1 这个物理引脚 -> ADC 内部的通道 11
    ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);                              // 使能 EOC 转换完成中断，每次采样完成后触发中断。

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);                      // 设置中断优先级分组。
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;                    // 选择 ADC1 和 ADC2 共用的中断通道。
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;            // 设置抢占优先级为 1。
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;                   // 设置响应优先级为 1。
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                      // 使能这个中断通道。
    NVIC_Init(&NVIC_InitStructure);                                      // 把上述中断配置写入 NVIC。

    ADC_Cmd(ADC2, ENABLE);                                               // 使能 ADC2 外设。

    ADC_ResetCalibration(ADC2);                                          // 复位 ADC2 校准寄存器。
    while (ADC_GetResetCalibrationStatus(ADC2));                         // 等待校准寄存器复位完成。

    ADC_StartCalibration(ADC2);                                          // 启动 ADC2 自校准。
    while (ADC_GetCalibrationStatus(ADC2));                              // 等待 ADC2 自校准完成。

    ADC_SoftwareStartConvCmd(ADC2, ENABLE);                              // 软件启动第一次转换，之后连续转换会自动进行。
}

void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC2, ADC_IT_EOC) == SET)
    {
        // ADC 转换完成后进入中断，读取 12 位转换结果。
        ADC_ConvertedValue = ADC_GetConversionValue(ADC2);
        ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
    }
}

static float ADC_ToVoltage(uint16_t adcValue)
{
    // 将 0~4095 的 ADC 原始值换算为 0~3.3V 电压。
    return (float)adcValue * ADC_VREF / ADC_FULL_SCALE;
}

static uint16_t Voltage_ToY(float voltage, float rangeMax)
{
    float limitedVoltage = voltage;
    uint16_t y;

    // 超出当前量程的电压按边界处理，避免曲线画出显示区域。
    if (limitedVoltage < 0.0f)
    {
        limitedVoltage = 0.0f;
    }
    else if (limitedVoltage > rangeMax)
    {
        limitedVoltage = rangeMax;
    }

    // LCD 的 Y 轴向下，所以电压越高，Y 坐标越小。
    y = (uint16_t)(PLOT_Y_BOTTOM - (limitedVoltage * PLOT_HEIGHT / rangeMax));

    if (y < PLOT_Y0)
    {
        y = PLOT_Y0;
    }
    else if (y > PLOT_Y_BOTTOM)
    {
        y = PLOT_Y_BOTTOM;
    }

    return y;
}

static void Scope_DrawFrame(float rangeMax, uint8_t filterEnabled)
{
    char text[40];

    // 重新绘制整张示波器界面，常在切换量程/滤波状态或曲线扫到末尾时调用。
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);

    LCD_SetFont(&Font8x16);
    LCD_SetColors(WHITE, BLACK);
    ILI9341_DispString_EN(8, 8, "ADC simple oscilloscope");

    sprintf(text, "Range:0-%.1fV  Filter:%s", rangeMax, filterEnabled ? "ON" : "OFF");
    ILI9341_DispString_EN(8, 24, text);
    ILI9341_DispString_EN(8, 224, "K1:Range  K2:Filter");

    LCD_SetTextColor(GREY);
    ILI9341_DrawRectangle(PLOT_X0, PLOT_Y0, PLOT_WIDTH, PLOT_HEIGHT, 0);

    LCD_SetTextColor(GREEN);
    ILI9341_DrawLine(PLOT_X0, PLOT_Y_BOTTOM, PLOT_X0 + PLOT_WIDTH, PLOT_Y_BOTTOM);
    ILI9341_DrawLine(PLOT_X0, PLOT_Y0, PLOT_X0, PLOT_Y_BOTTOM);
}

static void Scope_UpdateText(uint16_t adcValue, float voltage, float displayVoltage)
{
    char text[48];

    // 只刷新底部数值区域，不重画整个屏幕，减少闪烁。
    LCD_SetFont(&Font8x16);
    LCD_SetColors(YELLOW, BLACK);

    ILI9341_Clear(8, 204, 260, 18);

    sprintf(text, "ADC:%4u  Vin:%.3fV", adcValue, voltage);
    ILI9341_DispString_EN(8, 204, text);

    sprintf(text, "Draw: %.3fV", displayVoltage);
    ILI9341_DispString_EN(160, 204, text);
}

int main(void)
{
    float voltage;
    float filteredVoltage = 0.0f;
    float displayVoltage;
    float rangeMax = g_scopeRanges[0];
    uint16_t x = PLOT_X0;
    uint16_t lastX = PLOT_X0;
    uint16_t y;
    uint16_t lastY = PLOT_Y_BOTTOM;
    uint16_t sampleCount = 0;
    uint8_t rangeIndex = 0;
    uint8_t filterEnabled = 1;

    USART_Config();
    Key_GPIO_Config();
    ADC_PC1_Init();

    ILI9341_Init();
    // 横屏显示，且避免镜像文字。
    ILI9341_GramScan(LCD_LANDSCAPE_SCAN);
    Scope_DrawFrame(rangeMax, filterEnabled);

    printf("\r\n ---- ADC LCD simple oscilloscope ----\r\n");
    printf("Input: PC1 / ADC_Channel_11\r\n");

    filteredVoltage = ADC_ToVoltage(ADC_ConvertedValue);
    lastY = Voltage_ToY(filteredVoltage, rangeMax);

    while (1)
    {
        // K1：循环切换显示量程。切换后重画坐标框，并从左侧重新开始绘制曲线。
        if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON)
        {
            rangeIndex = (uint8_t)((rangeIndex + 1) % SCOPE_RANGE_COUNT);
            rangeMax = g_scopeRanges[rangeIndex];
            Scope_DrawFrame(rangeMax, filterEnabled);
            x = PLOT_X0;
            lastX = x;
            lastY = Voltage_ToY(filteredVoltage, rangeMax);
        }

        // K2：开关滤波。切换时用当前 ADC 值重置滤波输出，避免曲线突然跳变太大。
        if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON)
        {
            filterEnabled = !filterEnabled;
            filteredVoltage = ADC_ToVoltage(ADC_ConvertedValue);
            Scope_DrawFrame(rangeMax, filterEnabled);
            x = PLOT_X0;
            lastX = x;
            lastY = Voltage_ToY(filteredVoltage, rangeMax);
        }

        voltage = ADC_ToVoltage(ADC_ConvertedValue);
        if (filterEnabled)
        {
            // 一阶低通滤波：新显示值 = 90%旧值 + 10%新采样值。
            // 这样曲线更平滑，但电压快速变化时响应会稍慢。
            filteredVoltage = filteredVoltage * (1.0f - FILTER_ALPHA_NEW) +
                              voltage * FILTER_ALPHA_NEW;
            displayVoltage = filteredVoltage;
        }
        else
        {
            filteredVoltage = voltage;
            displayVoltage = voltage;
        }

        y = Voltage_ToY(displayVoltage, rangeMax);

        if (x >= (PLOT_X0 + PLOT_WIDTH))
        {
            // 曲线扫到右边界后清屏重画坐标框，从左侧重新开始。
            Scope_DrawFrame(rangeMax, filterEnabled);
            x = PLOT_X0;
            lastX = x;
            lastY = y;
        }

        // 画新曲线前清除一条窄区域，避免旧曲线残留。
        ILI9341_Clear(x + 1, PLOT_Y0 + 1, 2, PLOT_HEIGHT - 1);

        LCD_SetTextColor(CYAN);
        ILI9341_DrawLine(lastX, lastY, x, y);

        lastX = x;
        lastY = y;
        x++;

        if ((sampleCount++ % 12) == 0)
        {
            // 降低文字和串口输出频率，避免刷新过快造成卡顿。
            Scope_UpdateText(ADC_ConvertedValue, voltage, displayVoltage);
            printf("ADC = %4u, Voltage = %.3f V, Draw = %.3f V, Range = %.1f V, Filter = %s\r\n",
                   ADC_ConvertedValue,
                   voltage,
                   displayVoltage,
                   rangeMax,
                   filterEnabled ? "ON" : "OFF");
        }

        Delay(0x8FFFF);
    }
}
