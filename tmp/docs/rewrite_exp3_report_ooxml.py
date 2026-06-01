from pathlib import Path
import os
import shutil
import tempfile
import zipfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
DOCX_PATH = ROOT / "EX13" / "Paper" / "stm32_exp3_report.docx"

NS = {
    "w": "http://schemas.openxmlformats.org/wordprocessingml/2006/main",
    "xml": "http://www.w3.org/XML/1998/namespace",
}

NAMESPACES = {
    "wpc": "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas",
    "mc": "http://schemas.openxmlformats.org/markup-compatibility/2006",
    "o": "urn:schemas-microsoft-com:office:office",
    "r": "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
    "m": "http://schemas.openxmlformats.org/officeDocument/2006/math",
    "v": "urn:schemas-microsoft-com:vml",
    "wp14": "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing",
    "wp": "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing",
    "w": NS["w"],
    "w14": "http://schemas.microsoft.com/office/word/2010/wordml",
    "w10": "urn:schemas-microsoft-com:office:word",
    "w15": "http://schemas.microsoft.com/office/word/2012/wordml",
    "wpg": "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup",
    "wpi": "http://schemas.microsoft.com/office/word/2010/wordprocessingInk",
    "wne": "http://schemas.microsoft.com/office/word/2006/wordml",
    "wps": "http://schemas.microsoft.com/office/word/2010/wordprocessingShape",
    "wpsCustomData": "http://www.wps.cn/officeDocument/2013/wpsCustomData",
}

for prefix, uri in NAMESPACES.items():
    ET.register_namespace(prefix, uri)


def qn(tag: str) -> str:
    prefix, local = tag.split(":")
    return f"{{{NS[prefix]}}}{local}"


def w_el(name: str, attrs=None, text=None):
    el = ET.Element(qn(f"w:{name}"), attrs or {})
    if text is not None:
        el.text = text
    return el


def make_run(text: str, bold=False, size="21", mono=False):
    run = w_el("r")
    rpr = w_el("rPr")
    if mono:
        fonts = w_el("rFonts", {
            qn("w:ascii"): "Courier New",
            qn("w:hAnsi"): "Courier New",
            qn("w:eastAsia"): "宋体",
        })
    else:
        fonts = w_el("rFonts", {
            qn("w:ascii"): "Times New Roman",
            qn("w:hAnsi"): "Times New Roman",
            qn("w:eastAsia"): "宋体",
        })
    rpr.append(fonts)
    if bold:
        rpr.append(w_el("b"))
    rpr.append(w_el("sz", {qn("w:val"): size}))
    rpr.append(w_el("szCs", {qn("w:val"): size}))
    run.append(rpr)
    t = w_el("t")
    t.text = text
    if text.startswith(" ") or text.endswith(" ") or "  " in text:
        t.set(f"{{{NS['xml']}}}space", "preserve")
    run.append(t)
    return run


def make_para(text: str, kind="normal"):
    p = w_el("p")
    ppr = w_el("pPr")
    spacing = w_el("spacing", {qn("w:after"): "80", qn("w:line"): "276", qn("w:lineRule"): "auto"})
    ppr.append(spacing)
    if kind == "normal":
        ppr.append(w_el("ind", {qn("w:firstLine"): "420"}))
    p.append(ppr)
    if kind == "heading":
        p.append(make_run(text, bold=True, size="23"))
    elif kind == "subheading":
        p.append(make_run(text, bold=True, size="21"))
    elif kind == "code":
        p.append(make_run(text, size="19", mono=True))
    else:
        p.append(make_run(text, size="21"))
    return p


