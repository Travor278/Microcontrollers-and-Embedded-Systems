# STM32实验一算法流程图 Mermaid 代码

本文档依据 `EX11/User/main.c`、`bsp_led.h` 和 `bsp_key.h` 整理，适合放入实验报告“实验线路示图、程序算法流程图”部分。实验一提高题实现 RGB LED 双模式控制：模式一为红、绿、蓝纯色循环，模式二为黄、紫、青、白混色切换；K1 用于改变当前模式内的显示状态，K2 用于切换模式。

## 1. 主程序总体流程图

```mermaid
flowchart TD
    A(["开始"]) --> B["定义状态变量<br/>mode = MODE_PURE<br/>pureIndex = 0<br/>mixedIndex = 0<br/>pureStep = 1"]
    B --> C["调用 LED_GPIO_Config()<br/>初始化 PB5/PB0/PB1 为推挽输出"]
    C --> D["调用 Key_GPIO_Config()<br/>初始化 PA0(K1)、PC13(K2) 为输入"]
    D --> E{"mode 是否为 MODE_PURE?"}
    E -- "是" --> F["进入纯色循环模式<br/>HandlePureMode()"]
    E -- "否" --> G["进入混色显示模式<br/>HandleMixedMode()"]
    F --> E
    G --> E
```

适用说明：
- `mode` 是主状态变量，用来区分当前处于纯色循环模式还是混色显示模式。
- `pureIndex` 表示当前纯色序号，0/1/2 分别对应红、绿、蓝。
- `mixedIndex` 表示当前混色序号，0/1/2/3 分别对应黄、紫、青、白。
- `pureStep` 表示纯色循环方向，值为 `1` 时正向循环，值为 `-1` 时反向循环。

## 2. 提高题模式一：纯色循环流程图

```mermaid
flowchart TD
    A["进入 HandlePureMode"] --> B["根据 pureIndex 输出当前纯色<br/>LED_SetColor(pureColors[pureIndex])"]
    B --> C["elapsed = 0"]
    C --> D{"elapsed < 1000 ms ?"}
    D -- "否" --> I{"pureStep > 0 ?"}
    D -- "是" --> E{"K1 是否按下?"}
    E -- "是" --> F["pureStep = -pureStep<br/>改变红绿蓝循环方向"]
    E -- "否" --> G{"K2 是否按下?"}
    F --> G
    G -- "是" --> H["mode = MODE_MIXED<br/>立即返回主循环"]
    G -- "否" --> J["DelayMs(10)<br/>elapsed += 10"]
    J --> D
    I -- "是" --> K["pureIndex = (pureIndex + 1) % 3"]
    I -- "否" --> L["pureIndex = (pureIndex + 3 - 1) % 3"]
    K --> M["返回主循环"]
    L --> M
    H --> M
```

适用说明：
- 每种纯色理论保持 1 秒，但程序没有直接阻塞 1 秒，而是拆成 100 次 10 ms 轮询。
- 在 1 秒等待过程中仍持续扫描 K1/K2，所以按键响应不会等到 1 秒结束才发生。
- K1 不直接改变颜色，而是改变后续循环方向，例如红 -> 绿 -> 蓝可以变为红 -> 蓝 -> 绿。
- K2 在任意 10 ms 轮询周期内按下，都会立即切换到混色模式。

## 3. 提高题模式二：混色切换流程图

```mermaid
flowchart TD
    A["进入 HandleMixedMode"] --> B["根据 mixedIndex 输出当前混色<br/>LED_SetColor(mixedColors[mixedIndex])"]
    B --> C{"K1 是否按下?"}
    C -- "是" --> D["mixedIndex = (mixedIndex + 1) % 4"]
    D --> E["立即显示新的混色"]
    C -- "否" --> F{"K2 是否按下?"}
    E --> F
    F -- "是" --> G["mode = MODE_PURE<br/>切回纯色循环模式"]
    F -- "否" --> H["DelayMs(10)"]
    G --> H
    H --> I["返回主循环"]
```

适用说明：
- 混色数组 `mixedColors` 中的顺序为黄、紫、青、白。
- K1 每按一次，混色序号加 1 并对 4 取余，实现循环切换。
- K2 用于返回模式一，返回后纯色循环会从当前 `pureIndex` 和 `pureStep` 继续运行。

## 4. RGB LED 与按键控制逻辑图

```mermaid
flowchart LR
    subgraph MCU["STM32F103 指南者开发板"]
        PB5["PB5 / LED1 红色通道"]
        PB0["PB0 / LED2 绿色通道"]
        PB1["PB1 / LED3 蓝色通道"]
        PA0["PA0 / K1"]
        PC13["PC13 / K2"]
    end

    subgraph RGB["板载 RGB LED<br/>低电平点亮"]
        R["红色 LED"]
        G["绿色 LED"]
        B["蓝色 LED"]
    end

    subgraph KEY["板载按键<br/>按下为高电平"]
        K1["K1<br/>模式一:改变方向<br/>模式二:下一混色"]
        K2["K2<br/>切换模式一/模式二"]
    end

    PB5 --> R
    PB0 --> G
    PB1 --> B
    PA0 --> K1
    PC13 --> K2
```

说明：
- RGB LED 为低电平点亮，因此驱动函数内部通过 `BRR` 置低电平点亮，通过 `BSRR` 置高电平熄灭。
- 红、绿、蓝三个通道组合后可显示混色：
  - 黄 = 红 + 绿
  - 紫 = 红 + 蓝
  - 青 = 绿 + 蓝
  - 白 = 红 + 绿 + 蓝

## 5. 模式状态转换图

```mermaid
stateDiagram-v2
    [*] --> MODE_PURE: 上电默认
    MODE_PURE --> MODE_PURE: K1按下 / pureStep取反
    MODE_PURE --> MODE_MIXED: K2按下
    MODE_MIXED --> MODE_MIXED: K1按下 / mixedIndex加1
    MODE_MIXED --> MODE_PURE: K2按下
```

适用说明：
- 该图适合在报告中解释 `mode` 状态变量的来源和变化。
- `mode` 不是由硬件自动产生的，而是程序根据 K2 按键扫描结果在两个枚举值之间切换。

## 6. 报告中推荐放置方式

- 主程序总体流程图：说明 `main()` 的整体结构。
- 纯色循环流程图：说明 K1 改变循环方向、K2 切换模式。
- 混色切换流程图：说明 K1 切换混色、K2 返回纯色模式。
- RGB LED 与按键控制逻辑图：作为实验线路示意图使用。
