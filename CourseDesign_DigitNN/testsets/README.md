# Testsets

TF 卡建议目录：

```text
tf_card/
├── manifest.csv
├── external_usps/
│   ├── usps_0000_6.bmp
│   ├── usps_0001_5.bmp
│   └── label.txt
├── mnist/
│   ├── img_0000_7.bmp
│   ├── img_0001_2.bmp
│   └── label.txt
└── personal/
    ├── number_00_7.bmp
    ├── number_01_1.bmp
    └── label.txt
```

`label.txt` 格式：

```text
img_0000_7.bmp,7
img_0001_2.bmp,2
```

生成 MNIST 标准 BMP 测试集：

```powershell
python tools\make_testset.py --output tf_card\mnist --count 100
```

个人测试集要求至少 10 张，可用画图软件保存 28x28 或更大尺寸 BMP，文件名包含真实标签，例如 `my_0003_5.bmp`。

生成 USPS 外部公开手写测试集：

```powershell
python tools\make_external_usps.py --output-dir tf_card\external_usps --per-class 10
python tools\build_tf_manifest.py
```

当前已准备好可直接复制到 TF 卡根目录的镜像：`CourseDesign_DigitNN/tf_card/`。
