# Firmware

本目录提供可加入 Keil 工程的 C 代码骨架和模型参数导出目录；当前实物演示主工程在 `../keil_touch_digit_nn/`。

## 目录说明

```text
firmware/
├── generated/              # 当前激活导出的 C 权重
├── generated_cache/
│   ├── digit/              # 数字固件 P/F/C 权重缓存
│   └── letter/             # 字母固件 P/F/DS-CNN 权重缓存
├── inc/core/               # 算法接口
├── inc/drivers/            # LCD/触摸/串口/TF 卡适配接口
├── src/core/               # 推理、预处理和识别器实现
└── src/drivers/            # 驱动适配层示例
```

`generated_cache/` 是导出缓存，默认不提交；如果需要交付字母 generated 文件，请使用 `packages/letter_models_20260706/firmware_letter_generated/`。

## 推荐加入 Keil 工程的文件

- `inc/core/*.h`
- `src/core/*.c`
- `generated/RecognitionDomain.h`
- `generated/PerceptronData.c/.h`
- `generated/FNN_Data.c/.h`
- `generated/CNN_Data.c/.h`

实物 Keil 工程已将这些核心文件同步到：

```text
keil_touch_digit_nn/User/digit_nn/core/
keil_touch_digit_nn/User/digit_nn/generated/
```

## 识别域

`RecognitionDomain.h` 决定当前固件类别域：

```c
#define RECOGNITION_DOMAIN_DIGIT   1U
#define RECOGNITION_DOMAIN_LETTER  2U
```

数字域：

- `RECOGNIZER_CLASS_COUNT = 10`
- `RECOGNIZER_LABEL_BASE = '0'`
- P/F/C 分别为 Perceptron、FNN、Tiny-CNN 或 DS-CNN。

字母域：

- `RECOGNIZER_CLASS_COUNT = 26`
- `RECOGNIZER_LABEL_BASE = 'A'`
- P/F/C 分别为 Letter-Perceptron、Letter-FNN、Letter-DS-CNN。

切换域时必须整体替换 `RecognitionDomain.h` 与三套模型 C 权重，不能只替换单个模型文件。

## 量化说明

当前板端推理使用：

- `uint8_t` 输入图像，范围 0..255。
- `int8_t` 权重。
- `int32_t` 偏置和累加器。
- 右移或 multiplier/shift 控制层间尺度。

这对应首页中解释的：

```text
r ~= scale * (q - zero_point)
acc = sum((x_q - zero_x) * w_q) + b_q
```

## 模型结构参考

数字域：

- Perceptron：`784 -> 10`。
- FNN：`784 -> 64 -> 10`。
- Tiny-CNN：`Conv4 -> Pool14 -> Conv8 -> Pool7 -> FC10`。

字母域：

- Letter-Perceptron：`784 -> 26`。
- Letter-FNN：`784 -> 96 -> 26`。
- Letter-DS-CNN：`Conv12 -> DW/PW -> DW/PW -> FC26`。

## 移植到野火 STM32 工程

1. 复制或引用野火工程的 `stm32f10x` 标准库、`bsp_ili9341_lcd`、`bsp_xpt2046_lcd`、`bsp_usart`、`bsp_systick`。
2. 将 `inc` 和 `generated` 添加到 Include Paths。
3. 将 `src/core` 和 `generated` 加入编译。
4. `src/drivers` 是适配层示例，需根据实际野火驱动函数名补全 LCD、触摸屏、TF 卡和串口发送函数。
5. 主循环建议按 `docs/algorithm_flow_mermaid.md` 实现。

## 内存关注点

- FNN 主要占 Flash，权重量化收益明显。
- CNN/DS-CNN 权重不一定最大，但中间特征图会占 SRAM。
- 字母 DS-CNN 比数字 Tiny-CNN 更大，切换字母域后应重新查看 Keil map 中 Flash/SRAM。
- STM32F103VE 为 512 KB Flash / 64 KB SRAM，适合当前量化模型；若换更小容量芯片，需要减少隐藏层或通道数。
