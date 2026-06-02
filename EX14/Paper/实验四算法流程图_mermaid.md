# STM32实验四 WiFi实验算法流程图 Mermaid 代码

本文档依据 `EX14/Paper/stm32_exp4_report.docx` 整理，适合放入实验报告“三、实验线路示图、程序算法流程图”部分。实验四实现 STM32 通过 ESP8266 连接手机热点，手机网络调试 APP 作为 TCP 服务端发送 ASCII 指令，开发板解析指令并控制板载 RGB LED；提高题扩展为多块开发板之间通过 WiFi 命令帧互相控制 RGB 灯。

## 1. 主程序总体流程图

```mermaid
flowchart TD
    A(["开始"]) --> B["LED_GPIO_Config()<br/>初始化 RGB LED 三个通道"]
    B --> C["USART_Config()<br/>初始化调试串口"]
    C --> D["ESP8266_USART_Config()<br/>初始化 ESP8266 通信串口"]
    D --> E["清空串口接收缓冲区<br/>设置连接状态为未连接"]
    E --> F["ESP8266_InitAndConnect()<br/>发送 AT 指令并连接 WiFi/TCP"]
    F --> G{"TCP 连接是否成功?"}
    G -- "否" --> H["串口打印错误信息<br/>延时后重新连接"]
    H --> F
    G -- "是" --> I["串口提示 WiFi Ready<br/>RGB LED 默认熄灭"]
    I --> J{"是否收到完整命令?"}
    J -- "否" --> K["检测连接状态<br/>必要时发送心跳或重连"]
    K --> J
    J -- "是" --> L["ExtractCommand()<br/>提取 ASCII 控制命令"]
    L --> M["RGB_ExecuteCommand(cmd)<br/>执行颜色/闪烁控制"]
    M --> N["SendAck()<br/>返回 OK 或 ACK 应答"]
    N --> J
```

适用说明：
- 初始化顺序为 LED、调试串口、ESP8266 串口、WiFi/TCP 连接，先保证本地外设可用，再进入网络通信。
- 主循环不直接阻塞等待某一个字节，而是持续检查接收缓冲区是否已经组成完整命令。
- 若 ESP8266 掉线或 TCP 断开，程序回到连接流程重新执行 AT 指令，避免实验过程中只能重新上电恢复。

## 2. ESP8266 AT 指令连接流程图

```mermaid
flowchart TD
    A["进入 ESP8266_InitAndConnect"] --> B["发送 AT<br/>检测模块是否响应 OK"]
    B --> C{"AT 是否成功?"}
    C -- "否" --> D["模块无响应<br/>检查供电、串口线、波特率"]
    D --> Z["返回连接失败"]
    C -- "是" --> E["发送 ATE0<br/>关闭回显，减少解析干扰"]
    E --> F["发送 AT+CWMODE=1<br/>设置为 STA 客户端模式"]
    F --> G["发送 AT+CWJAP<br/>连接手机热点 SSID/PWD"]
    G --> H{"热点连接成功?"}
    H -- "否" --> I["等待超时或返回 ERROR<br/>提示检查热点名和密码"]
    I --> Z
    H -- "是" --> J["发送 AT+CIFSR<br/>读取本机 IP，便于调试"]
    J --> K["发送 AT+CIPSTART<br/>连接手机 TCP Server IP:8000"]
    K --> L{"TCP 是否 CONNECT?"}
    L -- "否" --> M["提示检查手机 APP 是否监听<br/>检查 IP 和端口"]
    M --> Z
    L -- "是" --> N["发送 AT+CIPMODE=1 或保持普通模式<br/>按例程要求设置透传/非透传"]
    N --> O["连接完成<br/>返回成功"]
```

适用说明：
- `AT` 用来确认 ESP8266 模块和串口链路正常。
- `ATE0` 关闭回显后，STM32 接收缓冲区中不会重复出现自己发送的 AT 指令，后续解析更干净。
- `AT+CWMODE=1` 表示 ESP8266 作为 STA 连接手机热点；手机网络调试 APP 作为 TCP Server，开发板作为 TCP Client。
- 端口按实验要求使用 `8000`，IP 地址应填写手机 APP 显示的服务端地址或热点网关下分配的地址。

## 3. 网络接收与命令解析流程图

