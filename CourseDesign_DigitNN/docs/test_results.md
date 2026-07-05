# 当前联调结果

## 训练结果

训练命令：

```powershell
python CourseDesign_DigitNN\tools\train_mnist.py --model perceptron --epochs 2 --batch-size 512 --data-dir CourseDesign_DigitNN\data --export-c --export-keil
python CourseDesign_DigitNN\tools\train_mnist.py --model fnn --epochs 8 --batch-size 512 --data-dir CourseDesign_DigitNN\data --augment --export-c --export-keil
python CourseDesign_DigitNN\tools\train_mnist.py --model cnn --epochs 5 --batch-size 512 --data-dir CourseDesign_DigitNN\data --augment --export-c --export-keil
```

官方 MNIST 测试集结果：

| 模型 | 训练设置 | MNIST 测试准确率 | 说明 |
| --- | --- | ---: | --- |
| Perceptron | 2 epoch | 89.25% | 基础任务模型，作为低资源对照 |
| FNN | 8 epoch + affine augmentation | 94.48% | 进阶全连接模型，速度和准确率比较均衡 |
| Tiny-CNN | 5 epoch + affine augmentation | 95.97% | 卷积进阶模型，能提取局部结构特征，但计算量更高 |

## TF 卡测试集

TF 卡目录镜像：

```text
CourseDesign_DigitNN/tf_card/
├── manifest.csv
├── mnist/          # MNIST 标准数字测试集，1000 张 BMP
├── personal/       # 上位机采集个人数字测试集，129 张 BMP
├── external_usps/  # USPS 公开手写数字测试集，100 张 BMP
├── emnist_letters/ # EMNIST Letters 字母测试集，260 张 BMP
└── ui_collected/   # 上位机采集缓存，已同步到 personal/，默认不进入 manifest
```

个人手写集来自上位机 `tf_card/ui_collected/` 新采集样本，当前 `tf_card/personal` 使用其中 129 张 28x28 预处理 BMP，并保留 `label.txt` 作为真实标签记录。

外部公开集来自 USPS handwritten digits。该数据集为 16x16 灰度手写数字，本项目通过 `sklearn.datasets.fetch_openml("usps", version=2)` 拉取，每类选取 10 张并转换为 28x28 BMP，用于验证模型在非 MNIST 来源上的泛化能力。

字母准备集来自 EMNIST Letters。当前先导出每类 10 张、共 260 张 BMP，作为后续字母识别工作区和自动化测试页面的数据基础。

汇总命令：

```powershell
python CourseDesign_DigitNN\tools\evaluate_tf_card.py
```

PC 端量化模型结果：

| 测试集 | 模型 | 图片数量 | 正确数量 | 准确率 | PC 平均推理时间 |
| --- | --- | ---: | ---: | ---: | ---: |
| mnist | Perceptron | 1000 | 874 | 87.40% | 688.55 us |
| mnist | FNN | 1000 | 929 | 92.90% | 396.29 us |
| mnist | Tiny-CNN | 1000 | 958 | 95.80% | 28117.62 us |
| personal | Perceptron | 129 | 124 | 96.12% | 752.03 us |
| personal | FNN | 129 | 128 | 99.22% | 593.30 us |
| personal | Tiny-CNN | 129 | 128 | 99.22% | 29047.95 us |
| external_usps | Perceptron | 100 | 58 | 58.00% | 800.85 us |
| external_usps | FNN | 100 | 93 | 93.00% | 413.48 us |
| external_usps | Tiny-CNN | 100 | 83 | 83.00% | 27921.47 us |

说明：PC 端 Tiny-CNN 时间受 Python 循环实现影响较大，不能直接等同于 STM32 端 C 代码时间；报告中可作为相对复杂度说明，最终硬件时间应以板端串口或屏幕实测为准。

## 易混淆样例

`tools/host_batch_test.py` 现在会统计每个测试集、每个模型中真实标签到预测标签的错误对，并保留最多 3 个样例文件名。自动化测试网页运行后会展示最容易混淆的几组数字。

当前 1000 张 MNIST 标准集上，Perceptron 容易把 `2 -> 8`、`4 -> 9`、`7 -> 9` 混淆；FNN 和 Tiny-CNN 的错误数量明显减少。个人采集集上 FNN 与 Tiny-CNN 均为 128/129，主要剩余错误可在网页的混淆表中直接定位样例。

## 进阶结论

- 标准测试集扩展到 1000 张后，FNN 为 929/1000，Tiny-CNN 为 958/1000，均优于 Perceptron。
- 新采集个人手写集上，FNN 与 Tiny-CNN 均达到 128/129，说明上位机采集数据已经可以作为可靠的个人测试集。
- USPS 外部公开集上，FNN 达到 93/100，Tiny-CNN 为 83/100；外部来源和 MNIST 分布差异较大，后续可继续通过数据增强或迁移测试优化泛化能力。
- Perceptron 保留为基础模型和低资源对照，准确率不作为进阶方案主指标。
- 字母识别固件主线为 Letter-Perceptron、Letter-FNN、Letter-DS-CNN 三个模型。完整 EMNIST Letters 测试中，Letter-DS-CNN 最好结果为 18739/20800，90.09%，高于普通 Letter-Tiny-CNN 的 18600/20800，89.42%；当前网页展示用 260 张 `tf_card/emnist_letters` 子集上普通 CNN 为 234/260，DS-CNN 为 231/260，后续可继续扩充字母 TF 卡测试集来降低小样本波动。
- Keil 验证：数字域 P/F/C build 通过，约 100.6 KB Flash、13.6 KB SRAM；字母域 P/F/DS-CNN build 通过，约 179.2 KB Flash、43.5 KB SRAM，仍低于 STM32F103VE 的 512 KB Flash / 64 KB SRAM 限制。
