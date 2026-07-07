# CourseDesign_DigitNN

基于神经网络的手写数字/字母识别系统，面向《微控制器与嵌入式系统》课程设计题目 6。当前工程已经从基础数字识别扩展到网页上位机、字母识别、自动化测试、中文识别 API 原型和模型打包交付。

## 已完成能力

- 板端采集：STM32F103VE + ILI9341 LCD + XPT2046 电阻触摸屏，触摸轨迹预处理为 28 x 28 灰度输入。
- 数字识别：Perceptron、FNN、Tiny-CNN/DS-CNN 三模型量化推理，LCD 与串口输出 P/F/C 结果。
- 字母识别：A-Z 独立固件域，模型为 Letter-Perceptron、Letter-FNN、Letter-DS-CNN，不与数字权重混烧。
- 上位机：网页 Dashboard 支持中英文切换、手写输入、板端实时轨迹、像素预览、模型置信度、自动化测试、样本采集、Keil 构建/烧录、中文识别和 TF 卡页。
- 量化展示：首页解释 float32 到 int8/int32 的 scale、zero point、bias、accumulator、multiplier/shift 流程。
- 数据集：`tf_card/` 整理 MNIST、personal、USPS、EMNIST Letters 和采集缓存。
- 字母模型包：`packages/letter_models_20260706/` 与 `.zip` 已整理模型、量化权重、metrics 和字母 generated C 文件。

## 目录

```text
CourseDesign_DigitNN/
├── docs/                 # 需求摘录、系统设计、流程图、串口协议、进度和升级说明
├── firmware/             # STM32 C 端核心算法和驱动适配接口
│   ├── generated/        # 当前导出的模型参数 C 文件
│   ├── generated_cache/  # digit/letter 分域缓存，默认不提交
│   ├── inc/
│   └── src/
├── host_app/             # 网页上位机、串口监控和本地服务
├── keil_touch_digit_nn/  # 基于野火触摸画板例程改造的 Keil 实物工程
├── models/               # 训练权重、量化参数、模型评估结果
├── packages/             # 课程设计交付包，如字母模型包
├── report/               # 课程设计报告草稿
├── testsets/             # 测试集组织说明
├── tf_card/              # 可复制到 TF 卡根目录的测试集镜像
└── tools/                # 训练、导出、测试集制作、Keil 和 API 工具脚本
```

## 数字识别流程

```powershell
cd CourseDesign_DigitNN
python tools\train_mnist.py --model perceptron --epochs 2 --batch-size 512 --export-c --export-keil
python tools\train_mnist.py --model fnn --epochs 8 --batch-size 512 --augment --export-c --export-keil
python tools\train_mnist.py --model cnn --epochs 5 --batch-size 512 --augment --export-c --export-keil
python tools\evaluate_tf_card.py
```

数字固件只包含数字 P/F/C 权重。网页中 `Build` 或 `Flash` 可复用已导出的权重；只有 `Export` 或 `Export+Flash` 会重新训练/导出。

## 字母识别流程

```powershell
cd CourseDesign_DigitNN
python tools\make_emnist_letters_testset.py
python tools\train_letters.py --model all --epochs 8 --batch-size 128 --augment --export-c --export-keil
```

当前字母主模型：

| 模型 | 结构 | 最佳准确率 |
| --- | --- | --- |
| Letter-Perceptron | `784 -> 26` | 66.40% |
| Letter-FNN | `784 -> 96 -> 26` | 86.11% |
| Letter-DS-CNN | `Conv12 -> DW/PW -> DW/PW -> FC26` | 90.09% |

普通 Letter-Tiny-CNN 作为历史对照保存在 `packages/letter_models_20260706/legacy_reference/`。

## 上位机运行

```powershell
cd CourseDesign_DigitNN
python host_app\web_dashboard_server.py
```

浏览器打开：

```text
http://127.0.0.1:8765/
```

网页页面包括：

- 首页：模型、量化、Flash/SRAM、固件流程可视化。
- 数字工作区：浏览器输入或 STM32 板端输入，显示 P/F/C 推理结果。
- 字母工作区：浏览器/板端输入 A-Z，显示 Letter P/F/C 结果。
- 自动化测试：批量评估数字和字母测试集，统计准确率、平均耗时和混淆对。
- 中文识别：STM32 作为手写板，上位机或视觉 API 负责中文识别。
- TF 卡页：展示本地制卡镜像和板端读卡诊断入口。

## Keil 工程

打开：

```text
keil_touch_digit_nn/Project/RVMDK（uv5）/BH-F103.uvprojx
```

下载方式：CMSIS-DAP / DAPLink，Debug 与 Utilities 均选择 `CMSIS-DAP Debugger`，Port 选择 `SW`。

命令行构建/烧录：

```powershell
python tools\keil_flash.py --action build
python tools\keil_flash.py --action flash --uv4 D:\UV4\UV4.exe
python tools\keil_flash.py --action export-build-flash --domain letter --model all --epochs 3 --batch-size 128 --augment
```

## 规范入口

- 任务和评分要求：`docs/requirements_from_pdfs.md`
- 总体方案：`docs/system_design.md`
- 程序流程：`docs/algorithm_flow_mermaid.md`
- 串口协议：`docs/serial_protocol.md`
- 上位机与多字符升级说明：`docs/upgrade_notes.md`
- 联调结果：`docs/test_results.md`
- Keil 构建/烧录脚本：`tools/keil_flash.py`
- 字母模型包：`packages/letter_models_20260706/README.md`
- 报告草稿：`report/report_draft.md`
