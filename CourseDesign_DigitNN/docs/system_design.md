# 系统设计

## 总体架构

```text
触摸屏/XPT2046
    ↓ 坐标采集
轨迹缓冲区
    ↓ 滤波、包围盒、归一化
28x28 灰度图
    ↓
模型推理层
    ├── Perceptron 基本任务
    └── FNN 进阶任务
    ↓
LCD 显示 + 串口上报 + 自动化测试统计
```

## STM32 端模块

- `image_preprocess`：把触摸轨迹转换为 28x28 灰度图，包含去抖、线段栅格化、边界归一化和简易加粗。
- `perceptron`：执行单层感知机推理，读取 `PerceptronData.c/.h` 中的 Flash 常量。
- `fnn`：执行一层隐藏层 FNN 推理，读取 `FNN_Data.c/.h` 中的 Flash 常量。
- `recognizer`：统一模型选择，便于 LCD 和自动化测试复用。
- `lcd_view`：显示标题、绘图区、轨迹和识别结果。
- `touch_panel`：适配野火 XPT2046 触摸屏驱动。
- `serial_protocol`：输出识别结果、测试统计和状态信息。
- `sd_testset`：读取 TF 卡 BMP 测试集并批量测试。

## PC 工具链

- `train_mnist.py`：训练 Perceptron/FNN/CNN，导出权重和量化 C 数组。
- `make_testset.py`：从 MNIST 测试集导出 BMP 图片和 `label.txt`。
- `host_batch_test.py`：在 PC 上验证导出模型对 BMP 测试集的准确率，便于和 STM32 结果比对。
- `serial_dashboard.py`：串口监控识别结果和批量测试统计。

## 模型路线

1. Perceptron：784 输入，10 输出，权重规模小，适合基本任务快速部署。
2. FNN：784 输入，64 隐藏层，10 输出，明显提高准确率，仍可放入 STM32 Flash。
3. CNN：用于 PC/Linux 训练和边缘协同展示；STM32F103 端直接部署 CNN 难度较高，建议作为跨平台进阶。

## 自行设计的加分方案

- 串口协议统一输出 `RESULT`、`TEST`、`STATUS` 帧，上位机可实时记录实验数据。
- 同一份 28x28 预处理结果可同时给 Perceptron 和 FNN 推理，LCD 上显示双模型对比。
- TF 卡批量测试统计平均推理时间，形成报告中的量化对比表。
- 个人手写测试集与 MNIST 标准测试集分开统计，体现泛化能力分析。
