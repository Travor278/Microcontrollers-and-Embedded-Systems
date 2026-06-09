# TF Card Test Sets

本目录是 microSD/TF 卡根目录镜像。拿到 TF 卡后，将本目录中的 `mnist/`、`personal/`、`external_usps/` 和 `manifest.csv` 复制到 TF 卡根目录。

## Directory Layout

```text
/
├── manifest.csv
├── external_usps/
│   ├── usps_0000_6.bmp
│   ├── usps_0001_5.bmp
│   ├── ...
│   └── label.txt
├── mnist/
│   ├── img_0000_7.bmp
│   ├── img_0001_2.bmp
│   ├── ...
│   └── label.txt
└── personal/
    ├── number_00_7.bmp
    ├── number_01_1.bmp
    ├── ...
    └── label.txt
```

## Label Rule

三组测试集同时满足两种标签记录方式：

- 文件名包含真实标签：`img_0000_7.bmp` 的真实标签为 `7`，`number_03_2.bmp` 的真实标签为 `2`，`usps_0000_6.bmp` 的真实标签为 `6`。
- 每个目录提供 `label.txt`，格式为 `filename,label`。

## Counts

- `mnist/`：100 张，来自 MNIST 测试集，不少于任务书要求的 10 张。
- `personal/`：10 张，来自个人手写照片 `testsets/personal/raw/number.jpg`，采用 `dilate-size=3` 预处理，满足任务书要求的至少 10 张。
- `external_usps/`：100 张，来自公开 USPS 手写数字集，每类 10 张，用于补充外部泛化测试。

## External Source

USPS 数据集包含美国邮政手写数字，公开资料说明其训练/测试集为 16x16 灰度图，像素值缩放到 `[-1, 1]`。本项目通过 `sklearn.datasets.fetch_openml("usps", version=2)` 拉取，并转换为 28x28 BMP。

## PC Verification

```powershell
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\mnist --model perceptron
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\personal --model perceptron
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\external_usps --model fnn
python CourseDesign_DigitNN\tools\evaluate_tf_card.py
```

当前 PC 端量化模型汇总结果见 `CourseDesign_DigitNN/models/tf_card_eval.json` 与 `docs/test_results.md`。
