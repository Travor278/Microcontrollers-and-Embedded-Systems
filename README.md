# 微控制器与嵌入式系统课程仓库

本仓库整理了《微控制器与嵌入式系统》课程相关的实验代码、Proteus 仿真、Keil/STM32 工程、课程设计资料与课程参考文档。内容覆盖 8051/C51、8086/微机原理、STM32F103、Proteus 电路仿真、上位机脚本以及课程设计报告材料。

## 仓库内容

| 路径 | 内容 |
| --- | --- |
| `EX1/` | 实验一：8051 汇编/C51 基础、I/O 控制、LED 与数码管相关程序，含流程图和实验报告。 |
| `EX2/` | 实验二：外部中断、交通灯/紧急控制等 8051 实验，含 Proteus 仿真工程。 |
| `EX3/` | 实验三：定时/计数器、电子发声，以及 LED/数码管/音乐综合提高题。 |
| `EX4/` | 实验四：静态存储器扩展、片内/片外 RAM 排序、8x8/16x16 点阵显示。 |
| `EX5/` | 实验五：串口通讯、ADC0809 A/D 转换、串口命令控制显示精度等综合题。 |
| `EX6/` | 实验六：步进电机、直流电机 PWM 调速、按键/串口综合控制。 |
| `EX11/`、`EX112/`、`EX113/`、`EX123/`、`EX13/` | STM32F103 系列实验，包含 LED/按键、ILI9341 LCD、ADC 采样、报警与简易示波器等内容。 |
| `CourseDesign_DigitNN/` | 课程设计：基于神经网络的手写数字识别系统，包含 STM32 端代码、Keil 工程、训练脚本、测试集、报告草稿和串口协议文档。 |
| `CookMirror/` | 课程大作业/产品设计：CookMirror 厨魔镜，包含产品设计报告、前端演示、Proteus 仿真、STM32CubeIDE 工程、PCB/EDA 工作文档等。 |
| `1-书籍配套例程-F103VE指南者_20240202/` | 野火 STM32F103VE 指南者开发板配套例程，用作 STM32 实验和课程设计的参考工程。 |
| `单片机/`、`微控制器/`、`嵌入式/` | 课程课件、教材、复习资料等 PDF 文档。 |
| `homework/` | 课后作业相关 Proteus 工程、题目图片与导出文件。 |
| 根目录 PDF/DOC/DOCX | 实验任务书、实验指导书、课程设计任务书、报告模板和参考资料。 |

部分实验目录下已经有更细的说明文件，例如 `EX3/README.md`、`EX4/README.md`、`EX5/README.md`、`EX6/README.md` 和 `CourseDesign_DigitNN/README.md`。做某个实验时建议先读对应目录的 README。

## 推荐环境

- Windows 10/11
- Keil uVision / MDK，用于打开 `.uvproj`、`.uvprojx` 工程
- Proteus 8，用于打开 `.pdsprj` 仿真工程
- STM32CubeIDE，用于 `CookMirror/STM32CubeIDE/` 下的 Cube 工程
- Python 3.10+，用于 `CourseDesign_DigitNN/tools/` 中的训练、导出和批量测试脚本
- DAPLink/CMSIS-DAP、USB-TTL 串口工具，按实验需要连接实物板

## 快速开始

### 1. 8051/C51 实验

进入 `EX1` 到 `EX6` 中对应实验目录，使用 Keil 打开 `.uvproj` 工程。若一个目录中有多个 `.c` 或 `.asm` 文件，建议每次只让一个带 `main()` 的源文件参与编译，避免重复入口导致编译错误。

编译前在 Keil 中确认：

- 器件型号与实验要求一致，例如 8051/SST89E554RC 或兼容内核器件。
- `Options for Target -> Output` 勾选 `Create HEX File`。
- 晶振频率与源码注释或实验说明一致，很多 C51 实验按 `12 MHz` 计算延时和定时器初值。

需要仿真时，打开同目录或子目录中的 `.pdsprj`，把 Keil 生成的 `.hex` 加载到 Proteus 单片机元件中运行。

### 2. STM32F103 实验

STM32 实验多采用野火 F103 标准库工程结构。常见入口为：

