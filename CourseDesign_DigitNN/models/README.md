# Models

本目录保存训练后的模型、量化权重和评估指标。数字和字母模型分开维护，烧录时也应按任务域分别导出，避免数字/字母权重混烧。

## 数字模型

```text
models/
├── perceptron.pt
├── perceptron_quant.npz
├── perceptron_metrics.json
├── fnn.pt
├── fnn_quant.npz
├── fnn_metrics.json
├── cnn.pt
├── cnn_quant.npz
├── cnn_metrics.json
└── tf_card_eval.json
```

常用命令：

```powershell
python tools\train_mnist.py --model perceptron --epochs 2 --batch-size 512 --export-c --export-keil
python tools\train_mnist.py --model fnn --epochs 8 --batch-size 512 --augment --export-c --export-keil
python tools\train_mnist.py --model cnn --epochs 5 --batch-size 512 --augment --export-c --export-keil
python tools\evaluate_tf_card.py
```

`--export-c` 会覆盖 `firmware/generated` 中对应 C 参数文件；`--export-keil` 会同步到 `keil_touch_digit_nn/User/digit_nn/generated`。

## 字母模型

```text
models/
├── letter_classes.json
├── letter_perceptron.pt
├── letter_perceptron_quant.npz
├── letter_perceptron_metrics.json
├── letter_fnn.pt
├── letter_fnn_quant.npz
├── letter_fnn_metrics.json
├── letter_ds_cnn.pt
├── letter_ds_cnn_quant.npz
├── letter_ds_cnn_metrics.json
├── letter_cnn.pt
├── letter_cnn_quant.npz
└── letter_cnn_metrics.json
```

当前字母主模型为 `letter_perceptron`、`letter_fnn` 和 `letter_ds_cnn`。`letter_cnn` 是旧版普通 Tiny-CNN 对照模型。

| 模型 | 结构 | 最佳准确率 | 说明 |
| --- | --- | --- | --- |
| `letter_perceptron` | `784 -> 26` | 66.40% | 线性 baseline，最小最轻。 |
| `letter_fnn` | `784 -> 96 -> 26` | 86.11% | 字母全连接主力模型。 |
| `letter_ds_cnn` | `Conv12 -> DW/PW -> DW/PW -> FC26` | 90.09% | 当前字母 C 模型，优先用于板端演示。 |
| `letter_cnn` | 普通 Tiny-CNN | 89.42% | 历史对照，不作为当前固件主模型。 |

训练与导出：

```powershell
python tools\make_emnist_letters_testset.py
python tools\train_letters.py --model all --epochs 8 --batch-size 128 --augment --export-c --export-keil
```

## 量化文件说明

- `.pt`：PyTorch 浮点模型，用于继续训练或复现实验。
- `_quant.npz`：量化后的权重、偏置和结构参数，供上位机推理或导出 C 数组使用。
- `_metrics.json`：按 epoch 记录 loss、accuracy 和混淆字符对。
- `tf_card_eval.json`：数字 TF 卡测试集的批量评估汇总。

## 交付包

字母模型与权重已整理到：

```text
packages/letter_models_20260706/
packages/letter_models_20260706.zip
```

包内包含浮点模型、量化权重、metrics、A-Z 类别表、训练脚本和 `firmware_letter_generated/` 字母固件 C 权重。
