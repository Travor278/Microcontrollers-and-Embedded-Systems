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
| FNN | 8 epoch + affine augmentation | 94.48% | 进阶全连接模型，个人手写泛化明显改善 |
| Tiny-CNN | 5 epoch + affine augmentation | 95.97% | 卷积进阶模型，参数量小但运算量更高 |

## TF 卡测试集

TF 卡目录镜像：

```text
CourseDesign_DigitNN/tf_card/
├── manifest.csv
├── mnist/          # MNIST 标准测试集，100 张 BMP
├── personal/       # 个人手写测试集，10 张 BMP
└── external_usps/  # USPS 公开手写测试集，100 张 BMP
```

个人手写集来自 `testsets/personal/raw/number.jpg`，标签序列为 `7132564908`。对比 `dilate-size=1/3/5` 后，`dilate-size=3` 在三模型上更均衡，因此当前 `tf_card/personal` 使用厚度 3 的预处理结果。

外部公开集来自 USPS handwritten digits。该数据集为 16x16 灰度手写数字，本项目通过 `sklearn.datasets.fetch_openml("usps", version=2)` 拉取，每类选取 10 张并转换为 28x28 BMP，用于验证模型在非 MNIST 来源上的泛化能力。

汇总命令：

```powershell
python CourseDesign_DigitNN\tools\evaluate_tf_card.py
```

PC 端量化模型结果：

| 测试集 | 模型 | 图片数量 | 正确数量 | 准确率 | PC 平均推理时间 |
| --- | --- | ---: | ---: | ---: | ---: |
| mnist | Perceptron | 100 | 90 | 90.00% | 180.19 us |
| mnist | FNN | 100 | 98 | 98.00% | 372.59 us |
| mnist | Tiny-CNN | 100 | 98 | 98.00% | 26053.06 us |
| personal | Perceptron | 10 | 7 | 70.00% | 207.08 us |
| personal | FNN | 10 | 10 | 100.00% | 338.62 us |
| personal | Tiny-CNN | 10 | 9 | 90.00% | 67472.91 us |
| external_usps | Perceptron | 100 | 58 | 58.00% | 897.64 us |
| external_usps | FNN | 100 | 93 | 93.00% | 738.15 us |
| external_usps | Tiny-CNN | 100 | 89 | 89.00% | 67481.25 us |

说明：PC 端 Tiny-CNN 时间由 Python 循环实现影响较大，不能直接等同于 STM32 端 C 代码时间；报告中可作为相对复杂度说明，最终硬件时间应以板端串口或屏幕实测为准。

## 进阶结论

- 扩展到 100 张 MNIST 标准测试图后，FNN 和 Tiny-CNN 均达到 98/100，明显优于 Perceptron。
- 个人手写集上，增强 FNN 达到 10/10，Tiny-CNN 达到 9/10，明显优于旧版 FNN 的 5/10。
- USPS 外部公开集上，FNN 达到 93/100，Tiny-CNN 达到 89/100，说明模型对非 MNIST 来源也有一定泛化能力。
- Perceptron 保留为基础模型和低资源对照，准确率不作为进阶方案主指标。
- 下一步若接入 TF 卡/FATFS，可在 STM32 端复现同一组 `label.txt` 批量测试，并记录真实推理时间。
