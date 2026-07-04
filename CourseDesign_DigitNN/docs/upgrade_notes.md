# 上位机与多字符升级说明

## 已实现的升级

- 新增 `host_app/realtime_digit_ui.py` 实时上位机界面。
- 新增 `host_app/web_dashboard_server.py` 网页上位机，推荐用于答辩演示。
- 新增 `host_app/web/` 前端资源，包含画布、像素矩阵、模型结果、串口、资源和烧录面板。
- 网页工作台支持中文/English 一键切换。
- 支持鼠标手写数字，并显示 28x28 像素化模型输入。
- 支持 PC 端量化 Perceptron、FNN、Tiny-CNN 实时推理。
- 支持显示 PC 端推理标签、置信度和备选类别。
- 支持串口接收 STM32 的 `RESULT` 帧并展示板端结果。
- 支持从 Keil 构建日志读取 Flash/SRAM 利用率。
- 支持一键保存采集样本到 `tf_card/ui_collected/`，并自动追加 `label.txt`。
- UI 支持选择 `0-9A-Z` 标签，便于采集后续字母数据。
- UI 新增 `Firmware Deploy` 面板，可调用 Keil 命令行执行模型导出、构建和烧录。
- 新增 `tools/keil_flash.py`，支持命令行构建、下载、重新导出模型后下载。
- 新增 `tools/train_alnum.py`，用于训练 PC 端 `0-9A-Z` 36 类 Tiny-CNN 原型。
- STM32 端识别完成后新增结构化串口输出：

```text
RESULT,model=P,label=<0-9>,confidence=<0-100>,time_us=0
RESULT,model=F,label=<0-9>,confidence=<0-100>,time_us=0
RESULT,model=C,label=<0-9>,confidence=<0-100>,time_us=0
```

## 当前类别状态

当前板端模型和固件仍只支持数字 `0-9`。

PC 端已经提供 `tools/train_alnum.py` 作为英文字母原型入口。运行示例：

```powershell
python tools\train_alnum.py --epochs 5 --batch-size 512 --augment
```

快速试跑可限制每类样本数：

```powershell
python tools\train_alnum.py --epochs 1 --batch-size 256 --max-train-per-class 200 --max-test-per-class 50
```

要把英文字母加入板端，需要同步改动：

1. 数据集：引入 EMNIST Letters、EMNIST Balanced 或自采集字母数据。
2. 类别表：从 `0-9` 扩展到 `0-9A-Z` 或 `0-9A-Za-z`。
3. 训练脚本：把 `RECOGNIZER_CLASS_COUNT`、模型输出层和标签映射同步扩展。
4. 量化导出：重新导出 FNN/CNN 的 C 数组和 `.npz`。
5. 板端显示：把数字标签改为查表显示字符。
6. 测试集：新增字母标准测试集和个人手写测试集。

建议优先做 `0-9A-Z` 共 36 类，而不是大小写共 62 类。大小写手写体容易混淆，且模型和测试集压力更大。

板端移植前建议先看 `models/alnum_metrics.json` 的 36 类准确率和易混淆类别。如果 PC 端准确率不足，先扩充 UI 自采集数据和数据增强，再考虑改固件类别数。

## 笔顺、倾斜和方向问题

本项目识别的是最终图像，不使用笔顺序列，所以“从上到下写”或“从下到上写”这类笔顺变化通常不会影响模型输入。

会影响识别的是图像形态：

- 倾斜：可通过 `Auto deskew`、数据增强和训练集扩充改善。
- 字体偏移：可通过裁剪、居中和缩放改善。
- 线条粗细变化：可通过 `Thicken stroke` 和数据增强改善。
- 旋转或倒写：可加入旋转增强或候选角度投票，但 `6/9`、`2/5` 等倒置后存在天然歧义。
- 镜像书写：需要专门加入镜像增强，否则不建议作为主要要求。

课程设计场景中，推荐方案是：预处理做居中/倾斜校正，训练时加入轻微旋转、平移、缩放、线宽增强。

## 量化升级方向

当前是导出 int8 权重、int32 偏置，并用整数 MAC 在单片机上推理。

可继续增强：

- 对不同层使用独立 scale，降低 FNN/CNN 的量化误差。
- 引入激活量化统计，固定中间层缩放，减少手工 shift。
- 评估 QAT 量化感知训练，让模型训练时适应 int8。
- 针对 Cortex-M3 优化卷积和全连接循环。
- 如果换到 Cortex-M4/M7，可考虑 CMSIS-NN。

## 中文 5000 字识别建议

STM32F103VE 不适合在板端完成固定 5000 汉字识别。

原因：

- Flash 只有 512 KB，当前数字工程已经使用约 97 KB ROM。
- 仅一个 `784 -> 128 -> 5000` 的简单 FNN，int8 权重也约 784*128 + 128*5000 = 740352 bytes，还没有算代码、偏置、字库和缓冲区。
- CNN 最后一层如果接 5000 类，参数通常达到数 MB 级。
- 5000 类分类计算量很大，Cortex-M3 推理时间和交互体验都不理想。

更合理的架构是：

- STM32F103VE 负责触摸采集、LCD 显示、简单预处理和串口/USB/SD 卡传输。
- 上位机负责中文识别、模型推理、数据管理和结果展示。
- 如果一定要板端识别，只建议做小类别任务，例如数字、字母、符号，或固定几十个命令字。
