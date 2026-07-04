# CourseDesign_DigitNN

基于神经网络的手写数字识别系统，面向《微控制器与嵌入式系统》课程设计题目 6。

## 目标

本目录按任务书和培训文档先搭好可持续迭代的工程骨架：

- 基本任务：触摸屏采集手写轨迹，预处理到 28x28 图像，训练单层感知机，导出 `PerceptronData.c/.h`，在 STM32 端完成推理。
- 进阶任务：多层全连接网络 FNN 与 Tiny-CNN 训练/部署，MNIST 与个人手写 BMP 测试集，批量自动化测试，串口结果上报，实时上位机 UI，后续可扩展多类型字符/EMNIST。
- 规范要求：C 代码使用蛇形命名、`.h/.c` 分离、Doxygen 风格注释、按 `inc/src` 与 `core/drivers/utils` 分层。

## 目录

```text
CourseDesign_DigitNN/
├── docs/                 # 需求摘录、系统设计、流程图、串口协议、进度清单
├── firmware/             # STM32 C 端核心算法和驱动适配接口
│   ├── generated/        # 训练脚本导出的模型参数 C 文件
│   ├── inc/
│   │   ├── core/
│   │   ├── drivers/
│   │   └── utils/
│   └── src/
│       ├── core/
│       ├── drivers/
│       └── utils/
├── host_app/             # 上位机/串口监控方案
├── keil_touch_digit_nn/  # 基于野火触摸画板例程改造的 Keil 实物工程
├── models/               # 训练权重、量化参数、模型评估结果
├── report/               # 课程设计报告草稿
├── testsets/             # TF 卡测试集组织方式
└── tools/                # 训练、导出、测试集制作和 PC 端验证脚本
```

## 推荐推进顺序

1. 运行 `tools/train_mnist.py --model perceptron --epochs 2 --batch-size 512 --export-c --export-keil`，生成基础模型参数。当前 Perceptron 官方 MNIST 测试准确率约 89.25%。
2. 打开 `keil_touch_digit_nn/Project/RVMDK（uv5）/BH-F103.uvprojx`，使用 DAPLink/CMSIS-DAP 编译下载。
3. Keil 工程已接入野火 STM32 的 ILI9341 LCD 与 XPT2046 触摸屏驱动，触摸点会送入 `preprocess_add_point()` 并通过 `REC` 按钮触发识别。
4. 运行 `tools/train_mnist.py --model fnn --epochs 8 --batch-size 512 --augment --export-c --export-keil`，完成增强 FNN 部署。当前 FNN 官方 MNIST 测试准确率约 94.48%。
5. 运行 `tools/train_mnist.py --model cnn --epochs 5 --batch-size 512 --augment --export-c --export-keil`，完成 Tiny-CNN 进阶部署。当前 Tiny-CNN 官方 MNIST 测试准确率约 95.97%。
6. 运行 `tools/evaluate_tf_card.py`，对 `tf_card/mnist`、`tf_card/personal` 与 `tf_card/external_usps` 做 PC 端批量评估并生成 `models/tf_card_eval.json`。
7. 运行 `python host_app\web_dashboard_server.py`，浏览器打开 `http://127.0.0.1:8765/`，进入网页上位机，用于画布输入、像素预览、量化模型置信度展示、串口联调和样本采集。
8. 如需一键构建/下载，在 UI 的 `Firmware Deploy` 面板填写 `UV4.exe` 路径，或运行 `python tools\keil_flash.py --action build`、`python tools\keil_flash.py --action flash`。
9. 如需尝试英文字母，运行 `python tools\train_alnum.py --epochs 5 --batch-size 512 --augment` 训练 PC 端 `0-9A-Z` 原型。

## 规范入口

- 任务和评分要求：`docs/requirements_from_pdfs.md`
- 总体方案：`docs/system_design.md`
- 程序流程：`docs/algorithm_flow_mermaid.md`
- 串口协议：`docs/serial_protocol.md`
- 上位机与多字符升级说明：`docs/upgrade_notes.md`
- 联调结果：`docs/test_results.md`
- 多类型字符扩展：`docs/multitype_extension_plan.md`
- Keil 构建/烧录脚本：`tools/keil_flash.py`
- 36 类字母原型训练：`tools/train_alnum.py`
- 报告草稿：`report/report_draft.md`