BODY = [
    ("实验名称：STM32实验三 A/D实验", "heading"),
    ("一、实验内容、目的与要求", "heading"),
    ("本实验对应《野火STM32实验任务书-自动化+测控(1)》中的“实验三 A/D实验”。基本要求为参考《STM32库开发实战指南--基于野火指南者开发板》第30章“ADC电压采集”，使用 ADC 采集可变电阻电压，并在串口助手上显示 ADC 原始值和换算电压。本次实验在基本题基础上完成两个提高题：提高题（1）简易火警报警器由 EX113 工程实现，提高题（2）LCD 简易示波器由 EX123 工程实现。", "normal"),
    ("实验目的包括：掌握 STM32F103 片上 ADC 的输入通道、采样时间、连续转换、EOC 转换完成中断、右对齐数据读取和自校准过程；理解 PC1 与 ADC_Channel_11 的对应关系；掌握把 ADC 原始值换算为实际电压的方法；能够把 ADC、串口、GPIO、按键和 LCD 显示组织成完整的测控程序。", "normal"),
    ("提高题（1）的目标是设计简易火警报警器：当模拟传感器电压升高到报警阈值时，板载红色 LED 点亮；当电压降低到解除阈值时，红灯熄灭。提高题（2）的目标是设计 LCD 简易示波器：把 PC1 上的模拟电压实时绘制为曲线，并支持 K1 切换量程、K2 打开或关闭软件滤波。", "normal"),
    ("二、实验硬件与软件环境条件", "heading"),
    ("硬件环境：PC 机、野火 STM32F103 指南者开发板、CMSIS-DAP 仿真器、USB 转串口接口、10 kΩ 可变电阻器或模拟烟雾传感器、板载 RGB LED、3.2 寸 ILI9341 LCD 液晶屏、杜邦线和 USB 供电线。可变电阻两端分别接 3.3V 和 GND，中间滑动端接 PC1。", "normal"),
    ("软件环境：Keil uVision5、ARMCC 工具链、STM32F10x 标准外设库、野火 ADC 电压采集例程、LCD 显示例程、USART 串口例程和野火串口调试助手。实验报告保存于 EX13/Paper，火警报警器工程为 EX113，LCD 示波器工程为 EX123。", "normal"),
    ("主要引脚和外设分配：PC1 对应 ADC_Channel_11，用作模拟电压输入；USART1 通过板载 USB 转串口与上位机通信，波特率 115200；PB5 为板载 RGB 红色通道，低电平点亮；K1 接 PA0，用于示波器量程切换；K2 接 PC13，用于示波器滤波开关；LCD 使用 FSMC 并行接口驱动 ILI9341。", "normal"),
    ("三、实验线路示图、程序算法流程图", "heading"),
    ("1. 实验线路说明：可变电阻左端接 3.3V，右端接 GND，中间滑动端接 PC1。ADC 测量的是 PC1 相对 GND 的电压，因此传感器、电位器和开发板必须共地。若使用烟雾传感器模块，应先确认模拟输出不超过 3.3V，防止损坏 STM32 ADC 输入。", "normal"),
    ("【图片占位：可变电阻/传感器接 PC1、LCD 示波器显示、红灯报警状态照片】", "normal"),
    ("2. ADC 初始化算法：程序先打开 GPIOC 与 ADC2 外设时钟。外部晶振经过 PLL 得到系统时钟，系统时钟再经 APB2 总线分配给 GPIOC 和 ADC2；只有外设时钟开启后，对应模块才工作。随后把 PC1 配置为模拟输入，设置 ADC 时钟为 PCLK2/8，即 72MHz/8=9MHz。这个 9MHz 是 ADC 内部工作时钟，不等于每秒采样 900 万次。由于本程序采样时间为 55.5 个 ADC 周期，再加上约 12.5 个转换周期，一次转换约 68 个 ADC 周期，理论转换频率约为 9MHz/68，约 132k 次/秒。LCD 实际绘图速度远低于该值，只取后台 ADC 连续采样得到的最新值用于显示。", "normal"),
    ("3. ADC 通道配置算法：STM32F103 的 ADC 是 12 位 ADC，转换结果范围为 0~4095。PC1 这个物理引脚在芯片内部对应 ADC_Channel_11，所以需要调用 ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5)。其中 ADC2 表示使用 ADC2 外设，ADC_Channel_11 表示采样 PC1，参数 1 表示把该通道放在规则转换序列的第 1 个位置，最后一个参数表示采样保持时间。", "normal"),
    ("4. 中断采样算法：程序使能 ADC_IT_EOC 转换完成中断。EOC 是 End Of Conversion 的缩写，即一次转换完成。ADC 连续采样时，每采完一次就进入 ADC1_2_IRQHandler()，中断服务函数读取 ADC_GetConversionValue(ADC2)，并把结果保存到全局变量 ADC_ConvertedValue。主循环不必反复查询 ADC 是否采完，只要使用 ADC_ConvertedValue 中保存的最新结果即可。", "normal"),
    ("5. 提高题（1）火警报警器流程：EX113 中主循环把 ADC_ConvertedValue 换算为电压 voltage。若当前未报警且 voltage >= 1.50V，则 alarmState 置 1 并点亮 PB5 红灯；若当前已报警且 voltage <= 1.40V，则 alarmState 清零并关闭红灯。1.50V 与 1.40V 构成迟滞区间，避免电压在 1.5V 附近轻微抖动时红灯频繁闪烁。", "normal"),
    ("6. 提高题（2）LCD 示波器流程：EX123 中 ADC2 在后台连续采样，主循环读取最新 ADC_ConvertedValue，把它换算为 voltage。K1 使量程在 0-3.3V、0-2.0V、0-1.0V 之间循环切换；K2 开关一阶低通滤波。程序使用 ILI9341_GramScan(5) 设置横屏显示，选择 5 是因为该 LCD 使用扫描方向 7 会出现英文左右镜像。曲线绘图区为横屏 320x240 中的 280x160 区域，电压通过 y = y_bottom - voltage * height / rangeMax 映射为纵坐标，相邻采样点用 ILI9341_DrawLine 连接。", "normal"),
    ("关键伪代码：", "subheading"),
    ("ADC_Init_PC1(); LCD_Init(); USART_Config(); Key_GPIO_Config();", "code"),
    ("while (1) {", "code"),
    ("    if (K1 pressed) switch range among 3.3V, 2.0V, 1.0V;", "code"),
    ("    if (K2 pressed) toggle filter;", "code"),
    ("    voltage = ADC_ConvertedValue * 3.3 / 4096;", "code"),
    ("    if (filterEnabled) displayVoltage = 0.9 * oldValue + 0.1 * voltage;", "code"),
    ("    else displayVoltage = voltage;", "code"),
    ("    y = yBottom - displayVoltage * plotHeight / rangeMax;", "code"),
    ("    draw line from previous point to current point;", "code"),
    ("}", "code"),
    ("四、实验调试步骤、实验数据记录及实验结果", "heading"),
    ("1. 基本 ADC 与串口调试：先连接可变电阻，确认 PC1 电压不超过 3.3V。打开串口助手，设置 115200、8 位数据位、1 位停止位、无校验。下载 EX113 或 EX123 后，旋转电位器，串口应能看到 ADC 原始值和换算电压随电位器连续变化。", "normal"),
    ("2. 火警报警器调试：下载 EX113 工程，缓慢升高输入电压。当电压升至 1.50V 及以上时，PB5 红灯点亮，串口状态显示 ALARM；再缓慢降低电压，当电压降至 1.40V 及以下时，红灯熄灭，串口状态显示 NORMAL。若在 1.40V 到 1.50V 之间，系统保持之前状态，这正是迟滞控制的作用。", "normal"),
    ("3. LCD 示波器调试：下载 EX123 工程，LCD 横屏显示 ADC simple oscilloscope。旋转电位器时，曲线随输入电压上升或下降。按 K1 后量程依次切换为 3.3V、2.0V、1.0V；量程越小，同样的电压变化在屏幕上显示得越明显。按 K2 后 Filter 在 ON/OFF 间切换；滤波打开时曲线更平滑，但快速旋转电位器时响应略慢。", "normal"),
    ("理论数据记录表：", "subheading"),
    ("输入电压/V    理论ADC值    火警状态说明                 示波器显示说明", "code"),
    ("0.00          0           NORMAL，红灯灭              曲线在底部", "code"),
    ("0.50          620         NORMAL，红灯灭              低量程下变化更明显", "code"),
    ("1.00          1241        NORMAL，红灯灭              曲线约在 3.3V 量程的 1/3 高度", "code"),
    ("1.40          1737        已报警时解除报警边界        低于 1.5V 阈值线", "code"),
    ("1.50          1861        未报警时进入报警边界        接近报警阈值", "code"),
    ("2.00          2482        ALARM，红灯亮               2.0V 量程下接近顶部", "code"),
    ("3.30          4095        ALARM，红灯亮               3.3V 量程下接近顶部", "code"),
    ("实验结果：EX113 能够在电压超过 1.50V 时点亮红灯报警，并在电压低于 1.40V 后解除报警；EX123 能够在 LCD 上实时绘制 PC1 电压曲线，K1 量程切换和 K2 滤波开关均可正常工作，串口能输出 ADC 值、电压、绘图电压、量程和滤波状态。", "normal"),
    ("五、实验结果分析与误差讨论", "heading"),
    ("1. ADC 量化误差：STM32F103 ADC 为 12 位，理论分辨率为 3.3V/4096，约 0.805mV/LSB。因此即使输入电压稳定，ADC 原始值也可能有 1 个或几个计数的波动。实际误差还会受到 3.3V 参考电压、USB 供电、电位器接触电阻和板上噪声影响。", "normal"),
    ("2. 采样频率与显示频率不同：ADC 后台连续转换速度很高，但 LCD 绘图、串口打印和软件延时会显著降低屏幕曲线刷新速度。示波器显示的是主循环取到的一系列最新 ADC 值，而不是 ADC 每一次转换都画到 LCD 上。这种实现适合观察慢变化模拟量，例如电位器和烟雾传感器电压。", "normal"),
    ("3. 滤波作用：EX123 使用一阶低通滤波，公式为 filtered = 0.9 * old + 0.1 * new。滤波打开后，短时间噪声和抖动被平滑，曲线更稳定；代价是输入电压快速变化时显示会稍微滞后。K2 可以关闭滤波，用未滤波电压直接绘制，便于比较两种显示效果。", "normal"),
    ("4. 量程切换作用：K1 切换的 3.3V、2.0V、1.0V 是显示量程，不改变 ADC 的真实输入范围。ADC 输入仍必须保持 0-3.3V。量程越小，单位电压对应的像素高度越大，低幅度信号更容易观察；若电压超过当前量程，程序会把曲线限制在显示区域顶部。", "normal"),
    ("5. 火警报警迟滞：EX113 未使用电压平滑滤波，而是采用状态迟滞。进入报警阈值为 1.50V，退出报警阈值为 1.40V，中间 0.10V 区间保持原状态。这种方法适合报警器，因为它防止临界点附近的反复跳变，比单一阈值判断更稳定。", "normal"),
    ("六、实验总结与心得", "heading"),
    ("通过本实验，我完成了 STM32 ADC 从采样、换算、串口输出到报警判断和 LCD 曲线显示的完整应用。火警报警器体现了 ADC 与 GPIO 输出的组合应用，LCD 示波器体现了 ADC、按键、LCD 绘图和滤波算法的综合应用。实验中最关键的理解点是：PC1 是物理引脚，ADC_Channel_11 是 ADC 内部通道；PCLK2/8 得到的是 ADC 工作时钟，不等于屏幕刷新频率；EOC 中断用于后台更新最新采样值；右对齐使 12 位结果可直接按 0~4095 使用。经过本实验，对 STM32 模拟量采集和嵌入式测控程序的结构有了更清晰的认识。", "normal"),
]


