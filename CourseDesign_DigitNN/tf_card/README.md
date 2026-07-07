# TF Card Test Sets

本目录是 microSD/TF 卡根目录镜像。将本目录中的正式测试集目录和 `manifest.csv` 复制到 TF 卡根目录，即可作为课程设计测试集使用。

## Directory Layout

```text
/
├── manifest.csv
├── mnist/
│   ├── img_0000_7.bmp
│   └── label.txt
├── personal/
│   ├── web_20260703_224848_086937_7.bmp
│   └── label.txt
├── external_usps/
│   ├── usps_0000_6.bmp
│   └── label.txt
├── emnist_letters/
│   ├── emnist_0000_A.bmp
│   └── label.txt
└── ui_collected/
    ├── web_*.bmp
    ├── web_*_raw.png
    ├── label.txt
    └── capture_log.csv
```

`ui_collected/` 是上位机采集缓存目录，不作为默认 `manifest.csv` 正式测试集；同步个人测试集时再复制到 `personal/`。

## Label Rule

各测试集同时满足两种标签记录方式：

- 文件名包含真实标签：`img_0000_7.bmp` 的真实标签为 `7`，`emnist_0000_A.bmp` 的真实标签为 `A`。
- 每个目录提供 `label.txt`，格式为 `filename,label`。
- `manifest.csv` 汇总正式测试集目录，便于上位机或后续板端批量扫描。

## Counts

| 目录 | 数量 | 用途 |
| --- | ---: | --- |
| `mnist/` | 1000 BMP | 标准数字测试集 |
| `personal/` | 129 BMP | 个人/上位机采集数字测试集 |
| `external_usps/` | 100 BMP | USPS 外部数字泛化测试 |
| `emnist_letters/` | 260 BMP | A-Z 字母测试准备，每类 10 张 |
| `ui_collected/` | 131 BMP + 131 raw PNG | 上位机采集缓存 |

`*_raw.png` 为调试原图，已加入 `.gitignore`；正式评估使用 BMP 和 `label.txt`。

## External Sources

USPS 数据集包含美国邮政手写数字，原始图像为 16 x 16 灰度图。本项目通过 `sklearn.datasets.fetch_openml("usps", version=2)` 拉取，并转换为 28 x 28 BMP。

EMNIST Letters 数据集提供 A-Z 手写字母样本。脚本会对 EMNIST 原始朝向做旋转和镜像修正，再导出为与数字工作流一致的 28 x 28 灰度 BMP。

## PC Verification

```powershell
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\mnist --model perceptron
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\personal --model fnn
python CourseDesign_DigitNN\tools\host_batch_test.py --set-dir CourseDesign_DigitNN\tf_card\external_usps --model cnn
python CourseDesign_DigitNN\tools\evaluate_tf_card.py
python CourseDesign_DigitNN\tools\build_tf_manifest.py
```

自动化测试网页会额外展示每个测试集最容易混淆的数字或字母对。

## 板端 TF 卡说明

TF 卡插在 STM32 开发板上时，电脑不会出现对应盘符；电脑看到的是本地制卡镜像目录。板端是否读卡成功，需要依赖固件串口回传 FatFs/SDIO 诊断帧。当前网页 TF 卡页用于展示镜像结构和预留板端诊断入口。
