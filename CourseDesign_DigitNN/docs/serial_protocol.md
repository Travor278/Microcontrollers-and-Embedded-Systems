# 串口协议

默认参数：USART1，115200 baud，8N1。

## STM32 到上位机

识别结果：

```text
RESULT,model=<P|F>,label=<0-9>,confidence=<0-100>,time_ms=<n>
```

批量测试统计：

```text
TEST,set=<mnist|personal>,model=<P|F>,total=<n>,correct=<n>,accuracy=<0-10000>,avg_time_us=<n>
```

系统状态：

```text
STATUS,state=<idle|drawing|infer|test>,message=<text>
```

## 上位机到 STM32

```text
CMD,CLEAR
CMD,MODEL,P
CMD,MODEL,F
CMD,TEST,mnist
CMD,TEST,personal
CMD,INFO
```

## 设计原则

- 使用 ASCII 文本帧，串口助手和 Qt 上位机都容易调试。
- 每帧以 `\r\n` 结束。
- 关键字段名固定，便于 PC 端脚本解析。
- STM32 端收到未知命令时回复 `STATUS,state=idle,message=unknown_command`。
