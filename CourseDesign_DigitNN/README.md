# CourseDesign_DigitNN

基于神经网络的手写数字识别系统，面向《微控制器与嵌入式系统》课程设计题目 6。

## 目标

本目录按任务书和培训文档先搭好可持续迭代的工程骨架：

- 基本任务：触摸屏采集手写轨迹，预处理到 28x28 图像，训练单层感知机，导出 `PerceptronData.c/.h`，在 STM32 端完成推理。
- 进阶任务：多层全连接网络 FNN 训练/部署，MNIST 与个人手写 BMP 测试集，批量自动化测试，串口结果上报，后续可扩展 CNN/边缘协同/EMNIST。
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

1. 运行 `tools/train_mnist.py --model perceptron --epochs 5 --export-c`，生成基础模型参数。当前已生成一版 2 epoch 参数，测试集准确率约 89.26%。
2. 打开 `keil_touch_digit_nn/Project/RVMDK（uv5）/BH-F103.uvprojx`，使用 DAPLink/CMSIS-DAP 编译下载。
3. Keil 工程已接入野火 STM32 的 ILI9341 LCD 与 XPT2046 触摸屏驱动，触摸点会送入 `preprocess_add_point()` 并通过 `REC` 按钮触发识别。
4. 运行 `tools/train_mnist.py --model fnn --epochs 8 --export-c`，完成 FNN 进阶部署。当前已生成一版 2 epoch 参数，测试集准确率约 92.21%。
5. 用 `tools/make_testset.py` 生成 MNIST BMP 测试集，把个人手写图放进 `testsets/personal`，再做 STM32/PC 端批量测试。当前 `testsets/mnist` 已生成 20 张 BMP 联调集。

## 规范入口

- 任务和评分要求：`docs/requirements_from_pdfs.md`
- 总体方案：`docs/system_design.md`
- 程序流程：`docs/algorithm_flow_mermaid.md`
- 串口协议：`docs/serial_protocol.md`
- 联调结果：`docs/test_results.md`
- 报告草稿：`report/report_draft.md`
