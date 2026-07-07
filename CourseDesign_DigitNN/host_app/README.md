# Host App

本目录包含课程设计上位机工具，推荐主入口为网页 Dashboard。

## 工具概览

- `web_dashboard_server.py`：本地网页工作台，推荐用于答辩演示、串口联调、样本采集、自动化测试和 Keil 构建/烧录。
- `realtime_digit_ui.py`：早期 Tkinter 实时手写输入与串口显示工具。
- `serial_dashboard.py`：轻量串口日志脚本，用于记录 STM32 输出的 `STATUS`、`POINT`、`STROKE`、`IMAGE`、`RESULT` 帧。
- `web/`：网页前端资源。

## Web Dashboard

运行：

```powershell
cd CourseDesign_DigitNN
python host_app\web_dashboard_server.py
```

浏览器打开：

```text
http://127.0.0.1:8765/
```

主要页面：

- 首页：解释 P/F/C/DS 模型、量化公式、Flash/SRAM 对比、固件流程和课程设计展示逻辑。
- 数字工作区：浏览器画板或 STM32 板端触摸输入，显示数字 P/F/C 推理结果。
- 字母工作区：A-Z 输入、Letter-Perceptron/Letter-FNN/Letter-DS-CNN 结果、字母固件构建/烧录。
- 自动化测试：对数字和字母测试集批量推理，统计准确率、平均运行时间和易混淆字符。
- 中文识别：STM32 只作为手写板，上位机或视觉 API 负责中文识别。
- TF 卡页：展示本地 `tf_card/` 镜像与板端读卡诊断入口。

## 串口连接

- 固件默认使用 USART1，参数为 `115200 8N1`，对应引脚为 `PA9/PA10`。
- SWD/DAPLink 负责下载调试，不能替代串口数据链路。
- 野火指南者板通常使用板载“USB 转串口”接口查看串口输出，网页中选择类似 `COM8 - USB-SERIAL CH340` 的端口。
- 当前协议支持逐点轨迹和 REC 后完整图像：
  - `POINT`：板端实时轨迹点。
  - `STROKE`：一笔结束。
  - `IMAGE`：28 x 28 模型输入图像。
  - `RESULT`：P/F/C 模型标签、置信度和耗时。

## 样本采集

网页可以保存浏览器画板或板端触摸屏采集样本：

- 浏览器画板：`Save Sample`。
- 板端触摸屏：`Save Board Sample`。
- 自动保存：开启 `REC 后自动保存`，每次板上点击 `REC` 后保存最新 `IMAGE` 帧。

采集缓存默认写入：

```text
tf_card/ui_collected/
```

`*_raw.png` 是调试用原始图，已加入 `.gitignore`；正式 BMP 和 `label.txt` 可用于同步到 `tf_card/personal/`。

## 构建和烧录

Web Dashboard 的 `Firmware Deploy` 面板提供：

- `Export`：按任务域和模型重新训练/导出权重。
- `Build`：调用 Keil 编译当前工程。
- `Flash`：调用 Keil 下载当前已有 AXF。
- `Export+Flash`：导出、构建并下载。

示例：

```powershell
python tools\keil_flash.py --action build
python tools\keil_flash.py --action flash --uv4 D:\UV4\UV4.exe
python tools\keil_flash.py --action export-build-flash --domain digit --model all --epochs 3 --batch-size 512 --augment
python tools\keil_flash.py --action export-build-flash --domain letter --model all --epochs 3 --batch-size 128 --augment
```

说明：

- 数字固件只包含数字 P/F/C 权重。
- 字母固件只包含字母 P/F/C 权重。
- `Build`/`Flash` 本质上仍依赖 Keil 工程、DAP 配置、Flash Download Algorithm 和 MDK 授权状态。
- 如果命令行 Build 超出 Keil Lite 链接限制，需要换可链接更大镜像的授权，或改用已能成功 Build 的 Keil 环境。

## API 密钥

中文识别页面可读取本地 `.env` / `.env.local` 中的 API 配置。密钥文件已加入 `.gitignore`，不要提交到仓库。
