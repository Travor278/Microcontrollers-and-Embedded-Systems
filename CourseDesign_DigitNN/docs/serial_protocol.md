# 串口协议

默认参数：USART1，115200 baud，8N1。

## STM32 到上位机

实时触摸轨迹：

```text
INFO,fw=DigitNN_Touch,proto=touch_stream_v1,image=<width>x<height>
CLEAR
POINT,x=<lcd_x>,y=<lcd_y>
STROKE,end=1
```

- `INFO`：固件启动/清空时输出的版本提示，用于确认已烧录支持实时轨迹协议的新固件。
- `CLEAR`：板端清空画板或初始化识别缓存，上位机同步清空板端轨迹预览。
- `POINT`：触摸屏原始 LCD 坐标，当前画板有效区域约为 `x=81..319, y=0..239`。固件端已做节流，避免串口阻塞触摸响应。
- `STROKE`：一次笔画抬笔结束，上位机断开上一段连线，下一点重新起笔。

模型实际输入图像：

```text
IMAGE,w=<width>,h=<height>,data=<hex grayscale bytes>
```

当前模型输入为 `28x28`，每个像素用两位十六进制灰度表示，因此 `data` 长度为 `28*28*2=1568` 个字符。协议保留 `w/h` 字段，后续如果升级为 `32x32`，上位机无需改协议。

识别结果：

```text
RESULT,model=<P|F|C>,label=<0-9>,confidence=<0-100>,time_us=<n>
```

当前 Keil 工程在每次点击 `REC` 完成识别后，会依次输出三帧：

```text
IMAGE,w=28,h=28,data=<1568 hex chars>
RESULT,model=P,label=<0-9>,confidence=<0-100>,time_us=0
RESULT,model=F,label=<0-9>,confidence=<0-100>,time_us=0
RESULT,model=C,label=<0-9>,confidence=<0-100>,time_us=0
```

`time_us=0` 表示板端暂未加入精确推理耗时计时；后续可用定时器或 DWT 计数补齐。

批量测试统计：

```text
TEST,set=<mnist|personal>,model=<P|F|C>,total=<n>,correct=<n>,accuracy=<0-10000>,avg_time_us=<n>
```

系统状态：

```text
STATUS,state=<idle|drawing|infer|test>,message=<text>
```

错误状态可以额外携带 `status=<n>` 字段，上位机应忽略未知字段。

## 上位机到 STM32

以下命令为协议预留格式，`realtime_digit_ui.py` 已提供发送按钮；当前板端主要完成识别结果上报，串口命令接收可作为后续增强项。

```text
CMD,CLEAR
CMD,MODEL,P
CMD,MODEL,F
CMD,MODEL,C
CMD,TEST,mnist
CMD,TEST,personal
CMD,INFO
```

## 设计原则

- 使用 ASCII 文本帧，串口助手和 Qt 上位机都容易调试。
- 每帧以 `\r\n` 结束。
- 关键字段名固定，便于 PC 端脚本解析。
- 采集层保留高分辨率触摸轨迹，模型层再归一化为 `28x28`，兼顾演示效果和 STM32F103VE 的存储/计算成本。
- STM32 端收到未知命令时回复 `STATUS,state=idle,message=unknown_command`。