```text
EX11/Project/BH-F103.uvprojx
EX112/Project/RVMDK（uv4）/BH-F103.uvprojx
EX113/Project/RVMDK（uv5）/BH-F103.uvprojx
EX123/Project/RVMDK（uv5）/BH-F103.uvprojx
EX13/Project/RVMDK（uv4）/BH-F103.uvprojx
```

下载到开发板前，在 Keil 中检查：

- Debug 适配器选择 `CMSIS-DAP Debugger` 或实际使用的下载器。
- SWD 接线正确：`SWDIO -> PA13`、`SWCLK -> PA14`、`GND -> GND`、`3.3V/VTref -> 3.3V`。
- LCD、按键、ADC 输入等外设接线与 `User/main.c` 中的宏定义一致。

### 3. 手写数字识别课程设计

进入 `CourseDesign_DigitNN/`：

```powershell
cd CourseDesign_DigitNN
python -m venv .venv
.\.venv\Scripts\activate
pip install -r tools\requirements.txt
```

训练并导出基础模型示例：

```powershell
python tools\train_mnist.py --model perceptron --epochs 2 --batch-size 512 --export-c --export-keil
```

实物演示工程入口：

```text
CourseDesign_DigitNN/keil_touch_digit_nn/Project/RVMDK（uv5）/BH-F103.uvprojx
```

更多细节见 `CourseDesign_DigitNN/README.md`、`docs/system_design.md`、`docs/serial_protocol.md` 和 `docs/test_results.md`。

### 4. CookMirror 厨魔镜

`CookMirror/` 是课程大作业性质的综合设计目录，包含：

- `产品设计报告.md`：产品定位、市场分析、功能设计、硬件方案和验证清单。
- `app/index.html`：前端展示入口。
- `Proteus/`：核心子电路仿真工程。
- `STM32CubeIDE/`：多个 STM32CubeIDE 子工程。
- `立创EDA_嘉立创PCB制作指南.md`、`立创EDA绘图工作清单.md`：PCB 与 EDA 工作资料。

## 常见文件位置

| 需求 | 位置 |
| --- | --- |
| 实验任务书 | 根目录 `2026微控制器与嵌入式系统实验任务书（适用于自动化、测控2024级）.docx` |
| 单片机实验指导 | 根目录 `单片机实验指导.pdf` |
| 课程设计任务 | 根目录 `微控制器与嵌入式系统-课程设计任务书.pdf` |
| 实验报告模板 | 根目录 `微控制器与嵌入式系统实验报告（参考格式）.docx` |
| 实验流程图草稿 | 各实验目录下的 `Paper/实验*算法流程图_mermaid.md` |
| Keil 编译输出 | 通常在各工程的 `Objects/`、`Listings/` 或 `Project/Objects/` 下 |
| Proteus 自动备份 | 各工程的 `Project Backups/` 下 |

## 注意事项

- 仓库中包含大量 PDF、DOCX、图片、仿真备份和编译产物，体积较大属于正常情况。
- 部分历史文本文件可能是 GBK/ANSI 编码，若出现乱码，可在编辑器中切换为 GBK 或使用支持中文编码的 IDE 打开。
- Keil/Proteus 工程路径包含中文目录名，建议在 Windows 环境下使用；若工具链对中文路径不稳定，可把对应实验目录复制到纯英文路径后再编译。
- `.gitignore` 已忽略 `*.mp4`、`__pycache__/`、`CourseDesign_DigitNN/cube_workspace/` 和 `CourseDesign_DigitNN/data/`，数据集与临时工程不建议直接提交。
- 本仓库中的教材、课件、开发板例程和第三方资料仅用于课程学习与实验参考，版权归原作者或发布方所有。

## 建议的实验流程

1. 先阅读根目录任务书和对应实验目录的 README。
2. 在 Keil 中单独编译基础题源文件，生成 `.hex`。
3. 用 Proteus 或实物板验证现象，保存关键截图。
4. 完成提高题或综合题，补充流程图、端口说明和调试记录。
5. 按报告模板整理实验目的、原理、源码、现象截图、问题分析和总结。

