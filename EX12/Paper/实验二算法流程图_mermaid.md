# STM32实验二算法流程图 Mermaid 代码

本文档依据 `EX112/User/main.c` 整理，适合放入实验报告“实验线路示图、程序算法流程图”部分。实验二提高题实现 LCD 数字时钟与 12 小时制表盘显示：顶部显示标题 `jinyifan`，中部显示 `时:分:秒`，下半部分绘制表盘、刻度和时分秒三根指针；K1 调小时，K2 调分钟并清零秒；显示刷新采用局部更新，避免每秒整屏闪烁。

## 1. 主程序总体流程图

```mermaid
flowchart TD
    A(["开始"]) --> B["定义 ClockTime_t time = 12:00:00"]
    B --> C["ILI9341_Init()<br/>初始化 LCD 控制器"]
    C --> D["ILI9341_GramScan(6)<br/>设置 240x320 竖屏扫描方向"]
    D --> E["Key_GPIO_Config()<br/>初始化 K1(PA0)、K2(PC13)"]
    E --> F["设置黑色背景并清屏一次"]
    F --> G["DrawStaticClockLayout()<br/>绘制标题、按键提示、表盘外圈和刻度"]
    G --> H["DrawClock(&time)<br/>首次绘制数字时间和三根指针"]
    H --> I["elapsed = 0"]
    I --> J{"elapsed < 1000 ms ?"}
    J -- "是" --> K{"K1 是否按下?"}
    K -- "是" --> L["Clock_AddHour(&time)<br/>小时加 1"]
    L --> M["DrawClock(&time)<br/>立即局部刷新"]
    K -- "否" --> N{"K2 是否按下?"}
    M --> N
    N -- "是" --> O["Clock_AddMinute(&time)<br/>分钟加 1，秒清零"]
    O --> P["DrawClock(&time)<br/>立即局部刷新"]
    N -- "否" --> Q["DelayMs(20)<br/>elapsed += 20"]
    P --> Q
    Q --> J
    J -- "否" --> R["Clock_Tick(&time)<br/>自动走一秒"]
    R --> S["DrawClock(&time)<br/>局部刷新显示"]
    S --> I
```

适用说明：
- 程序把 1 秒拆成 50 个 20 ms 轮询周期，因此等待走秒时仍能及时响应按键。
- 开机只执行一次全屏清屏，后续只刷新数字时间区域和指针，避免整屏闪烁。
- K1 调整小时，K2 调整分钟，分钟调整后秒清零，符合手动校时习惯。

## 2. 按键扫描与消抖流程图

```mermaid
flowchart TD
    A["调用 Key_Scan(GPIOx, GPIO_Pin)"] --> B{"读取引脚是否为 KEY_ON?"}
    B -- "否" --> C["返回 KEY_OFF"]
    B -- "是" --> D["DelayMs(15)<br/>软件消抖"]
    D --> E{"再次读取是否仍为 KEY_ON?"}
    E -- "否" --> C
    E -- "是" --> F{"按键是否仍未释放?"}
    F -- "是" --> F
    F -- "否" --> G["返回 KEY_ON<br/>一次按下只触发一次"]
```

适用说明：
- 野火指南者板上 K1、K2 按下为高电平，所以 `KEY_ON` 定义为 1。
- 等待按键释放的设计可以防止长按时在主循环中被连续识别成多次短按。

## 3. 时间更新流程图

```mermaid
flowchart TD
    A["Clock_Tick"] --> B["second 加 1"]
    B --> C{"second >= 60 ?"}
    C -- "否" --> H["结束"]
    C -- "是" --> D["second = 0<br/>minute 加 1"]
    D --> E{"minute >= 60 ?"}
    E -- "否" --> H
    E -- "是" --> F["minute = 0"]
    F --> G["hour = (hour + 1) % 24"]
    G --> H
```

```mermaid
flowchart TD
    A["K1 调小时<br/>Clock_AddHour"] --> B["hour = (hour + 1) % 24"]
    B --> C["返回并刷新显示"]

    D["K2 调分钟<br/>Clock_AddMinute"] --> E["minute 加 1"]
    E --> F{"minute >= 60 ?"}
    F -- "是" --> G["minute = 0<br/>hour = (hour + 1) % 24"]
    F -- "否" --> H["保持当前小时"]
    G --> I["second = 0"]
    H --> I
    I --> J["返回并刷新显示"]
```

适用说明：
- 程序内部按 24 小时制保存时间。
- 表盘绘制时再通过 `hour % 12` 转换为 12 小时制位置。

## 4. LCD 局部刷新流程图

