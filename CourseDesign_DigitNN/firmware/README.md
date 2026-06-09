# Firmware

本目录提供可加入 Keil 工程的 C 代码骨架；当前实物演示主工程在 `../keil_touch_digit_nn/`。

## 推荐加入工程的文件

- `inc/core/*.h`
- `src/core/*.c`
- `inc/drivers/*.h`
- `src/drivers/*.c`
- `generated/PerceptronData.c/.h`
- `generated/FNN_Data.c/.h`
- `generated/CNN_Data.c/.h`

## 移植到野火 STM32 工程

1. 先复制或引用现有野火工程的 `stm32f10x` 标准库、`bsp_ili9341_lcd`、`bsp_xpt2046_lcd`、`bsp_usart`、`bsp_systick`。
2. 将本目录 `inc` 添加到 Include Paths。
3. 将 `src/core` 和 `generated` 加入编译。
4. `src/drivers` 中的文件是适配层，需根据实际野火驱动函数名补全 LCD、触摸屏、TF 卡和串口发送函数。
5. 主循环建议按 `docs/algorithm_flow_mermaid.md` 实现。

## 内存估算

- Perceptron：`10 x 784` int8 权重约 7.7 KB，偏置约 40 B。
- FNN 64 隐藏层：第一层约 49 KB，第二层约 640 B，偏置约 296 B。
- Tiny-CNN：两层 3x3 卷积和一层全连接，int8 权重约 4.3 KB，中间特征缓冲约 4.7 KB RAM。
- 28x28 图像缓冲区约 784 B。

这些数据均可放入 STM32F103ZE/VE 的 Flash；若使用 Flash 较小的芯片，需要降低 FNN 隐藏层数量，或只保留 Perceptron + Tiny-CNN 对比。
