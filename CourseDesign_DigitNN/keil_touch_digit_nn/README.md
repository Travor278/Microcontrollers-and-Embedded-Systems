# Keil Touch DigitNN Project

本工程基于野火示例改造：

```text
1-书籍配套例程-F103VE指南者_20240202/29-电阻触摸屏—触摸画板
```

目标是在 STM32F103VE 指南者开发板上完成触摸屏手写采集、28 x 28 预处理、量化神经网络推理、LCD 显示和串口回传。

## 打开工程

Keil uVision 打开：

```text
Project/RVMDK（uv5）/BH-F103.uvprojx
```

Target：

```text
DigitNN_Touch
```

## 下载配置

在 Keil 中：

1. `Options for Target`
2. `Debug`
3. 选择 `CMSIS-DAP Debugger`
4. 点击 `Settings`
5. `Port` 选择 `SW`
6. `Utilities` 页也选择 `CMSIS-DAP Debugger`
7. 确认 Flash Download Algorithm 正确后 Build / Download

DAPLink SWD 连接：

- `SWDIO` -> `PA13`
- `SWCLK` -> `PA14`
- `GND` -> `GND`
- `VTref/3V3` -> 开发板 `3.3V`
- `RST` -> `NRST` 可选

## 串口

- 默认 USART1，`115200 8N1`。
- 板载 USB 转串口或外接 CH340 需要连接到固件使用的 USART 引脚。
- SWD 只负责下载调试，不等于串口回传。
- 网页上位机中选择类似 `COM8 - USB-SERIAL CH340` 的端口。

常见串口帧：

```text
STATUS,state=idle,message=ready,proto=touch_stream_v1
POINT,x=120,y=88,pressure=1
STROKE,end=1
IMAGE,w=28,h=28,data=<1568 hex>
RESULT,model=P,label=2,label_index=2,confidence=30,time_us=...
RESULT,model=F,label=2,label_index=2,confidence=44,time_us=...
RESULT,model=C,label=2,label_index=2,confidence=45,time_us=...
```

## 使用流程

1. 开发板上电。
2. 如果提示校准，先完成触摸屏校准。
3. 在白色区域写数字或字母。
4. 点击右下角 `REC`。
5. LCD 顶部显示 P/F/C 三个模型的标签和置信度。
6. 串口输出 `IMAGE` 与 `RESULT`，网页工作区同步显示板端实时轨迹和模型输入。
7. 点击清屏按钮可清除 LCD 和识别缓存。

## 数字和字母固件

本工程共用同一套触摸屏、LCD、串口和推理框架，模型权重按识别域切换：

- 数字域：`0-9`，P/F/C 为 Perceptron、FNN、Tiny-CNN 或 DS-CNN。
- 字母域：`A-Z`，P/F/C 为 Letter-Perceptron、Letter-FNN、Letter-DS-CNN。

当前激活权重位于：

```text
User/digit_nn/generated/
```

切换数字/字母时必须整体替换 `RecognitionDomain.h`、`PerceptronData.*`、`FNN_Data.*`、`CNN_Data.*`，不要只替换单个文件。

字母模型交付包见：

```text
../packages/letter_models_20260706/
```

## 文件分层

```text
User/
├── digit_nn/core/       # 推理、预处理和识别器核心
├── digit_nn/generated/  # 当前激活的模型 C 权重
├── lcd/                 # ILI9341 与触摸屏驱动
├── usart/               # 串口驱动
├── FATFS/               # TF 卡/FatFs 相关移植文件
├── sdio/                # SDIO/TF 卡底层驱动
├── main.c
└── stm32f10x_it.c
```

## Git 说明

`Output/`、`Listing/`、`.uvguix.*` 是本机编译产物或用户界面状态，已加入 `.gitignore`。若本地需要重新生成，直接在 Keil 中 Build 即可。
