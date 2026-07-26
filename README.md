# 微控制器与嵌入式系统课程仓库

本仓库用于整理《微控制器与嵌入式系统》课程的学习资料、实验工程与课程设计成果，内容覆盖 8086/微机原理、MCS-51/8051、STM32F103、Keil、Proteus、STM32CubeIDE，以及嵌入式神经网络和综合产品设计。

仓库既包含可以直接打开的工程和源代码，也保留了任务书、教材、实验报告、流程图、仿真截图和课程设计交付材料，适合用于课程复习、实验复现、报告整理与答辩演示。

> [!NOTE]
> 本仓库以 Windows 工具链为主，部分目录和文件名包含中文。首次使用前建议先阅读本页的“环境准备”和“注意事项”。

## 内容导航

- [仓库结构](#仓库结构)
- [8051/C51 实验](#8051c51-实验)
- [STM32F103 实验](#stm32f103-实验)
- [课程设计：手写数字与字母识别](#课程设计手写数字与字母识别)
- [综合设计：CookMirror 厨魔镜](#综合设计cookmirror-厨魔镜)
- [环境准备](#环境准备)
- [快速开始](#快速开始)
- [报告与资料](#报告与资料)
- [常见问题](#常见问题)
- [仓库维护约定](#仓库维护约定)

## 仓库结构

| 路径 | 主要内容 |
| --- | --- |
| [`EX1/`](EX1/) - [`EX6/`](EX6/) | 8051/C51 基础与综合实验，包含汇编/C 源码、Keil 工程、Proteus 仿真、结果截图、流程图和实验报告。 |
| [`EX11/`](EX11/)、[`EX112/`](EX112/)、[`EX113/`](EX113/)、[`EX123/`](EX123/)、[`EX13/`](EX13/)、[`EX14/`](EX14/) | STM32F103 系列实验，包括 RGB LED、按键、ILI9341 LCD、ADC、串口、报警器和简易示波器。 |
| [`CourseDesign_DigitNN/`](CourseDesign_DigitNN/) | 基于 STM32F103 的手写数字/字母神经网络识别系统，包含板端固件、模型训练、量化导出、网页上位机、测试集和报告。 |
| [`CookMirror/`](CookMirror/) | CookMirror 厨魔镜综合设计，包含产品报告、答辩稿、Proteus、Keil、STM32CubeIDE、PCB/EDA 指南和前端演示。 |
| [`1-书籍配套例程-F103VE指南者_20240202/`](1-书籍配套例程-F103VE指南者_20240202/) | 野火 STM32F103VE 指南者开发板配套例程，用于外设学习和工程移植参考。 |
| [`homework/`](homework/) | 课后作业的 Proteus 工程、原理图、题目图片和导出文件。 |
| [`单片机/`](单片机/)、[`微控制器/`](微控制器/)、[`嵌入式/`](嵌入式/) | 课程课件、教材、复习资料和参考文档。 |
| 根目录文档 | 实验任务书、实验指导书、课程设计任务书、培训材料、参考模板及已完成报告。 |

## 8051/C51 实验

`EX1` 至 `EX6` 以 SST89E554RC/兼容 8051 内核为主要目标平台，工程文件主要使用 Keil C51，部分综合题配有 Proteus 仿真。不同源码通常各自包含一个 `main()`，应分别加入独立 Target 或独立 Keil 工程，不要同时编译。

| 实验 | 主题 | 主要内容 | 详细说明 |
| --- | --- | --- | --- |
| [`EX1/`](EX1/) | 汇编与并行 I/O | 内部 RAM 操作、BCD/ASCII 转换、查表、平方运算、P1 数字量输入输出、流水灯和方向控制。 | 直接查看各 `.asm`、`DigitIO.c` 与对应 `.uvproj`。 |
| [`EX2/`](EX2/) | 中断系统 | 定时器中断、外部中断、交通灯状态机、紧急车辆全红优先和数码管倒计时。 | 查看 `EX2_Int1.c`、`EX2_INT2.c`、`EX2_Traffic_Emergency.c` 和 `cc.c`。 |
| [`EX3/`](EX3/) | 定时/计数与发声 | 定时器、外部计数、Timer2 时钟输出、电子发声，以及 LED 时序、数码管和双旋律综合题。 | [实验三操作说明](EX3/README.md) |
| [`EX4/`](EX4/) | 存储器与点阵 | 片内/外 RAM 数据传送与排序、8×8/16×16 点阵扫描、姓名或学号滚动显示。 | [实验四操作说明](EX4/README.md) |
| [`EX5/`](EX5/) | 串口与 A/D | 串口发送、ADC0809 采样、数码管显示、上位机精度设置和 ADC 数据回传。 | [实验五操作说明](EX5/README.md) |
| [`EX6/`](EX6/) | 电机控制 | 四相八拍步进电机、直流电机 PWM 调速、按键/串口控制、状态显示和越界报警。 | [实验六操作说明](EX6/README.md) |

各实验目录中常见文件的作用如下：

| 文件或目录 | 作用 |
| --- | --- |
| `*.c`、`*.asm` | 实验源代码。 |
| `*.uvproj`、`*.uvopt` | Keil C51 工程及配置。 |
| `*.pdsprj` | Proteus 仿真工程。 |
| `Objects/`、`Listings/` | Keil 编译产物、HEX、链接和列表文件。 |
| `Image/` | 电路、运行结果或仿真截图。 |
| `Paper/` | 报告、生成脚本和 Mermaid 算法流程图。 |
| `Project Backups/` | Proteus 自动备份，仅用于恢复历史工程。 |

## STM32F103 实验

STM32 实验主要基于野火 STM32F103VE 指南者开发板及 STM32 标准外设库。Keil 工程通常位于实验目录下的 `Project/`，源代码位于 `User/`，公共外设驱动位于 `Libraries/` 或 `User` 的各 BSP 子目录。

| 目录 | 实验内容 | 关键外设 |
| --- | --- | --- |
| [`EX11/`](EX11/) | RGB LED 纯色/混色循环，按键切换顺序和显示模式。 | GPIO、按键、RGB LED |
| [`EX112/`](EX112/) | ILI9341 模拟时钟与数字时间显示，按键调整时间。 | GPIO、ILI9341 LCD、按键 |
| [`EX113/`](EX113/) | 简易火警报警器，ADC 电压阈值判断并采用迟滞避免状态抖动。 | ADC、USART、报警 LED |
| [`EX123/`](EX123/) | 简易 LCD 示波器，实时显示 ADC 曲线，可切换量程和低通滤波。 | ADC、ILI9341 LCD、按键、USART |
| [`EX13/`](EX13/) | ADC 火警报警与 LCD 示波器综合实现，包含移动平均滤波、错误提示和串口输出。 | ADC、LCD、RGB LED、USART |
| [`EX14/`](EX14/) | STM32 实验四报告与算法流程图材料。 | 报告材料 |

打开工程时请优先选择目录内的 `.uvprojx` 文件，并检查：

1. Target 芯片型号与实际开发板一致。
2. `Options for Target > C/C++` 中的 Include Paths 完整。
3. `Debug` 和 `Utilities` 使用正确的 CMSIS-DAP/DAPLink。
4. 下载前选择 SWD 接口，并确认 Flash Download Algorithm 已配置。
5. 串口实验使用的 COM 端口、波特率和引脚与源码一致。

## 课程设计：手写数字与字母识别

[`CourseDesign_DigitNN/`](CourseDesign_DigitNN/) 是本仓库当前最完整的课程设计工程。系统以 STM32F103VE、ILI9341 LCD 和 XPT2046 电阻触摸屏为板端输入设备，将手写轨迹预处理为 `28 × 28` 灰度图，再运行量化神经网络推理。

### 已实现功能

- 数字识别：Perceptron、FNN、Tiny-CNN/DS-CNN 三模型对比。
- 字母识别：EMNIST Letters A-Z，使用独立的 Letter-Perceptron、Letter-FNN 和 Letter-DS-CNN 权重。
- 嵌入式推理：`uint8_t` 输入、`int8_t` 权重和 `int32_t` 累加器，支持权重导出到 Keil 工程。
- 网页上位机：手写输入、板端轨迹、像素预览、置信度、串口监控、样本采集和中英文界面。
- 自动化评估：批量测试 MNIST、personal、USPS 与 EMNIST Letters 数据。
- 工程部署：从网页或命令行执行模型导出、Keil 构建和固件烧录。
- 扩展能力：中文识别 API 原型、TF 卡测试集镜像和字母模型交付包。

### 子目录说明

| 路径 | 作用 |
| --- | --- |
| [`docs/`](CourseDesign_DigitNN/docs/) | 需求、系统设计、算法流程、串口协议、测试结果和升级记录。 |
| [`firmware/`](CourseDesign_DigitNN/firmware/) | 可移植的 STM32 C 推理核心、驱动接口和生成权重。 |
| [`host_app/`](CourseDesign_DigitNN/host_app/) | 网页 Dashboard、串口监控和早期 Tkinter 工具。 |
| [`keil_touch_digit_nn/`](CourseDesign_DigitNN/keil_touch_digit_nn/) | 基于野火触摸画板例程改造的实物 Keil 工程。 |
| [`models/`](CourseDesign_DigitNN/models/) | 训练权重、量化参数和评估指标。 |
| [`packages/`](CourseDesign_DigitNN/packages/) | 可直接交付的模型与固件资源包。 |
| [`tf_card/`](CourseDesign_DigitNN/tf_card/) | 可复制到 TF 卡根目录的测试集镜像。 |
| [`tools/`](CourseDesign_DigitNN/tools/) | 训练、导出、测试集制作、评估和烧录脚本。 |

完整说明见 [CourseDesign_DigitNN README](CourseDesign_DigitNN/README.md)，串口和网页操作见 [Host App README](CourseDesign_DigitNN/host_app/README.md)。

### 常用命令

先安装 Python 依赖：

```powershell
cd CourseDesign_DigitNN
python -m pip install -r tools\requirements.txt
```

启动网页上位机：

```powershell
python host_app\web_dashboard_server.py
```

然后在浏览器访问 `http://127.0.0.1:8765/`。

训练、导出和评估：

```powershell
python tools\train_mnist.py --model fnn --epochs 8 --batch-size 512 --augment --export-c --export-keil
python tools\train_mnist.py --model cnn --epochs 5 --batch-size 512 --augment --export-c --export-keil
python tools\train_letters.py --model all --epochs 8 --batch-size 128 --augment --export-c --export-keil
python tools\evaluate_tf_card.py
python tools\build_tf_manifest.py
```

命令行构建 Keil 工程：

```powershell
python tools\keil_flash.py --action build
```

> [!WARNING]
> 数字和字母使用不同的类别域与整套权重。切换识别域时应同时替换 `RecognitionDomain.h` 及三套模型参数，不能只混合替换单个模型文件。

## 综合设计：CookMirror 厨魔镜

[`CookMirror/`](CookMirror/) 是面向厨房场景的综合产品设计，材料覆盖从需求与产品方案到硬件仿真、嵌入式工程、前端展示和答辩交付的完整链路。

| 入口 | 内容 |
| --- | --- |
| [产品设计报告](CookMirror/产品设计报告.md) | 项目背景、需求、方案、功能与设计说明。 |
| [答辩演讲词](CookMirror/答辩演讲词.md) | 答辩展示的讲解稿。 |
| [Proteus 搭建手册](CookMirror/Proteus搭建手册.md) | 仿真电路搭建和运行说明。 |
| [`Proteus/`](CookMirror/Proteus/) | Proteus 仿真资源。 |
| [`Keil/`](CookMirror/Keil/) | Keil 工程与相关代码。 |
| [`STM32CubeIDE/`](CookMirror/STM32CubeIDE/) | STM32CubeIDE 工程。 |
| [`app/`](CookMirror/app/) | 前端或应用演示材料。 |
| [立创 EDA/PCB 制作指南](CookMirror/立创EDA_嘉立创PCB制作指南.md) | 原理图、PCB 与打样流程参考。 |

## 环境准备

### 必需或推荐软件

| 工具 | 用途 |
| --- | --- |
| Git | 克隆仓库和版本管理。 |
| Keil uVision / MDK | 打开 8051 的 `.uvproj` 与 STM32 的 `.uvprojx` 工程，编译和下载固件。 |
| Proteus 8 | 打开 `.pdsprj` 仿真工程，验证 8051 和综合电路。 |
| STM32CubeIDE | 打开 CookMirror 中的 Cube 工程。 |
| Python 3.10+ | 模型训练、权重导出、测试集制作、网页上位机和 PC 端评估。 |
| 支持 Mermaid 的 Markdown 查看器 | 查看 `Paper/` 和 `docs/` 中的算法流程图。 |

### 推荐硬件

- 野火 STM32F103VE 指南者开发板。
- CMSIS-DAP/DAPLink 或其他兼容 SWD 下载器。
- USB-TTL/板载 USB 转串口模块。
- ILI9341 LCD 与 XPT2046 电阻触摸屏。
- 8051 实验箱，或使用 Proteus 完成等效仿真。

## 快速开始

### 1. 克隆仓库

仓库包含 PDF、DOCX、仿真工程、模型和测试集，体积较大，克隆可能需要一些时间：

```powershell
git clone https://github.com/Travor278/Microcontrollers-and-Embedded-Systems.git
cd Microcontrollers-and-Embedded-Systems
```

### 2. 运行一个 8051 实验

1. 进入 `EX1` - `EX6` 中的目标实验目录。
2. 优先阅读该目录的 `README.md`，确认任务、端口和晶振约定。
3. 使用 Keil C51 打开对应 `.uvproj`，只保留当前实验的入口源码。
4. 在 `Options for Target > Output` 勾选 `Create HEX File`。
5. 编译生成 HEX，并在 Proteus 的单片机属性中加载该文件。
6. 按 README 的调试要点观察端口、波形、串口或显示结果。

### 3. 运行一个 STM32 实验

1. 打开对应实验目录下的 `.uvprojx`。
2. 检查芯片型号、头文件路径、编译器版本和下载器设置。
3. 连接开发板与 DAPLink，编译并下载。
4. 如需串口输出，再连接 USB 转串口，并按源码设置串口参数。
5. 对 ADC 实验先确认输入电压不超过 `3.3 V`，再观察 LCD、LED 或串口结果。

### 4. 整理实验报告

1. 阅读根目录任务书和实验指导书。
2. 参考实验目录的 `Paper/` 流程图及报告文件。
3. 保存 Keil 编译结果、Proteus 电路、关键波形、串口输出和实物运行照片。
4. 在报告中说明芯片、晶振、端口分配、算法流程、关键代码和验证结果。
5. 提交前核对基本题、提高题与任务书要求是否一一对应。

## 报告与资料

根目录包含以下类型的课程资料：

- 《单片机实验指导》及年度实验任务书。
- 微控制器与嵌入式系统课程设计任务书和培训材料。
- 实验报告、课程设计报告参考格式。
- 8086/微机原理笔记、课件、教材和复习资料。
- 已完成的课程设计报告及 PDF 交付稿。

实验说明与任务书编号偶尔存在排版差异，`EX3` - `EX6` 的 README 已结合指导书内容标注对应章节。实际提交时请以任课教师最新发布的任务书为准。

## 常见问题

### Keil 打开工程后缺少头文件

先确认使用了匹配的 Keil 产品：8051 工程需要 C51，STM32 工程需要 MDK-ARM。然后检查 `Options for Target > C/C++ > Include Paths`，以及工程中的库文件是否仍位于原目录。

### Proteus 仿真没有运行或单片机无输出

确认已在 Keil 中生成 HEX、Proteus 中加载了最新 HEX，并让 Proteus 的单片机时钟与源码计算使用的晶振频率一致。还应检查 LED、数码管的共阳/共阴类型和有效电平。

### DAPLink 能下载但网页看不到串口

DAPLink/SWD 主要负责下载和调试，不一定提供程序使用的 UART 数据通道。请另外连接板载 USB 转串口或 USB-TTL，并核对 USART 引脚、COM 端口和波特率。手写识别工程默认使用 USART1 `115200 8N1`，对应 `PA9/PA10`。

### 中文路径导致工具报错

Windows 下的旧版 Keil、Proteus 或部分脚本可能无法稳定处理中文或过长路径。遇到问题时，可将目标工程复制到类似 `D:\mcu_lab\` 的短英文路径后再编译；不要随意改变源码中的相对目录结构。

### Python 训练数据从哪里来

`CourseDesign_DigitNN/data/` 用作可重新下载或生成的数据缓存，默认不会提交到 Git。运行训练或测试集制作脚本后，所需公开数据会按脚本逻辑准备到本地。

### Keil 提示代码大小超出限制

课程设计中的多模型固件可能超过 Keil Lite 的链接大小限制。应使用具备相应容量授权的 MDK 环境，或者选择更小的单模型配置；这不是源代码编译错误。

## 仓库维护约定

- 源码、工程配置、说明文档、关键截图和可复现实验结果应纳入版本管理。
- `Output/`、`Listings/`、`.uvgui.*`、临时日志、Python 缓存和本地数据缓存通常不需要提交。
- `.env`、API Key、串口设备信息等本机配置或密钥不得提交。
- 修改实验工程后，应同步更新对应 README 中的端口、晶振、操作步骤和验证结果。
- 新增大文件前先确认其是否属于不可再生成的课程交付物，避免重复提交构建产物。
- Proteus 的 `Project Backups/` 仅用于必要的历史恢复，日常修改应以主 `.pdsprj` 为准。

## 版权与使用说明

本仓库主要用于课程学习、实验复现和个人项目归档。教材、课件、开发板配套例程、第三方数据集及软件工具的版权归原作者或发布方所有。使用或分发相关内容时，请遵守课程要求、软件许可和原始资料的版权声明。
