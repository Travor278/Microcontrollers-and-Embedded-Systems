# Host App

`serial_dashboard.py` 是轻量串口监控脚本，用于记录 STM32 输出的 `RESULT`、`TEST`、`STATUS` 帧。

培训 PDF 中建议使用 Qt 5.14 开发串口助手；本脚本先用于快速联调。若要改成 Qt：

- `.pro` 中添加 `QT += serialport`。
- UI 包含 COM 口、波特率、打开串口、发送命令、接收窗口和统计标签。
- 串口接收槽函数按 `docs/serial_protocol.md` 解析帧。
- 界面和窗口标题中备注题目、作者姓名、班级。

运行示例：

```powershell
python host_app\serial_dashboard.py --port COM3 --baud 115200
```