def replace_body_cell(document_xml: bytes) -> bytes:
    root = ET.fromstring(document_xml)
    body = root.find("w:body", NS)
    tables = body.findall("w:tbl", NS)
    if len(tables) < 2:
        raise RuntimeError("document.xml 中未找到模板正文表格")
    table = tables[1]
    row = table.find("w:tr", NS)
    cell = row.find("w:tc", NS)
    tc_pr = cell.find("w:tcPr", NS)
    for child in list(cell):
        cell.remove(child)
    if tc_pr is not None:
        cell.append(tc_pr)
    for text, kind in BODY:
        cell.append(make_para(text, kind))
    return ET.tostring(root, encoding="utf-8", xml_declaration=True, short_empty_elements=True)


def main():
    if not DOCX_PATH.exists():
        raise FileNotFoundError(DOCX_PATH)

    fd, tmp_name = tempfile.mkstemp(suffix=".docx", dir=str(DOCX_PATH.parent))
    os.close(fd)
    tmp_path = Path(tmp_name)

    try:
        with zipfile.ZipFile(DOCX_PATH, "r") as zin, zipfile.ZipFile(tmp_path, "w", zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == "word/document.xml":
                    data = replace_body_cell(data)
                zout.writestr(item, data)
        shutil.move(str(tmp_path), str(DOCX_PATH))
    finally:
        if tmp_path.exists():
            tmp_path.unlink()


if __name__ == "__main__":
    main()
