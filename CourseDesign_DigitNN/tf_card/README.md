# TF Card Test Sets

本目录是 microSD/TF 卡根目录镜像。拿到 TF 卡后，将本目录中的 `mnist/`、`personal/` 和 `manifest.csv` 复制到 TF 卡根目录。

## Directory Layout

```text
/
├── manifest.csv
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

两组测试集同时满足两种标签记录方式：

- 文件名包含真实标签：`img_0000_7.bmp` 的真实标签为 `7`，`number_03_2.bmp` 的真实标签为 `2`。
- 每个目录提供 `label.txt`，格式为 `filename,label`。

## Counts

- `mnist/`：20 张，来自 MNIST 测试集，不少于任务书要求的 10 张。
- `personal/`：10 张，来自个人手写照片 `testsets/personal/raw/number.jpg`，满足任务书要求的至少 10 张。

## PC Verification

```powershell
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\mnist --model perceptron
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\personal --model perceptron
```
