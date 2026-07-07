# Testsets

测试集统一使用 `tf_card/` 作为可复制到 TF 卡根目录的镜像。正式测试目录均使用 BMP 文件和 `label.txt`。

## 推荐目录

```text
tf_card/
├── manifest.csv
├── mnist/             # MNIST 标准数字测试集，1000 张
├── personal/          # 个人/上位机采集数字测试集，129 张
├── external_usps/     # USPS 外部数字测试集，100 张
├── emnist_letters/    # A-Z 字母测试集，260 张
└── ui_collected/      # 上位机采集缓存，不默认进入 manifest
```

## 标签格式

```text
img_0000_7.bmp,7
web_20260703_224931_505539_2.bmp,2
emnist_0000_A.bmp,A
```

## 生成与更新

生成 MNIST 标准 BMP 测试集：

```powershell
python tools\make_testset.py --output tf_card\mnist --count 1000
```

生成 USPS 外部公开手写测试集：

```powershell
python tools\make_external_usps.py --output-dir tf_card\external_usps --per-class 10
```

生成 EMNIST Letters 字母测试集：

```powershell
python tools\make_emnist_letters_testset.py
```

更新汇总清单：

```powershell
python tools\build_tf_manifest.py
```

## 自动化测试

```powershell
python tools\evaluate_tf_card.py
```

网页上位机的“自动化测试”页可分别执行数字检测和字母检测，并展示准确率、平均耗时和易混淆字符对。
