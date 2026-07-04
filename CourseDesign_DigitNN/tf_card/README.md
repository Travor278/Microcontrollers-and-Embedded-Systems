# TF Card Test Sets

本目录是 microSD/TF 卡根目录镜像。拿到 TF 卡后，将本目录中的 `mnist/`、`personal/`、`external_usps/`、`emnist_letters/` 和 `manifest.csv` 复制到 TF 卡根目录即可作为测试集使用。

## Directory Layout

```text
/
├── manifest.csv
├── external_usps/
│   ├── usps_0000_6.bmp
│   ├── usps_0001_5.bmp
│   └── label.txt
├── mnist/
│   ├── img_0000_7.bmp
│   ├── img_0001_2.bmp
│   └── label.txt
├── personal/
│   ├── web_20260703_224848_086937_7.bmp
│   ├── web_20260703_224900_966252_8.bmp
│   └── label.txt
└── emnist_letters/
    ├── emnist_0000_A.bmp
    ├── emnist_0001_A.bmp
    └── label.txt
```

## Label Rule

各测试集同时满足两种标签记录方式：

- 文件名包含真实标签：`img_0000_7.bmp` 的真实标签为 `7`，`web_20260703_224931_505539_2.bmp` 的真实标签为 `2`，`emnist_0000_A.bmp` 的真实标签为 `A`。
- 每个目录提供 `label.txt`，格式为 `filename,label`。
- `manifest.csv` 汇总正式测试集目录，便于上位机或后续板端批量扫描；`ui_collected/` 作为采集缓存默认不重复写入。

## Counts

- `mnist/`：1000 张，来自 MNIST 测试集，用作标准数字测试集。
- `personal/`：129 张，来自上位机 `ui_collected/` 新采集样本，使用同一套 28x28 预处理结果，用作个人手写数字测试集。
- `external_usps/`：100 张，来自公开 USPS 手写数字集，每类 10 张，用于补充外部泛化测试。
- `emnist_letters/`：260 张，来自 EMNIST Letters 测试集，每类 10 张，用于字母识别准备。
- `ui_collected/`：原始上位机采集缓存目录，当前已同步到 `personal/`，不参与默认 `manifest.csv`。

## External Sources

USPS 数据集包含美国邮政手写数字，原始图像为 16x16 灰度图。本项目通过 `sklearn.datasets.fetch_openml("usps", version=2)` 拉取，并转换为 28x28 BMP。

EMNIST Letters 数据集提供 A-Z 手写字母样本。脚本会对 EMNIST 原始朝向做旋转和镜像修正，再导出为与数字工作流一致的 28x28 灰度 BMP。

## PC Verification

```powershell
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\mnist --model perceptron
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\personal --model fnn
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\external_usps --model cnn
python CourseDesign_DigitNN\tools\evaluate_tf_card.py
python CourseDesign_DigitNN\tools\build_tf_manifest.py
```

当前 PC 端量化模型汇总结果见 `CourseDesign_DigitNN/models/tf_card_eval.json` 和 `CourseDesign_DigitNN/docs/test_results.md`。自动化测试网页会额外展示最容易混淆的数字对。
