# Letter Models Package

本目录整理 A-Z 字母识别相关模型、量化权重、评估指标、训练脚本和 STM32 端 C 权重缓存。对应压缩包为：

```text
../letter_models_20260706.zip
```

## 目录说明

| 目录 | 内容 |
| --- | --- |
| `models_float_pt/` | PyTorch 浮点训练模型，便于继续训练或重新导出。 |
| `weights_quant_npz/` | int8 / int32 量化权重，供导出 C 数组或上位机复核。 |
| `metrics/` | 每轮训练指标与混淆信息，`metrics_summary.csv` 汇总最佳轮次。 |
| `classes/` | `letter_classes.json`，A-Z 类别顺序。 |
| `firmware_letter_generated/` | 已导出的字母固件 C 权重缓存，包含 P/F/DS-CNN 三模型。 |
| `tools/` | 字母模型训练与 EMNIST Letters 测试集生成脚本。 |
| `legacy_reference/` | 旧版 Letter-Tiny-CNN 对照模型，不是当前字母固件主模型。 |

## 当前主模型

| 模型 | 文件 | 当前结构 | 最佳准确率 | 用途 |
| --- | --- | --- | ---: | --- |
| Letter-Perceptron | `letter_perceptron.*` | `784 -> 26` | 66.40% | 线性 baseline。 |
| Letter-FNN | `letter_fnn.*` | `784 -> 96 -> 26` | 86.11% | 字母全连接主力模型。 |
| Letter-DS-CNN | `letter_ds_cnn.*` | `Conv12 -> DW/PW -> DW/PW -> FC26` | 90.09% | 当前字母 C 模型。 |

`legacy_reference/letter_cnn.*` 是旧版普通 Tiny-CNN 对照模型，最佳准确率 89.42%。当前字母固件的 C 模型采用 DS-CNN。

## 固件权重说明

`firmware_letter_generated/RecognitionDomain.h` 应包含：

```c
#define RECOGNITION_DOMAIN         RECOGNITION_DOMAIN_LETTER
#define RECOGNIZER_CLASS_COUNT     26U
#define RECOGNIZER_LABEL_BASE      'A'
```

该目录中的 `PerceptronData.c/h`、`FNN_Data.c/h`、`CNN_Data.c/h` 是字母 P/F/DS-CNN 的板端权重。注意：工程根目录当前激活的 `firmware/generated` 或 Keil `User/digit_nn/generated` 可能是数字权重，切换字母固件时应使用本包中的完整字母 generated 文件，避免数字和字母权重混烧。

## 建议使用方式

1. 写课程报告：引用 `metrics/metrics_summary.csv` 和各模型 metrics JSON。
2. 继续训练：使用 `models_float_pt/` 与 `tools/train_letters.py`。
3. 检查量化参数：使用 `weights_quant_npz/`。
4. 准备字母固件：把 `firmware_letter_generated/` 中的整套文件同步到 Keil 工程 `User/digit_nn/generated/`，再 Build/Flash。

## 重新训练示例

```powershell
cd CourseDesign_DigitNN
python tools\make_emnist_letters_testset.py
python tools\train_letters.py --model all --epochs 8 --batch-size 128 --augment --export-c --export-keil
```

## 清单

完整文件清单见 `file_manifest.csv`。压缩包内应有 25 个文件。
