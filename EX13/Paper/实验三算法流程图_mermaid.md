# STM32实验三提高题算法流程图 Mermaid 代码

本文档依据 `EX13/User/main.c` 整理，只描述实验三 A/D 实验提高题：PC1 采集模拟电压、超过阈值红灯报警、LCD 绘制实时电压曲线，并通过串口输出采样结果。

## 1. 主程序总体流程图

```mermaid
flowchart TD
    A(["开始"]) --> B["USART_Config()<br/>初始化串口输出"]
    B --> C["LED_GPIO_Config()<br/>初始化 RGB LED"]
    C --> D["ADC_PC1_Config()<br/>PC1 配置为 ADC 模拟输入"]
    D --> E["ILI9341_Init()<br/>LCD 初始化"]
    E --> F["ILI9341_GramScan(6)<br/>设置竖屏显示"]
    F --> G["清屏并绘制静态界面<br/>标题、坐标框、阈值线"]
    G --> H["读取 ADC 原始值 raw"]
    H --> I["换算电压<br/>voltageMv = raw * 3300 / 4095"]
    I --> J["滑动平均滤波<br/>filteredMv = MovingAverage_Update(voltageMv)"]
    J --> K{"filteredMv >= 1500mV ?"}
    K -- "是" --> L["红色 LED 点亮<br/>状态显示 ALARM"]
    K -- "否" --> M["RGB LED 熄灭<br/>状态显示 NORMAL"]
    L --> N["LCD 更新数值和状态"]
    M --> N
    N --> O["LCD 绘制实时电压曲线"]
    O --> P["串口打印 ADC、电压和报警状态"]
    P --> Q["DelayMs(50)"]
    Q --> H
```

## 2. ADC 单次采样流程图

```mermaid
flowchart TD
    A["ADC_PC1_ReadRaw"] --> B["启动软件转换<br/>ADC_SoftwareStartConvCmd"]
    B --> C{"EOC 转换完成标志为 1 ?"}
    C -- "否" --> C
    C -- "是" --> D["读取 ADC 转换结果<br/>ADC_GetConversionValue"]
    D --> E["返回 0-4095 原始采样值"]
```

## 3. 火警报警判断流程图

```mermaid
flowchart TD
    A["获得滤波电压 filteredMv"] --> B{"filteredMv >= 1500mV ?"}
    B -- "是" --> C["alarm = 1"]
    C --> D["LED_SetColor(RED)<br/>LCD 显示 ALARM"]
    B -- "否" --> E["alarm = 0"]
    E --> F["LED_SetColor(OFF)<br/>LCD 显示 NORMAL"]
```

## 4. LCD 简易示波器绘制流程图

```mermaid
flowchart TD
    A["Scope_Plot(voltageMv)"] --> B{"scopeX == 0 ?"}
    B -- "是" --> C["清除绘图区并重画网格、边框、1.5V阈值线"]
    B -- "否" --> D["保留当前绘图区"]
    C --> E["根据电压计算 y 坐标"]
    D --> E
    E --> F{"是否已有上一个点 ?"}
    F -- "是" --> G["连接上一个点和当前点"]
    F -- "否" --> H["绘制当前点"]
    G --> I["保存当前点为 lastX/lastY"]
    H --> I
    I --> J["scopeX 加 1"]
    J --> K{"scopeX >= 绘图区宽度 ?"}
    K -- "是" --> L["scopeX = 0<br/>下一轮重新开始扫描"]
    K -- "否" --> M["等待下一次采样"]
```

## 5. 硬件连接示意图

```mermaid
flowchart LR
    subgraph MCU["STM32F103 指南者开发板"]
        PC1["PC1 / ADC Channel 11"]
        PB5["PB5 / 红色 LED"]
        USART1["USART1 / USB转串口"]
        FSMC["FSMC LCD 接口"]
    end

    subgraph POT["外部可变电阻"]
        VCC["3.3V"]
        MID["滑动端"]
        GND["GND"]
    end

    subgraph OUT["显示与报警"]
        LED["板载 RGB 红灯"]
        LCD["ILI9341 LCD<br/>实时电压曲线"]
        PC["串口助手<br/>ADC 和电压值"]
    end

    VCC --- POT
    POT --- GND
    MID --> PC1
    PB5 --> LED
    FSMC --> LCD
    USART1 --> PC
```

说明：
- 可变电阻两端接 3.3V 和 GND，中间滑动端接 PC1。
- PC1 对应 ADC 通道 11，采样范围为 0-3.3V。
- 当滤波电压大于等于 1.5V 时，板载 RGB 红灯点亮报警。
