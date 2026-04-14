# 实验二算法流程图 Mermaid 代码

本文档依据《2026微控制器与嵌入式系统实验任务书（适用于自动化、测控2024级）》和《单片机实验指导》实验二要求整理，包含实验二基础题与提高题的算法流程图以及提高题 Proteus 接线示意，便于直接复制到支持 Mermaid 的编辑器中渲染。

## 1. 基础题1：定时器中断方波输出流程图

```mermaid
flowchart TD
    A[开始] --> B[初始化 TH0 TL0 TH1 TL1]
    B --> C[设置 TMOD = 11H]
    C --> D[设置 TCON = 50H 启动 T0 T1]
    D --> E[设置 IE = 8AH 开总中断和定时器中断]
    E --> F[主循环空转等待中断]
    F --> G{定时器0溢出?}
    G -- 是 --> H[进入 T0 中断服务程序]
    H --> I[Wave1 取反]
    I --> J[重装 TH0 TL0 为 F8 00]
    J --> F
    G -- 否 --> K{定时器1溢出?}
    K -- 是 --> L[进入 T1 中断服务程序]
    L --> M[Wave2 取反]
    M --> N[重装 TH1 TL1 为 F8 00]
    N --> F
    K -- 否 --> F
```

适用说明：
- 对应 `EX2_Int1.c`
- 输出端口为 `P1.0` 与 `P1.1`
- 结果可在 Keil Logic Analyzer 中观察两路方波

## 2. 基础题2：外部中断 LED 控制流程图

```mermaid
flowchart TD
    A[开始] --> B[P1 置 00H]
    B --> C[设置 IT0 = 1 IT1 = 1]
    C --> D[设置 EX0 = 1 EX1 = 1 EA = 1]
    D --> E[主循环等待中断]
    E --> F{INT0 下降沿到来?}
    F -- 是 --> G[进入 INT0 服务程序]
    G --> H[循环 4 次]
    H --> I[P1 = FFH]
    I --> J[调用 delay]
    J --> K[P1 = 00H]
    K --> L[调用 delay]
    L --> M{4 次完成?}
    M -- 否 --> I
    M -- 是 --> E
    F -- 否 --> N{INT1 下降沿到来?}
    N -- 是 --> O[进入 INT1 服务程序]
    O --> P[置 i = 03H j = 0]
    P --> Q[P1 = i]
    Q --> R[i 循环左移 2 位]
    R --> S[调用 delay]
    S --> T[j 加 1]
    T --> U{j < 16 ?}
    U -- 是 --> Q
    U -- 否 --> V[P1 = 00H]
    V --> E
    N -- 否 --> E
```

适用说明：
- 对应 `EX2_INT2.c`
- `INT0` 触发 8 个 LED 全亮全灭闪烁 4 次
- `INT1` 触发两灯一组循环左移显示

## 3. 提高题：交通灯主状态机流程图

```mermaid
flowchart TD
    A[开始] --> B[初始化 Timer0 中断与 INT0]
    B --> C[初始状态设为 NS_GREEN]
    C --> D[remain_seconds = 8]
    D --> E[显示当前灯态]
    E --> F[主循环]
    F --> G{emergency_req = 1 且当前不在急救模式?}
    G -- 是 --> H[保存当前 state 和 remain_seconds]
    H --> I[emergency_active = 1]
    I --> J[tick_50ms 清零 second_flag 清零]
    J --> K[remain_seconds = 10]
    K --> L[全部红灯亮]
    L --> M{second_flag = 1 ?}
    G -- 否 --> M
    M -- 否 --> F
    M -- 是 --> N[second_flag 清零]
    N --> O{remain_seconds > 0 ?}
    O -- 是 --> P[remain_seconds 减 1]
    O -- 否 --> Q
    P --> Q{remain_seconds = 0 ?}
    Q -- 否 --> F
    Q -- 是 --> R{emergency_active = 1 ?}
    R -- 是 --> S[清 emergency_active]
    S --> T[恢复 saved_state 和 saved_remain]
    T --> U[显示恢复后的灯态]
    U --> F
    R -- 否 --> V[按状态机切到下一个状态]
    V --> W[NS_GREEN→NS_YELLOW→EW_GREEN→EW_YELLOW→NS_GREEN]
    W --> X[装入对应持续时间 8s/2s]
    X --> Y[显示新灯态]
    Y --> F
```

适用说明：
- 对应 `EX2_Traffic_Emergency.c`
- `P1.0~P1.5` 分别驱动南北、东西两个方向的红黄绿灯
- `INT0` 作为急救车优先请求

## 4. 提高题：Timer0 与 INT0 协同流程图

```mermaid
flowchart LR
    A[Timer0 周期中断] --> B[tick_50ms 加 1]
    B --> C{达到节拍阈值?}
    C -- 否 --> D[返回主程序]
    C -- 是 --> E[tick_50ms 清零]
    E --> F[second_flag 置 1]
    F --> D

    G[INT0 下降沿到来] --> H[INT0 服务程序]
    H --> I[emergency_req 置 1]
    I --> J[立即返回主程序]

    K[主循环检测] --> L{emergency_req = 1?}
    L -- 是 --> M[转入全红 10 秒状态]
    L -- 否 --> N[继续按 second_flag 更新正常状态]
```

适用说明：
- 本图强调“中断提出请求，主循环完成控制”的设计思路
- 该结构避免在中断服务程序内执行复杂延时与状态恢复逻辑

## 5. 提高题 Proteus 交通灯接线示意图

```mermaid
flowchart LR
    subgraph MCU[AT89C51RD2 / 8051兼容单片机]
        P10[P1.0]
        P11[P1.1]
        P12[P1.2]
        P13[P1.3]
        P14[P1.4]
        P15[P1.5]
        INT0[P3.2 / INT0]
        XT1[XTAL1]
        XT2[XTAL2]
        RST[RST]
        EA[EA]
    end

    subgraph NS[南北方向交通灯]
        NSR[红灯]
        NSY[黄灯]
        NSG[绿灯]
    end

    subgraph EW[东西方向交通灯]
        EWR[红灯]
        EWY[黄灯]
        EWG[绿灯]
    end

    subgraph CLK[时钟与复位]
        CRY[晶振]
        C1[22pF]
        C2[22pF]
        RB[10k下拉]
        SWR[复位按钮]
        VCC1[POWER]
        GND1[GND]
    end

    subgraph EMG[急救车请求]
        RINT[10k上拉]
        SWI[按键]
        VCC2[POWER]
        GND2[GND]
    end

    P10 --> NSR
    P11 --> NSY
    P12 --> NSG
    P13 --> EWR
    P14 --> EWY
    P15 --> EWG

    XT1 --- CRY --- XT2
    XT1 --- C1 --- GND1
    XT2 --- C2 --- GND1
    RST --- RB --- GND1
    RST --- SWR --- VCC1
    EA --- VCC1

    INT0 --- RINT --- VCC2
    INT0 --- SWI --- GND2
```

适用说明：
- 该接线示意与已保存的 Proteus 工程 `EX2_Traffic_Emergency.pdsprj` 一致
- `INT0` 采用按键下拉方式形成下降沿触发
- 复位键采用高电平复位，与 `INT0` 按键的极性不同

## 6. 报告中推荐放置方式

- 基础题至少放 2 张流程图：
  - 定时器中断方波输出流程图
  - 外部中断 LED 控制流程图
- 提高题建议放 2 张图：
  - 交通灯主状态机流程图
  - Proteus 交通灯接线示意图
- 若版面允许，可再补充“Timer0 与 INT0 协同流程图”以突出设计思路
