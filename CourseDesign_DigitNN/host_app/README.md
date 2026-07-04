# Host App

本目录包含两个上位机工具：

- `web_dashboard_server.py`：本地网页工作台，推荐用于答辩演示和日常采集。
- `realtime_digit_ui.py`：实时手写输入、28x28 像素预览、PC 端量化模型推理、串口结果显示、Flash/SRAM 利用率展示、样本一键保存。
- `serial_dashboard.py`：轻量串口日志脚本，用于记录 STM32 输出的 `RESULT`、`TEST`、`STATUS` 帧。

## Web Dashboard

运行：

```powershell
cd CourseDesign_DigitNN
python host_app\web_dashboard_server.py
```

如需运行网页自动化检查，先安装浏览器驱动：

```powershell
python -m playwright install chromium
```

浏览器打开：

```text
http://127.0.0.1:8765/
```

串口连接说明：

- 固件默认使用 USART1，参数为 `115200 8N1`，对应引脚为 `PA9/PA10`。
- SWD/DAPLink 负责下载调试，不能直接替代串口数据链路。
- 如果只按课程截图连接 DAP 仿真器和开发板电源，网页不会收到板端数据。
- 如果 DAPLink 没有把虚拟串口接到 `PA9/PA10`，网页不会收到实时数据。
- 野火指南者板通常使用板载“USB 转串口”接口查看串口输出，网页中选择它对应的 COM 口；本机当前常见显示为 `COM8 - USB-SERIAL CH340`。
- 当前固件在 USB 转串口连接后会输出逐点轨迹；点击 `REC` 后再输出一次 `IMAGE` 和 `RESULT`，板端 LCD 也会同步显示识别标签和置信度。

网页工作台支持：

- 右上角中英文切换，默认中文，便于答辩演示。
- 手写输入和放大后的模型像素预览；当前模型输入为 28x28，协议已预留更大输入尺寸。
- Perceptron、FNN、Tiny-CNN 的 PC 端量化推理和置信度展示。
- Python 后端枚举本机 COM 口，网页下拉框选择 `COM8 - USB-SERIAL CH340` 之类的串口，实时显示 `POINT` 轨迹并解析 STM32 `IMAGE`、`RESULT` 帧。
- 读取 Keil 构建日志中的 Flash/SRAM 利用率。
- 保存 `0-9A-Z` 样本到 `tf_card/ui_collected/`：网页画布用 `Save Sample`，板端触摸屏采集用 `Save Board Sample`，也可以开启 `Auto save after REC` 让每次板上点击 `REC` 后自动保存最新 `IMAGE` 帧。
- 调用 Keil 命令行执行模型导出、构建和下载。

## Tkinter UI

运行：

```powershell
cd CourseDesign_DigitNN
python host_app\realtime_digit_ui.py
```

连接单片机时可指定串口：

```powershell
python host_app\realtime_digit_ui.py --port COM3 --baud 115200
```

## 一键构建和烧录

Web Dashboard 和 Tkinter UI 中的 `Firmware Deploy` 面板都提供：

- `Export`：按选择的模型重新训练并同步参数到 Keil 工程。
- `Build`：调用 Keil 编译当前工程。
- `Flash`：调用 Keil 下载当前工程。
- `Export+Flash`：重新导出所选模型参数，然后编译并下载完整固件。

如果自动找不到 Keil，请在 `UV4.exe` 输入框中填写路径，例如：

```text
C:\Keil_v5\UV4\UV4.exe
```

也可以用环境变量：

```powershell
$env:KEIL_UV4 = "C:\Keil_v5\UV4\UV4.exe"
```

命令行等价用法：

```powershell
python tools\keil_flash.py --action build
python tools\keil_flash.py --action flash --uv4 C:\Keil_v5\UV4\UV4.exe
python tools\keil_flash.py --action export-build-flash --model cnn --epochs 5 --batch-size 512 --augment
```

说明：

- 网页里的 `Build`/`Flash` 调用的是 Keil 命令行 `UV4.exe -b/-f`，本质上仍依赖 Keil 工程、DAP 配置、Flash Download Algorithm 和 MDK 授权状态；它不能绕过 Keil Lite 的镜像大小限制。
- 如果 `Build` 日志出现 `L6047U: The size of this image exceeds the maximum allowed for this version of the linker`，说明当前 MDK 授权无法链接该工程，需要使用可链接更大镜像的 Keil 授权，或改用能完成构建的工具链。
- `Flash` 只下载已经存在的 `Output/DigitNN_Touch.axf`，不会自动重新编译；若 Build 失败或 AXF 不存在，应先在 Keil 中成功 Build，或修复命令行 Build。
- 当前板端模型类别仍为数字 `0-9`。PC 端已提供 `tools/train_alnum.py` 作为 `0-9A-Z` 原型训练入口。
- `Auto deskew` 只做图像级居中和轻微倾斜校正，不依赖笔顺。
- 如果要把 `ui_collected` 纳入 TF 卡汇总，运行 `python tools\build_tf_manifest.py` 即可更新 `tf_card/manifest.csv`。

## 串口日志脚本

运行示例：

```powershell
cd CourseDesign_DigitNN
python host_app\serial_dashboard.py --port COM3 --baud 115200
```
