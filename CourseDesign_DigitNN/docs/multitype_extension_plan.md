# 多类型字符扩展方案

## 目标

在当前 10 类数字识别闭环稳定后，扩展到更多字符类型，例如：

- 数字：`0-9`
- 大写字母：`A-Z`
- 常用符号：`+ - x / =`

该扩展不建议直接替换当前答辩主线。主线仍保持“STM32 触摸屏手写数字识别”，多类型字符作为进阶设计方案和后续实现路径。

## 数据集路线

1. EMNIST Digits/Letters/Balanced：用于训练数字和字母模型。
2. 自制符号集：用触摸屏或纸笔照片采集 `+ - x / =`，每类至少 20-50 张。
3. 标签格式扩展：从当前 `filename,label` 扩展为 `filename,class_id,class_name`，例如 `sym_0001_plus.bmp,36,+`。

## 模型路线

| 路线 | 类别数 | 部署位置 | 说明 |
| --- | ---: | --- | --- |
| Digits 10 类 | 10 | STM32 | 当前已完成，Perceptron/FNN/Tiny-CNN 均可运行 |
| Digits + Letters | 36 | PC/Linux 或高配 MCU | 推荐先用 CNN 训练验证，再考虑量化 |
| Digits + Letters + Symbols | 41+ | PC/Linux 协同 | STM32 采集和预处理，串口/USB 发送 28x28 图像给上位机识别 |

## STM32 侧改造点

1. `RECOGNIZER_CLASS_COUNT` 从 10 改为目标类别数。
2. `recognizer_result_t.label` 保留数值 ID，LCD 显示时查表转换为字符。
3. 训练脚本导出的 `*_CLASS_COUNT` 与 C 端数组维度保持一致。
4. TF 卡 `label.txt` 增加 `class_name` 字段，用于报告和批量测试输出。

## 推荐答辩表述

当前系统已经完成 10 类数字识别的板端闭环。多类型字符扩展采用同一套触摸采集、28x28 归一化和量化部署流程；区别在于训练数据从 MNIST 扩展为 EMNIST/自制符号集，分类头从 10 类扩展到 36 类或 41 类以上。若 STM32F103 资源不足，可使用 STM32 负责采集和预处理、Linux/PC 负责 CNN 推理的边缘协同方案。