```mermaid
flowchart TD
    A["调用 DrawClock(time)"] --> B{"lastTime 是否有效?"}
    B -- "是" --> C["用黑色重画旧时针、旧分针、旧秒针<br/>擦除旧指针"]
    C --> D["DrawClockFace()<br/>补回被旧指针覆盖的刻度、外圈和数字"]
    B -- "否" --> E["首次绘制，不需要擦旧指针"]
    D --> F["DrawTimeText(time)<br/>只清除 TIME_TEXT 区域并重写数字时间"]
    E --> F
    F --> G["DrawHands(time, YELLOW, CYAN, RED)<br/>绘制新时针、分针、秒针"]
    G --> H["lastTime = time<br/>保存本次时间"]
    H --> I["返回主循环"]
```

适用说明：
- 旧指针用黑色擦除，新指针用不同颜色绘制。
- 数字时间区域通过 `ILI9341_Clear(TIME_TEXT_X, TIME_TEXT_Y, TIME_TEXT_W, TIME_TEXT_H)` 局部清除。
- 该方法避免每秒调用整屏清屏，因此表盘不会出现明显闪烁。

## 5. 表盘绘制与指针坐标计算流程图

```mermaid
flowchart TD
    A["DrawClockFace"] --> B["绘制双层外圆<br/>半径 72 和 71"]
    B --> C["i = 0"]
    C --> D{"i < 60 ?"}
    D -- "是" --> E{"i % 5 == 0 ?"}
    E -- "是" --> F["DrawTick(i, R-10, R, YELLOW)<br/>绘制整点长刻度"]
    E -- "否" --> G["DrawTick(i, R-4, R, BLUE2)<br/>绘制分钟短刻度"]
    F --> H["i 加 1"]
    G --> H
    H --> D
    D -- "否" --> I["绘制 12、3、6、9 四个数字"]
    I --> J["结束"]
```

```mermaid
flowchart TD
    A["DrawHands(time)"] --> B["hourPos = (hour % 12) * 5 + minute / 12"]
    B --> C["DrawHand(hourPos, 36, YELLOW)<br/>绘制时针"]
    C --> D["DrawHand(minute, 54, CYAN)<br/>绘制分针"]
    D --> E["DrawHand(second, 64, RED)<br/>绘制秒针"]
    E --> F["重画中心圆点<br/>遮住三针交汇处毛边"]
```

```mermaid
flowchart TD
    A["DrawHand(pos, length, color)"] --> B["pos = pos % 60"]
    B --> C["x = centerX + sin60[pos] * length / 1000"]
    C --> D["y = centerY - cos60[pos] * length / 1000"]
    D --> E["ILI9341_DrawLine(centerX, centerY, x, y)"]
```

适用说明：
- `sin60[]` 和 `cos60[]` 是把圆周 60 等分后的查表结果，数值放大 1000 倍。
- LCD 坐标系的 Y 轴向下，所以计算 Y 坐标时使用 `centerY - cos60[pos] * length / 1000`。
- 时针位置加入 `minute / 12`，使时针不会只在整点跳变，而是随分钟缓慢移动。

## 6. LCD 时钟模块连接示意图

```mermaid
flowchart LR
    subgraph MCU["STM32F103 指南者开发板"]
        FSMC["FSMC 并行接口"]
        PA0["PA0 / K1"]
        PC13["PC13 / K2"]
        SYS["系统时钟与软件延时"]
    end

    subgraph LCD["3.2寸 ILI9341 LCD"]
        GRAM["显存 GRAM"]
        PIX["240x320 像素显示区"]
        FACE["表盘、数字时间、指针"]
    end

    subgraph KEY["板载按键"]
        K1["K1<br/>小时加 1"]
        K2["K2<br/>分钟加 1<br/>秒清零"]
    end

    FSMC --> GRAM
    GRAM --> PIX
    PIX --> FACE
    PA0 --> K1
    PC13 --> K2
    SYS -->|"20 ms 轮询<br/>约 1 s 走秒"| FACE
```

说明：
- LCD 由 `ILI9341_Init()` 完成初始化，由 `ILI9341_GramScan(6)` 设置竖屏扫描方向。
- 固定内容包括标题、底部按键提示、表盘外圆和刻度；动态内容包括数字时间和三根指针。

## 7. 报告中推荐放置方式

- 主程序总体流程图：说明 LCD 时钟的初始化、按键轮询和走秒逻辑。
- LCD 局部刷新流程图：突出“不整屏清屏、不闪烁”的改进点。
- 表盘绘制与指针坐标计算流程图：说明表盘、刻度和三根指针的具体绘制方法。
- LCD 时钟模块连接示意图：作为实验线路/模块关系示意图使用。