```mermaid
flowchart TD
    A["USART 接收中断或轮询读到 1 字节"] --> B{"字节是否有效?"}
    B -- "否" --> C["丢弃异常字节"]
    B -- "是" --> D["写入 rxBuffer<br/>rxIndex 加 1"]
    D --> E{"是否遇到结束符?<br/>\\r\\n 或 #"}
    E -- "否" --> F{"rxIndex 是否达到缓冲区上限?"}
    F -- "否" --> G["继续接收下一字节"]
    F -- "是" --> H["缓冲区溢出<br/>清空并记录错误"]
    E -- "是" --> I["形成一条完整命令"]
    I --> J["去除 +IPD、空格、\\r\\n 等无关字符"]
    J --> K{"是否为普通 LED_xxx 命令?"}
    K -- "是" --> L["传入 RGB_ExecuteCommand"]
    K -- "否" --> M{"是否为提高题帧<br/>@目标ID:命令# ?"}
    M -- "是" --> N["传入 Frame_Process"]
    M -- "否" --> O["返回 ERR<br/>命令不识别"]
    L --> P["清空 rxBuffer<br/>准备接收下一条"]
    N --> P
    O --> P
    C --> G
    H --> G
```

适用说明：
- TCP 是字节流，不能认为一次串口接收就等于一条完整命令，因此需要接收缓冲区和结束符判断。
- 基本题可直接使用 `LED_RED`、`LED_GREEN` 等字符串；提高题推荐使用 `@B:LED_RED#` 这种带帧头帧尾的格式。
- 若 APP 采用 ESP8266 非透传模式，接收数据前可能带有 `+IPD` 前缀，解析时应先提取真正的数据字段。

## 4. RGB LED 指令执行流程图

```mermaid
flowchart TD
    A["RGB_ExecuteCommand(cmd)"] --> B{"cmd == LED_RED ?"}
    B -- "是" --> C["红灯亮<br/>绿灯、蓝灯灭"]
    B -- "否" --> D{"cmd == LED_GREEN ?"}
    D -- "是" --> E["绿灯亮<br/>红灯、蓝灯灭"]
    D -- "否" --> F{"cmd == LED_BLUE ?"}
    F -- "是" --> G["蓝灯亮<br/>红灯、绿灯灭"]
    F -- "否" --> H{"cmd == LED_YELLOW ?"}
    H -- "是" --> I["红灯+绿灯亮<br/>显示黄色"]
    H -- "否" --> J{"cmd == LED_PURPLE ?"}
    J -- "是" --> K["红灯+蓝灯亮<br/>显示紫色"]
    J -- "否" --> L{"cmd == LED_CYAN ?"}
    L -- "是" --> M["绿灯+蓝灯亮<br/>显示青色"]
    L -- "否" --> N{"cmd == LED_WHITE ?"}
    N -- "是" --> O["红绿蓝全亮<br/>显示白色"]
    N -- "否" --> P{"cmd == LED_RGBOFF ?"}
    P -- "是" --> Q["红绿蓝全灭"]
    P -- "否" --> R{"cmd == LED_BLINK ?"}
    R -- "是" --> S["进入闪烁模式<br/>按固定周期翻转当前颜色"]
    R -- "否" --> T["返回错误<br/>未知命令"]
```

适用说明：
- 野火指南者板载 RGB LED 通常为低电平点亮，实际程序中应调用已有 LED 宏或封装函数，避免直接写 GPIO 时把亮灭逻辑写反。
- 黄、紫、青、白属于三色 LED 的组合显示：黄 = 红 + 绿，紫 = 红 + 蓝，青 = 绿 + 蓝，白 = 红 + 绿 + 蓝。
- `LED_BLINK` 是提高题可选扩展动作，可让被控端在收到命令后进入周期闪烁状态。

## 5. 提高题多开发板组网流程图

```mermaid
flowchart TD
    A(["多板实验开始"]) --> B["所有开发板连接同一手机热点"]
    B --> C["为每块开发板设置 LOCAL_ID<br/>例如 A、B、C"]
    C --> D{"通信结构选择"}
    D -- "手机 APP 转发/中心服务" --> E["各开发板作为 TCP Client<br/>连接手机 TCP Server"]
    D -- "开发板 A 作为控制端" --> F["A 建立 TCP Server 或连接中心<br/>B/C 作为被控节点"]
    E --> G["控制端输入命令帧<br/>@目标ID:命令#"]
    F --> G
    G --> H["网络发送到目标节点或中心转发"]
    H --> I["接收端 Frame_Process(frame)"]
    I --> J{"目标ID == LOCAL_ID<br/>或目标ID == ALL ?"}
    J -- "否" --> K["丢弃或转发该帧<br/>本机不执行"]
    J -- "是" --> L["解析命令字段<br/>调用 RGB_ExecuteCommand"]
    L --> M["发送应答帧<br/>@LOCAL_ID:ACK:命令#"]
    M --> N["控制端显示 ACK<br/>确认控制成功"]
    K --> O["继续监听下一帧"]
    N --> O
```

适用说明：
- 帧格式建议为 `@目标ID:命令#`，例如 `@B:LED_BLUE#` 控制 B 板蓝灯点亮，`@ALL:LED_RGBOFF#` 广播关闭所有 RGB 灯。
- 加入目标 ID 后，多块开发板接入同一网络时不会因为普通字符串命令产生歧义。
- 加入 ACK 应答后，控制端可以判断目标节点是否收到命令；若超时未收到 ACK，可重发或提示离线。

