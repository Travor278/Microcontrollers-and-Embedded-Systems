# 微控制器与嵌入式系统课程仓库

本仓库整理《微控制器与嵌入式系统》课程相关资料，覆盖 8051/C51 实验、STM32F103 实验、Proteus 仿真、Keil 工程、STM32CubeIDE 工程、课程设计和课程报告材料。

## 目录总览

| 路径 | 内容 |
| --- | --- |
| `EX1/` - `EX6/` | 8051/C51 基础到综合实验，包含汇编/C 代码、Keil 工程、Proteus 仿真、流程图和报告材料。 |
| `EX11/`、`EX112/`、`EX113/`、`EX123/`、`EX13/`、`EX14/` | STM32F103 系列实验，包含 LED、按键、ILI9341 LCD、ADC、报警、简易示波器等内容。 |
| `CourseDesign_DigitNN/` | 课程设计题目 6：基于神经网络的手写数字/字母识别系统，包含 Keil 实物工程、网页上位机、量化模型、TF 卡测试集、中文识别 API 原型和报告材料。 |
| `CookMirror/` | 课程综合设计 CookMirror 厨魔镜，包含产品设计报告、答辩稿、Proteus 仿真、STM32CubeIDE 工程、EDA/PCB 文档和前端演示材料。 |
| `1-书籍配套例程-F103VE指南者_20240202/` | 野火 STM32F103VE 指南者开发板配套例程，作为 STM32 实验和课程设计移植参考。 |
| `homework/` | 课后作业相关 Proteus 工程、题目图片和导出文件。 |
| `单片机/`、`微控制器/`、`嵌入式/` | 课程课件、教材、复习资料等 PDF 文档。 |
| 根目录 PDF/DOC/DOCX | 实验任务书、实验指导书、课程设计任务书、培训材料和报告模板。 |

## 快速入口

| 需求 | 推荐入口 |
| --- | --- |
| 8051/C51 实验 | `EX1/` - `EX6/` 中对应实验目录 |
| STM32F103 实验 | `EX11/Project/BH-F103.uvprojx`、`EX112/Project/RVMDK（uv4）/BH-F103.uvprojx` 等 |
| 手写识别课程设计说明 | `CourseDesign_DigitNN/README.md` |
| 手写识别 Keil 工程 | `CourseDesign_DigitNN/keil_touch_digit_nn/Project/RVMDK（uv5）/BH-F103.uvprojx` |
| 网页上位机 | `CourseDesign_DigitNN/host_app/web_dashboard_server.py`，浏览器打开 `http://127.0.0.1:8765/` |
| TF 卡测试集镜像 | `CourseDesign_DigitNN/tf_card/` |
| 字母模型交付包 | `CourseDesign_DigitNN/packages/letter_models_20260706.zip` |
| CookMirror 设计报告 | `CookMirror/产品设计报告.md` |
| CookMirror 答辩稿 | `CookMirror/答辩演讲词.md` |
| 野火 F103 配套例程 | `1-书籍配套例程-F103VE指南者_20240202/` |

## 课程设计进展

`CourseDesign_DigitNN/` 是当前推进最完整的课程设计工程：

- 板端工程：基于野火电阻触摸屏画板例程改造，使用 Keil uVision 打开和下载。
- 数字识别：Perceptron、FNN、Tiny-CNN/DS-CNN 量化模型，支持 STM32 端 P/F/C 三模型对比输出。
- 字母识别：EMNIST Letters A-Z，独立字母固件域，主模型为 Letter-Perceptron、Letter-FNN、Letter-DS-CNN，不与数字权重混烧。
- 上位机：网页工作台支持首页可视化讲解、数字/字母工作区、自动化测试、中文识别、TF 卡页、串口回传、样本采集和 Keil 构建/烧录按钮。
- 模型包：`packages/letter_models_20260706/` 与 `.zip` 已整理字母模型、量化权重、指标和字母 C 权重缓存。
- 测试集：`tf_card/` 可直接复制到 TF 卡根目录，包含 MNIST 1000 张、personal 129 张、USPS 100 张、EMNIST Letters 260 张。

常用命令：

```powershell
python CourseDesign_DigitNN\host_app\web_dashboard_server.py
python CourseDesign_DigitNN\tools\train_mnist.py --model fnn --epochs 8 --batch-size 512 --augment --export-c --export-keil
python CourseDesign_DigitNN\tools\train_mnist.py --model cnn --epochs 5 --batch-size 512 --augment --export-c --export-keil
python CourseDesign_DigitNN\tools\train_letters.py --model all --epochs 8 --batch-size 128 --augment --export-c --export-keil
python CourseDesign_DigitNN\tools\evaluate_tf_card.py
python CourseDesign_DigitNN\tools\build_tf_manifest.py
python CourseDesign_DigitNN\tools\keil_flash.py --action build
```

## 推荐环境

- Windows 10/11
- Keil uVision / MDK，用于打开 `.uvproj`、`.uvprojx` 工程
- Proteus 8，用于打开 `.pdsprj` 仿真工程
- STM32CubeIDE，用于 CookMirror 相关 Cube 工程
- Python 3.10+，用于模型训练、参数导出、BMP 测试集生成和 PC 端评估
- DAPLink/CMSIS-DAP、USB-TTL 串口工具，用于下载和调试实物板

## 实验流程建议

1. 先阅读根目录任务书和对应实验目录 README。
2. 在 Keil 中打开对应工程，确认芯片型号、晶振频率和输出 HEX 设置。
3. 使用 Proteus 或实物板验证功能，保存关键截图和串口输出。
4. 课程设计建议先跑通数字 P/F/C，再切换字母域，最后展示自动化测试和中文识别扩展。
5. 最后整理报告、答辩稿、测试记录和演示材料。

## 注意事项

- 仓库包含较多 PDF、DOCX、Proteus 备份、开发板配套例程和测试数据，体积较大属于正常情况。
- Keil `Output/`、`Listing/`、用户界面文件和上位机 raw 临时图已加入 `.gitignore`，不应作为源码长期维护。
- 部分历史资料和工具工程包含中文路径，建议在 Windows 环境下使用；如果工具链对中文路径不稳定，可复制到纯英文路径后再编译。
- `CourseDesign_DigitNN/data/` 用于缓存训练数据和在线数据源，不建议提交。
- 仓库中的教材、课件、开发板例程和第三方数据仅用于课程学习与实验参考，版权归原作者或发布方所有。
