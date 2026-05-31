# 当前联调结果

## 训练结果

训练命令：

```powershell
python CourseDesign_DigitNN\tools\train_mnist.py --model perceptron --epochs 2 --batch-size 512 --data-dir tmp\mnist --export-c
python CourseDesign_DigitNN\tools\train_mnist.py --model fnn --epochs 2 --batch-size 512 --data-dir tmp\mnist --export-c
```

结果：

| 模型 | Epoch | MNIST 测试准确率 |
| --- | ---: | ---: |
| Perceptron | 1 | 86.61% |
| Perceptron | 2 | 89.26% |
| FNN | 1 | 90.05% |
| FNN | 2 | 92.21% |

## BMP 联调测试

生成命令：

```powershell
python CourseDesign_DigitNN\tools\make_testset.py --output CourseDesign_DigitNN\testsets\mnist --data-dir tmp\mnist --count 20
```

PC 端量化模型测试命令：

```powershell
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\testsets\mnist --model perceptron
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\testsets\mnist --model fnn
```

结果：

| 模型 | BMP 数量 | 正确数量 | 准确率 | PC 平均推理时间 |
| --- | ---: | ---: | ---: | ---: |
| Perceptron | 20 | 19 | 95.00% | 838.58 us |
| FNN | 20 | 19 | 95.00% | 1282.14 us |

说明：20 张 BMP 只是链路联调集，不作为最终报告的完整性能结论。正式报告建议扩展到更多 MNIST 测试图和至少 10 张个人手写图，并记录 STM32 端平均推理时间。

## 个人手写照片测试

原始照片已移动到：

```text
testsets/personal/raw/number.jpg
```

照片包含从左到右 10 个数字，标签序列为 `7132564908`。使用以下命令自动分割、归一化并加粗笔画：

```powershell
python CourseDesign_DigitNN\tools\preprocess_personal_image.py --input CourseDesign_DigitNN\testsets\personal\raw\number.jpg --labels 7132564908 --output-dir CourseDesign_DigitNN\testsets\personal\processed_thick5 --prefix number --dilate-size 5
```

预处理预览图：

```text
testsets/personal/processed_thick5/number_preview.png
```

PC 端量化模型测试结果：

| 模型 | 图片数量 | 正确数量 | 准确率 | PC 平均推理时间 |
| --- | ---: | ---: | ---: | ---: |
| Perceptron | 10 | 8 | 80.00% | 587.39 us |
| FNN | 10 | 5 | 50.00% | 800.37 us |

结论：该照片可以作为个人手写测试集使用。当前模型对真实照片风格存在明显域差异，尤其是细长的 `7`、`9` 容易误判；笔画加粗后 Perceptron 有较好改善，可在报告中作为“标准测试集 vs 个人测试集”的对比分析材料。

## TF 卡目录

已按任务书进阶任务要求准备 TF 卡根目录镜像：

```text
CourseDesign_DigitNN/tf_card/
├── manifest.csv
├── mnist/       # MNIST 标准测试集，20 张 BMP
└── personal/    # 个人手写测试集，10 张 BMP
```

两组测试集均包含真实标签信息：图片文件名含标签，并提供 `label.txt` 对应关系。