## 6. 命令帧解析状态机流程图

```mermaid
stateDiagram-v2
    [*] --> WAIT_HEAD
    WAIT_HEAD --> READ_TARGET: 收到 @
    WAIT_HEAD --> WAIT_HEAD: 其他字符丢弃
    READ_TARGET --> READ_COMMAND: 收到 :
    READ_TARGET --> WAIT_HEAD: 目标字段过长或非法
    READ_COMMAND --> FRAME_DONE: 收到 #
    READ_COMMAND --> WAIT_HEAD: 命令字段过长或非法
    FRAME_DONE --> EXECUTE: 校验目标ID和命令
    EXECUTE --> SEND_ACK: 本机需要执行
    EXECUTE --> WAIT_HEAD: 本机不需要执行
    SEND_ACK --> WAIT_HEAD: 应答发送完成
```

适用说明：
- 状态机比一次性字符串查找更适合串口/WiFi 字节流，因为它能处理半包、粘包和杂散字符。
- `WAIT_HEAD` 只等待帧头 `@`，遇到其他字符直接丢弃。
- `FRAME_DONE` 后再判断目标 ID，可以支持单播和 `ALL` 广播两种模式。

## 7. WiFi 通信异常处理流程图

```mermaid
flowchart TD
    A["主循环定期检查连接状态"] --> B{"是否收到 CLOSED、ERROR<br/>或发送超时?"}
    B -- "否" --> C["保持当前连接<br/>继续接收命令"]
    B -- "是" --> D["停止发送业务数据<br/>清空接收缓冲区"]
    D --> E["尝试 AT+CIPSTART<br/>重新连接 TCP Server"]
    E --> F{"TCP 重连成功?"}
    F -- "是" --> G["发送上线提示<br/>恢复命令处理"]
    F -- "否" --> H["检查 WiFi 连接<br/>必要时重新 AT+CWJAP"]
    H --> I{"热点重连成功?"}
    I -- "是" --> E
    I -- "否" --> J["延时后重新执行 ESP8266 初始化流程"]
    J --> K["避免频繁重试造成串口刷屏"]
    K --> E
    G --> C
```

适用说明：
- WiFi 实验常见故障包括热点密码错误、手机 APP 未监听、IP 地址写错、ESP8266 供电不足和串口波特率不一致。
- 程序检测到 `CLOSED`、`ERROR` 或发送超时后，不应继续把 LED 命令写入断开的 TCP 链路，而应先恢复连接。
- 重连时加入短延时可以避免连续发送 AT 指令导致 ESP8266 响应混乱。

## 8. 硬件与数据流关系示意图

```mermaid
flowchart LR
    subgraph PHONE["手机"]
        HOTSPOT["手机热点<br/>2.4 GHz WiFi"]
        APP["网络调试 APP<br/>TCP Server:8000<br/>ASCII 收发"]
    end

    subgraph MCU["STM32F103 指南者开发板"]
        USART["USART<br/>AT 指令与数据收发"]
        PARSER["命令解析<br/>LED_xxx 或 @ID:CMD#"]
        RGBDRV["RGB LED 驱动函数"]
        LED["板载 RGB LED<br/>红/绿/蓝/混色/闪烁"]
    end

    subgraph WIFI["ESP8266 WiFi 模块"]
        AT["AT 指令接口"]
        TCP["TCP Client"]
    end

    HOTSPOT --- TCP
    APP <-->|"TCP 数据<br/>LED_RED 等命令"| TCP
    TCP <-->|"串口字节流"| AT
    AT <-->|"USART"| USART
    USART --> PARSER
    PARSER --> RGBDRV
    RGBDRV --> LED
```

说明：
- 手机热点负责提供局域网，网络调试 APP 负责监听 TCP 端口并发送 ASCII 命令。
- ESP8266 负责 WiFi 和 TCP 连接，STM32 通过 USART 发送 AT 指令并接收网络数据。
- STM32 不直接处理 WiFi 射频细节，只需要解析 ESP8266 串口返回的数据，并把命令映射为 RGB LED 控制。

## 9. 报告中推荐放置方式

- 主程序总体流程图：说明 STM32 初始化、连接 ESP8266、接收命令、执行 LED 控制的整体结构。
- ESP8266 AT 指令连接流程图：说明从 `AT` 检测到 `CIPSTART` 建立 TCP 连接的关键步骤。
- 网络接收与命令解析流程图：解释为什么要使用接收缓冲区和结束符判断。
- RGB LED 指令执行流程图：说明基本题中手机 APP 指令如何映射到板载 RGB 灯颜色。
- 提高题多开发板组网流程图和命令帧状态机：突出多板互控时的目标 ID、ACK 和广播控制设计。
- 硬件与数据流关系示意图：可作为实验线路图或系统框图使用。
