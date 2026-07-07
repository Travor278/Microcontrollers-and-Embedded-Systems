const state = {
  labels: [],
  models: [],
  drawing: false,
  lastPoint: null,
  letterDrawing: false,
  lastLetterPoint: null,
  chineseDrawing: false,
  lastChinesePoint: null,
  boardLastPoint: null,
  boardPointCount: 0,
  boardStrokeCount: 0,
  lastBoardImage: null,
  boardImageSaved: false,
  boardAutoSaveTimer: null,
  boardSourceSyncTimer: null,
  boardResizeTimer: null,
  usagePollTimer: null,
  lastMcuResults: {},
  pixelWidth: 28,
  pixelHeight: 28,
  inferTimer: null,
  serialPort: null,
  serialReader: null,
  serialBuffer: "",
  serialConnected: false,
  serialPollTimer: null,
  serialLastId: 0,
  serialPorts: [],
  deployBusy: false,
  lang: localStorage.getItem("digitnn-lang") || "zh",
  currentView: localStorage.getItem("digitnn-view") || "home",
  sourceMode: localStorage.getItem("digitnn-source-mode") || "mcu",
  letterSourceMode: localStorage.getItem("digitnn-letter-source-mode") || "browser",
  chineseSourceMode: localStorage.getItem("digitnn-chinese-source-mode") || "browser",
  modelExplain: localStorage.getItem("digitnn-model-explain") || "fnn",
  quantizationProfile: null,
};

const els = {
  homeView: document.getElementById("homeView"),
  workspaceView: document.getElementById("workspaceView"),
  batchTestView: document.getElementById("batchTestView"),
  tfCardView: document.getElementById("tfCardView"),
  lettersView: document.getElementById("lettersView"),
  chineseView: document.getElementById("chineseView"),
  homeNavBtn: document.getElementById("homeNavBtn"),
  workspaceNavBtn: document.getElementById("workspaceNavBtn"),
  openWorkspaceBtn: document.getElementById("openWorkspaceBtn"),
  homeRefreshBtn: document.getElementById("homeRefreshBtn"),
  drawCanvas: document.getElementById("drawCanvas"),
  boardCanvas: document.getElementById("boardCanvas"),
  boardStats: document.getElementById("boardStats"),
  pixelGrid: document.getElementById("pixelGrid"),
  pixelStats: document.getElementById("pixelStats"),
  pcResults: document.getElementById("pcResults"),
  mcuResults: document.getElementById("mcuResults"),
  labelSelect: document.getElementById("labelSelect"),
  brushRange: document.getElementById("brushRange"),
  deskewCheck: document.getElementById("deskewCheck"),
  thickenCheck: document.getElementById("thickenCheck"),
  inferBtn: document.getElementById("inferBtn"),
  clearBtn: document.getElementById("clearBtn"),
  clearBoardBtn: document.getElementById("clearBoardBtn"),
  saveBoardBtn: document.getElementById("saveBoardBtn"),
  autoSaveBoardCheck: document.getElementById("autoSaveBoardCheck"),
  letterSaveBoardBtn: document.getElementById("letterSaveBoardBtn"),
  letterAutoSaveBoardCheck: document.getElementById("letterAutoSaveBoardCheck"),
  penBtn: document.getElementById("penBtn"),
  saveBtn: document.getElementById("saveBtn"),
  saveDirInput: document.getElementById("saveDirInput"),
  modelStatus: document.getElementById("modelStatus"),
  languageBtn: document.getElementById("languageBtn"),
  refreshStatusBtn: document.getElementById("refreshStatusBtn"),
  flashMeter: document.getElementById("flashMeter"),
  sramMeter: document.getElementById("sramMeter"),
  flashText: document.getElementById("flashText"),
  sramText: document.getElementById("sramText"),
  homeFlashText: document.getElementById("homeFlashText"),
  homeSramText: document.getElementById("homeSramText"),
  floatParamText: document.getElementById("floatParamText"),
  int8ParamText: document.getElementById("int8ParamText"),
  floatParamBar: document.getElementById("floatParamBar"),
  int8ParamBar: document.getElementById("int8ParamBar"),
  floatFirmwareText: document.getElementById("floatFirmwareText"),
  floatFirmwareBar: document.getElementById("floatFirmwareBar"),
  quantFlashText: document.getElementById("quantFlashText"),
  quantSramText: document.getElementById("quantSramText"),
  quantFlashBar: document.getElementById("quantFlashBar"),
  quantSramBar: document.getElementById("quantSramBar"),
  modelStorageText: document.getElementById("modelStorageText"),
  quantCompressionText: document.getElementById("quantCompressionText"),
  quantSavingText: document.getElementById("quantSavingText"),
  quantModelStrip: document.getElementById("quantModelStrip"),
  quantFlowRatio: document.getElementById("quantFlowRatio"),
  quantDomainText: document.getElementById("quantDomainText"),
  mcuModeBtn: document.getElementById("mcuModeBtn"),
  inputModeBtn: document.getElementById("inputModeBtn"),
  mcuPane: document.getElementById("mcuPane"),
  inputPane: document.getElementById("inputPane"),
  serialPortSelect: document.getElementById("serialPortSelect"),
  refreshPortsBtn: document.getElementById("refreshPortsBtn"),
  serialBtn: document.getElementById("serialBtn"),
  serialLog: document.getElementById("serialLog"),
  sharedSerialSelects: Array.from(document.querySelectorAll(".shared-serial-select")),
  sharedSerialButtons: Array.from(document.querySelectorAll(".shared-serial-btn")),
  sharedRefreshPortsButtons: Array.from(document.querySelectorAll(".shared-refresh-ports-btn")),
  sharedClearBoardButtons: Array.from(document.querySelectorAll(".shared-clear-board-btn")),
  sharedBoardCanvases: Array.from(document.querySelectorAll(".shared-board-canvas")),
  sharedBoardStats: Array.from(document.querySelectorAll(".shared-board-stats")),
  copyBoardToLetterBtn: document.getElementById("copyBoardToLetterBtn"),
  copyBoardToChineseBtn: document.getElementById("copyBoardToChineseBtn"),
  deployModel: document.getElementById("deployModel"),
  epochsInput: document.getElementById("epochsInput"),
  batchInput: document.getElementById("batchInput"),
  uv4Input: document.getElementById("uv4Input"),
  augmentCheck: document.getElementById("augmentCheck"),
  batchModelSelect: document.getElementById("batchModelSelect"),
  runDigitBatchTestBtn: document.getElementById("runDigitBatchTestBtn"),
  runLetterBatchTestBtn: document.getElementById("runLetterBatchTestBtn"),
  tfCardPathInput: document.getElementById("tfCardPathInput"),
  refreshTfCardBtn: document.getElementById("refreshTfCardBtn"),
  refreshTfCardBtn2: document.getElementById("refreshTfCardBtn2"),
  tfPcStatus: document.getElementById("tfPcStatus"),
  tfMcuStatus: document.getElementById("tfMcuStatus"),
  tfDriveText: document.getElementById("tfDriveText"),
  tfImageText: document.getElementById("tfImageText"),
  tfLabelText: document.getElementById("tfLabelText"),
  tfSourceText: document.getElementById("tfSourceText"),
  tfPathList: document.getElementById("tfPathList"),
  tfDatasetBody: document.getElementById("tfDatasetBody"),
  tfMcuMessage: document.getElementById("tfMcuMessage"),
  tfSerialStatus: document.getElementById("tfSerialStatus"),
  tfEvidenceLog: document.getElementById("tfEvidenceLog"),
  batchResultBody: document.getElementById("batchResultBody"),
  confusionResultBody: document.getElementById("confusionResultBody"),
  standardAccuracyText: document.getElementById("standardAccuracyText"),
  standardTimeText: document.getElementById("standardTimeText"),
  personalAccuracyText: document.getElementById("personalAccuracyText"),
  personalTimeText: document.getElementById("personalTimeText"),
  letterCanvas: document.getElementById("letterCanvas"),
  letterLabelSelect: document.getElementById("letterLabelSelect"),
  letterBrushRange: document.getElementById("letterBrushRange"),
  clearLetterBtn: document.getElementById("clearLetterBtn"),
  saveLetterBtn: document.getElementById("saveLetterBtn"),
  letterSaveDirInput: document.getElementById("letterSaveDirInput"),
  letterPixelGrid: document.getElementById("letterPixelGrid"),
  letterPixelStats: document.getElementById("letterPixelStats"),
  letterResults: document.getElementById("letterResults"),
  letterDeployModel: document.getElementById("letterDeployModel"),
  letterEpochsInput: document.getElementById("letterEpochsInput"),
  letterBatchInput: document.getElementById("letterBatchInput"),
  letterUv4Input: document.getElementById("letterUv4Input"),
  letterAugmentCheck: document.getElementById("letterAugmentCheck"),
  letterFlashText: document.getElementById("letterFlashText"),
  letterSramText: document.getElementById("letterSramText"),
  letterFlashBar: document.getElementById("letterFlashBar"),
  letterSramBar: document.getElementById("letterSramBar"),
  letterSerialLog: document.getElementById("letterSerialLog"),
  chineseCanvas: document.getElementById("chineseCanvas"),
  chinesePreviewCanvas: document.getElementById("chinesePreviewCanvas"),
  chinesePreviewStats: document.getElementById("chinesePreviewStats"),
  runChineseBtn: document.getElementById("runChineseBtn"),
  clearChineseBtn: document.getElementById("clearChineseBtn"),
  probeChineseModelsBtn: document.getElementById("probeChineseModelsBtn"),
  refreshChineseStatusBtn: document.getElementById("refreshChineseStatusBtn"),
  chineseProviderSelect: document.getElementById("chineseProviderSelect"),
  chineseModelInput: document.getElementById("chineseModelInput"),
  chinesePromptInput: document.getElementById("chinesePromptInput"),
  chineseApiStatus: document.getElementById("chineseApiStatus"),
  chineseRuntimeStatus: document.getElementById("chineseRuntimeStatus"),
  chineseRemoteText: document.getElementById("chineseRemoteText"),
  chineseBaseUrlText: document.getElementById("chineseBaseUrlText"),
  chineseTokenNameText: document.getElementById("chineseTokenNameText"),
  chineseRuntimeGrid: document.getElementById("chineseRuntimeGrid"),
  chineseResultText: document.getElementById("chineseResultText"),
  chineseResultMeta: document.getElementById("chineseResultMeta"),
  chineseConfidenceBar: document.getElementById("chineseConfidenceBar"),
  chineseCandidateList: document.getElementById("chineseCandidateList"),
  chineseFlashText: document.getElementById("chineseFlashText"),
  chineseSramText: document.getElementById("chineseSramText"),
  chineseFlashBar: document.getElementById("chineseFlashBar"),
  chineseSramBar: document.getElementById("chineseSramBar"),
  chineseLog: document.getElementById("chineseLog"),
};

const ctx = els.drawCanvas.getContext("2d", { willReadFrequently: true });
const boardCtx = els.boardCanvas.getContext("2d", { willReadFrequently: true });
const sharedBoardContexts = els.sharedBoardCanvases.map((canvas) => ({
  canvas,
  context: canvas.getContext("2d", { willReadFrequently: true }),
}));
const boardInkCanvas = document.createElement("canvas");
const boardInkCtx = boardInkCanvas.getContext("2d", { willReadFrequently: true });
const letterCtx = els.letterCanvas.getContext("2d", { willReadFrequently: true });
const chineseCtx = els.chineseCanvas.getContext("2d", { willReadFrequently: true });
const chinesePreviewCtx = els.chinesePreviewCanvas.getContext("2d", { willReadFrequently: true });
const LETTER_LABELS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ".split("");
const BOARD_LCD = {
  left: 81,
  top: 0,
  right: 320,
  bottom: 240,
};

const i18n = {
  zh: {
    appTitle: "DigitNN 上位机",
    subtitle: "STM32F103VE 手写识别课程设计工作台",
    modelsLoading: "模型加载中",
    refresh: "刷新",
    home: "首页",
    workspace: "工作区",
    batchTest: "自动化测试",
    letters: "字母工作区",
    heroEyebrow: "课程设计展示",
    heroTitle: "STM32 手写数字识别系统",
    heroText: "覆盖触摸书写、28 x 28 预处理、量化神经网络推理、串口回传、测试集采集与 Keil 固件烧录的完整闭环。",
    openWorkspace: "进入工作区",
    metricBoard: "目标板",
    metricModels: "模型组合",
    pfcTitle: "P / F / C 含义",
    modelMeaningTitle: "三种轻量识别器，对应同一块 STM32",
    modelMeaningText: "首页用于答辩展示：用感知机、全连接网络、Tiny-CNN 和 DS-CNN 解释速度、准确率、Flash/SRAM 占用之间的取舍。",
    perceptronTitle: "感知机",
    fnnTitle: "全连接网络",
    cnnTitle: "Tiny-CNN",
    dsCnnTitle: "DS-CNN",
    pfcP: " 感知机：线性基线模型，体积最小，便于和神经网络效果对比。",
    pfcF: " 全连接网络：当前主力数字识别模型，速度和准确率比较均衡。",
    pfcC: " Tiny-CNN：轻量卷积模型，用来提取笔画形状和局部结构特征。",
    pfcD: " DS-CNN：深度可分离卷积，先逐通道提取笔画，再用 1x1 卷积混合通道；字母测试集上优于普通 Tiny-CNN。",
    resourceCompareTitle: "Flash / SRAM 与量化对比",
    quantVisualTitle: "FP32 权重变成 int8 表",
    floatBits: "32 位",
    intBits: "8 位",
    scalePending: "等待 scale",
    quantStep1Title: "测量范围",
    quantStep1Text: "找到每层权重绝对值最大值，用它推导一个缩放因子。",
    quantStep2Title: "权重取整",
    quantStep2Text: "把浮点权重除以 scale，四舍五入，再裁剪到 [-127, 127]。",
    quantStep3Title: "偏置换算",
    quantStep3Text: "偏置保存为 int32，让整数累加时仍保留原来的偏移量。",
    quantStep4Title: "整数乘加",
    quantStep4Text: "导出 C 数组后，STM32 用 int32 乘加推理，层间用右移控制尺度。",
    quantizedBuild: "量化后固件",
    floatEstimate: "浮点模型估算",
    int8Estimate: "量化模型估算",
    floatFirmwareEstimate: "未量化固件估算",
    quantizationNote: "板端固件使用 int8 权重量化、int32 偏置和整数累加。未量化固件为按实际参数差值推算；若要真实烧录 float 版本，还需要新增 float 推理核心。",
    firmwareFlowTitle: "固件与模型烧录流程",
    firmwareFlowExport: "导出：把选定模型的权重重新生成到 C 源文件。",
    firmwareFlowBuild: "构建：调用 Keil UV4 编译工程，生成 DigitNN_Touch.axf。",
    firmwareFlowFlash: "烧录：通过 CMSIS-DAP 下载 AXF；网页会先释放串口，烧录后再重连。",
    workflowTitle: "演示流程",
    workflowStep1: "在触摸屏或浏览器画布上写数字。",
    workflowStep2: "预处理成 28 x 28 灰度模型输入。",
    workflowStep3: "对比上位机仿真结果和 STM32 串口结果。",
    workflowStep4: "保存带标签样本，再更新并烧录模型。",
    quantFlashCaption: "量化后完整固件在 512 KB Flash 中的实际占用。",
    floatFlashCaption: "把同一批 weight/bias 按 float32 保存时的参数体积。",
    int8FlashCaption: "同一批权重按 int8 存储，并保留 int32 偏置。",
    floatFirmwareCaption: "当前量化固件加上 float32 参数差值；这是估算，不是已烧录版本。",
    quantSramCaption: "当前程序运行时在 64 KB SRAM 中的实际占用。",
    modelStorageTitle: "模型参数存储",
    modelStorageText: "int8 参数约 63 KB，float 参数约 247 KB；仅模型参数就节省约 184 KB。",
    modelStorageDynamic: "float32 参数 {floatSize}，int8/int32 参数 {int8Size}，模型参数节省 {savedSize}。",
    compressionDynamic: "压缩 {ratio}x",
    compressionRatio: "{ratio}x",
    savingDynamic: "节省 {percent}%",
    quantProfileUnavailable: "等待模型文件或 Keil 构建结果",
    activeFirmwareDomain: "当前固件：{domain} P/F/C",
    source: "来源",
    sourceHint: "左侧切换真实单片机回传或浏览器画板输入。",
    mcuMode: "单片机",
    inputMode: "输入",
    allModels: "全部模型",
    batchTestTitle: "自动化批量测试",
    batchTestIntro: "分别对数字和字母测试集逐张推理，并与 label.txt 中的真实标签比对。",
    runBatchTest: "开始批量测试",
    runDigitBatchTest: "数字检测",
    runLetterBatchTest: "字母检测",
    digitTestSets: "数字测试集",
    letterTestSets: "字母测试集",
    standardSet: "标准测试集",
    personalSet: "个人测试集",
    standardAccuracy: "标准集准确率",
    personalAccuracy: "个人集准确率",
    testDetails: "测试明细",
    confusionTitle: "易混淆字符",
    confusionPair: "真实 -> 预测",
    count: "次数",
    examples: "样例",
    noConfusions: "运行批量测试后显示混淆数字对。",
    dataset: "测试集",
    samples: "样本数",
    correct: "正确数",
    accuracy: "准确率",
    avgTime: "平均耗时",
    testEmpty: "点击开始批量测试后显示结果。",
    testRunning: "批量测试执行中",
    testFinished: "批量测试完成",
    letterWorkspaceTitle: "字母识别工作区",
    letterWorkspaceHint: "先准备 A-Z 样本采集和后续字母模型对比。",
    letterModelPending: "字母模型待训练",
    saveLetterSample: "保存字母样本",
    letterPlanTitle: "字母模型计划",
    letterFnnTitle: "Letter-FNN",
    letterFnnText: "一层隐藏层学习字母笔画组合，是字母固件里的主力全连接模型。",
    letterCnnTitle: "Letter-DS-CNN",
    letterCnnText: "深度可分离卷积模型，用 depthwise 提取每个通道的局部笔画，再用 pointwise 混合通道特征。",
    letterDsCnnTitle: "Letter-Perceptron",
    letterDsCnnText: "线性基线，体积最小，便于和神经网络模型做速度和精度对照。",
    letterPlan1: "使用同一套 28 x 28 预处理流程采集 A-Z 样本。",
    letterPlan2: "使用 EMNIST Letters 训练 Letter-Perceptron、Letter-FNN 和 Letter-DS-CNN。",
    letterPlan3: "导出完整字母 P/F/C 后可直接构建并烧录字母固件。",
    letterCanvasCleared: "字母画布已清空",
    letterSampleSaved: "字母样本已保存",
    input: "输入",
    pen: "画笔",
    clear: "清空",
    label: "标签",
    brush: "笔宽",
    autoDeskew: "自动校正",
    thicken: "加粗",
    recognize: "识别",
    saveSample: "保存样本",
    saveDirectory: "保存目录",
    modelInput: "模型输入",
    mcu: "单片机",
    connectSerial: "连接串口",
    disconnect: "断开串口",
    firmwareDeploy: "固件构建与烧录",
    deployModelHint: "数字固件只包含数字 P/F/C。已有权重可直接构建或烧录；只有点“导出”或“导出+烧录”才会重新训练并更新权重。",
    model: "模型",
    epochs: "轮数",
    batch: "批量",
    augment: "数据增强",
    exportModel: "导出",
    build: "构建",
    flash: "烧录",
    exportFlash: "导出+烧录",
    serialLog: "串口日志",
    serialPort: "串口",
    refreshPorts: "刷新串口",
    boardLive: "板端实时轨迹",
    clearBoardView: "清轨迹",
    saveBoardSample: "保存板端样本",
    autoSaveBoard: "REC 后自动保存",
    boardStats: "{points} 点 / {strokes} 笔 / 最后 {x},{y}",
    idle: "等待",
    missing: "缺失",
    mcuResult: "板端",
    canvasCleared: "画布已清空",
    inferenceUpdated: "识别已更新",
    sampleSaved: "样本已保存",
    boardSampleSaved: "板端样本已保存",
    savedFile: "已保存 {filename}",
    noBoardImage: "还没有收到板端 IMAGE 帧，请先在板上点击 REC",
    modelsReady: "{ready}/{total} 个模型就绪",
    webSerialUnavailable: "当前浏览器不支持 Web Serial",
    serialClosed: "串口已关闭",
    serialOpened: "串口已打开",
    serialOpenedWithPort: "串口已打开：{port}",
    serialReleasedForFlash: "烧录前已释放串口：{port}",
    serialReconnectAfterFlash: "烧录后重新连接串口：{port}",
    serialPortsLoaded: "已发现 {count} 个串口",
    noSerialPorts: "未发现串口，请检查 USB TO UART",
    deployAlreadyRunning: "部署任务正在运行",
    deployRunning: "{action} 执行中",
    deployFinished: "{action} 已完成",
    deployFailed: "{action} 失败",
    requestFailed: "请求失败：{status}",
    alt: "备选 {label}",
    activePixels: "{active} 点，最大 {max}",
    boardViewCleared: "板端轨迹已清空",
    boardImageUpdated: "板端图像已同步",
  },
  en: {
    appTitle: "DigitNN Dashboard",
    subtitle: "STM32F103VE handwritten recognition lab console",
    modelsLoading: "Models loading",
    refresh: "Refresh",
    home: "Home",
    workspace: "Workspace",
    batchTest: "Batch Test",
    letters: "Letters",
    heroEyebrow: "Course Design Showcase",
    heroTitle: "STM32 Handwritten Digit Recognition",
    heroText: "A complete loop from touch drawing, 28 x 28 preprocessing, quantized neural-network inference, serial telemetry, dataset collection, and Keil firmware deployment.",
    openWorkspace: "Open Workspace",
    metricBoard: "Target board",
    metricModels: "Models",
    pfcTitle: "P / F / C Meaning",
    modelMeaningTitle: "Three lightweight recognizers, one STM32 target",
    modelMeaningText: "The homepage explains speed, accuracy, and Flash/SRAM tradeoffs through a linear baseline, an FNN, a Tiny-CNN, and a DS-CNN.",
    perceptronTitle: "Perceptron",
    fnnTitle: "FNN",
    cnnTitle: "Tiny-CNN",
    dsCnnTitle: "DS-CNN",
    pfcP: " Perceptron: a linear baseline with the smallest code size.",
    pfcF: " FNN: a fully connected neural network and the current main digit model.",
    pfcC: " Tiny-CNN: a compact convolution model for stroke-shape features.",
    pfcD: " DS-CNN: depthwise separable convolution; depthwise extracts per-channel strokes and 1x1 pointwise mixes channels. It outperforms the regular Tiny-CNN on the letter test set.",
    resourceCompareTitle: "Flash / SRAM and Quantization",
    quantVisualTitle: "FP32 weights become int8 tables",
    floatBits: "32 bit",
    intBits: "8 bit",
    scalePending: "scale pending",
    quantStep1Title: "Measure range",
    quantStep1Text: "Find each layer's max absolute weight and derive one scale.",
    quantStep2Title: "Round weights",
    quantStep2Text: "Divide float weights by scale, round, then clip to [-127, 127].",
    quantStep3Title: "Convert bias",
    quantStep3Text: "Bias is stored as int32 so integer accumulation keeps the original offset.",
    quantStep4Title: "Run integer MAC",
    quantStep4Text: "The firmware exports C arrays and uses int32 multiply-accumulate, with shifts between layers.",
    quantizedBuild: "Quantized firmware",
    floatEstimate: "Float model estimate",
    int8Estimate: "Quantized model estimate",
    floatFirmwareEstimate: "Float firmware estimate",
    quantizationNote: "The board firmware uses int8 weights, int32 biases, and integer accumulation. The float firmware number is estimated from the actual parameter delta; a real float flash would also need a float inference core.",
    firmwareFlowTitle: "Firmware and Model Flow",
    firmwareFlowExport: "Export writes the selected model weights into C source files.",
    firmwareFlowBuild: "Build invokes Keil UV4 to compile the firmware and produce DigitNN_Touch.axf.",
    firmwareFlowFlash: "Flash downloads that AXF through CMSIS-DAP; serial is released first and reconnected afterward.",
    workflowTitle: "Demonstration Workflow",
    workflowStep1: "Write on the touch screen or browser canvas.",
    workflowStep2: "Preprocess to a 28 x 28 grayscale model input.",
    workflowStep3: "Compare PC simulation with STM32 serial results.",
    workflowStep4: "Save labeled samples, then update and flash the model.",
    quantFlashCaption: "Actual complete quantized firmware in 512 KB Flash.",
    floatFlashCaption: "The same weight/bias arrays stored as float32 parameters.",
    int8FlashCaption: "The same weights stored as int8 with int32 biases.",
    floatFirmwareCaption: "Current quantized firmware plus the float32 parameter delta; this is an estimate, not a flashed build.",
    quantSramCaption: "Current runtime memory in 64 KB SRAM.",
    modelStorageTitle: "Model storage",
    modelStorageText: "int8 parameters are about 63 KB; float parameters are about 247 KB, saving about 184 KB before application code.",
    modelStorageDynamic: "float32 params {floatSize}; int8/int32 params {int8Size}; model storage saved {savedSize}.",
    compressionDynamic: "{ratio}x smaller",
    compressionRatio: "{ratio}x",
    savingDynamic: "{percent}% saved",
    quantProfileUnavailable: "Waiting for model files or Keil build usage",
    activeFirmwareDomain: "Active firmware: {domain} P/F/C",
    source: "Source",
    sourceHint: "Switch the left panel between the real MCU stream and browser drawing input.",
    mcuMode: "MCU",
    inputMode: "Input",
    allModels: "All models",
    batchTestTitle: "Automated Batch Test",
    batchTestIntro: "Run PC-side inference over digit or letter test sets, then compare predictions with label.txt.",
    runBatchTest: "Run Batch Test",
    runDigitBatchTest: "Digit Test",
    runLetterBatchTest: "Letter Test",
    digitTestSets: "Digit sets",
    letterTestSets: "Letter sets",
    standardSet: "Standard set",
    personalSet: "Personal set",
    standardAccuracy: "Standard Accuracy",
    personalAccuracy: "Personal Accuracy",
    testDetails: "Result Details",
    confusionTitle: "Most Confused Characters",
    confusionPair: "True -> Pred",
    count: "Count",
    examples: "Examples",
    noConfusions: "Run a batch test to show confusion pairs.",
    dataset: "Dataset",
    samples: "Samples",
    correct: "Correct",
    accuracy: "Accuracy",
    avgTime: "Avg Time",
    testEmpty: "Run a batch test to fill this table.",
    testRunning: "Batch test running",
    testFinished: "Batch test finished",
    letterWorkspaceTitle: "Letter Recognition Workspace",
    letterWorkspaceHint: "Prepare A-Z sample capture and future letter model comparison.",
    letterModelPending: "Letter model pending",
    saveLetterSample: "Save Letter Sample",
    letterPlanTitle: "Letter Model Plan",
    letterFnnTitle: "Letter-FNN",
    letterFnnText: "One hidden layer learns letter stroke combinations for the main embedded fully connected model.",
    letterCnnTitle: "Letter-DS-CNN",
    letterCnnText: "Depthwise separable CNN: depthwise filters local strokes per channel, then 1x1 pointwise convolution mixes channels.",
    letterDsCnnTitle: "Letter-Perceptron",
    letterDsCnnText: "Linear baseline with the smallest footprint for speed and accuracy comparison.",
    letterPlan1: "Collect A-Z samples with the same 28 x 28 preprocessing path.",
    letterPlan2: "Train Letter-Perceptron, Letter-FNN, and Letter-DS-CNN from EMNIST Letters.",
    letterPlan3: "Export the full letter P/F/C set, then build and flash the letter firmware.",
    letterCanvasCleared: "Letter canvas cleared",
    letterSampleSaved: "Letter sample saved",
    input: "Input",
    pen: "Pen",
    clear: "Clear",
    label: "Label",
    brush: "Brush",
    autoDeskew: "Auto deskew",
    thicken: "Thicken",
    recognize: "Recognize",
    saveSample: "Save Sample",
    saveDirectory: "Save directory",
    modelInput: "Model Input",
    mcu: "MCU",
    connectSerial: "Connect Serial",
    disconnect: "Disconnect",
    firmwareDeploy: "Firmware Deploy",
    deployModelHint: "Digit firmware contains only digit P/F/C. Build or Flash can reuse existing exported weights; Export and Export+Flash retrain/update weights first.",
    model: "Model",
    epochs: "Epochs",
    batch: "Batch",
    augment: "Augment",
    exportModel: "Export",
    build: "Build",
    flash: "Flash",
    exportFlash: "Export+Flash",
    serialLog: "Serial Log",
    serialPort: "Serial Port",
    refreshPorts: "Refresh Ports",
    boardLive: "Board Live",
    clearBoardView: "Clear View",
    saveBoardSample: "Save Board Sample",
    autoSaveBoard: "Auto save after REC",
    boardStats: "{points} pts / {strokes} strokes / last {x},{y}",
    idle: "idle",
    missing: "missing",
    mcuResult: "MCU",
    canvasCleared: "Canvas cleared",
    inferenceUpdated: "Inference updated",
    sampleSaved: "Sample saved",
    boardSampleSaved: "Board sample saved",
    savedFile: "saved {filename}",
    noBoardImage: "No board IMAGE frame yet; press REC on the board first",
    modelsReady: "{ready}/{total} models ready",
    webSerialUnavailable: "Web Serial is unavailable in this browser",
    serialClosed: "serial closed",
    serialOpened: "serial opened",
    serialOpenedWithPort: "serial opened: {port}",
    serialReleasedForFlash: "serial released before flash: {port}",
    serialReconnectAfterFlash: "serial reconnecting after flash: {port}",
    serialPortsLoaded: "{count} serial ports found",
    noSerialPorts: "no serial ports found; check USB TO UART",
    deployAlreadyRunning: "Deploy task is already running",
    deployRunning: "{action} running",
    deployFinished: "{action} finished",
    deployFailed: "{action} failed",
    requestFailed: "request failed: {status}",
    alt: "alt {label}",
    activePixels: "{active} px, max {max}",
    boardViewCleared: "Board view cleared",
    boardImageUpdated: "Board image synced",
  },
};

function t(key, values = {}) {
  const table = i18n[state.lang] || i18n.zh;
  const fallback = i18n.en[key] || key;
  const template = table[key] || fallback;
  return template.replace(/\{(\w+)\}/g, (_match, name) => (values[name] ?? "").toString());
}

function applyLanguage() {
  document.documentElement.lang = state.lang === "zh" ? "zh-CN" : "en";
  for (const element of document.querySelectorAll("[data-i18n]")) {
    element.textContent = t(element.dataset.i18n);
  }
  els.languageBtn.textContent = state.lang === "zh" ? "EN" : "中文";
  syncSerialUi();
}

function toggleLanguage() {
  state.lang = state.lang === "zh" ? "en" : "zh";
  localStorage.setItem("digitnn-lang", state.lang);
  applyLanguage();
  renderModelTiles(state.models);
  renderLetterModelTiles();
  renderQuantizationProfile();
  setStatus(t("modelsReady", { ready: state.models.filter((item) => item.available).length, total: state.models.length }));
  runInfer();
}

function applyView() {
  const validViews = new Set(["home", "workspace", "batchTest", "tfCard", "letters", "chinese"]);
  const view = validViews.has(state.currentView) ? state.currentView : "home";
  state.currentView = view;
  for (const panel of document.querySelectorAll(".view")) {
    panel.classList.toggle("active", panel.id === `${view}View`);
  }
  for (const button of document.querySelectorAll("[data-view]")) {
    button.classList.toggle("active", button.dataset.view === view);
  }
}

function setView(view) {
  state.currentView = view;
  localStorage.setItem("digitnn-view", state.currentView);
  applyView();
  if (
    (state.currentView === "letters" && isMcuSourceMode("letter")) ||
    (state.currentView === "chinese" && isMcuSourceMode("chinese"))
  ) {
    initBoardCanvas();
  }
  if (state.currentView === "chinese") {
    initChineseCanvasIfBlank();
    loadChineseStatus();
  }
  if (state.currentView === "tfCard") {
    refreshTfCardStatus();
  }
}

function applySourceMode() {
  const mode = state.sourceMode === "input" ? "input" : "mcu";
  state.sourceMode = mode;
  const showMcu = mode === "mcu";
  els.mcuPane.classList.toggle("active", showMcu);
  els.inputPane.classList.toggle("active", !showMcu);
  els.mcuModeBtn.classList.toggle("active", showMcu);
  els.inputModeBtn.classList.toggle("active", !showMcu);
}

function setSourceMode(mode) {
  state.sourceMode = mode === "input" ? "input" : "mcu";
  localStorage.setItem("digitnn-source-mode", state.sourceMode);
  applySourceMode();
  if (state.sourceMode === "mcu") {
    scheduleBoardCanvasInit();
  }
}

function applyModelExplain() {
  const valid = new Set(["perceptron", "fnn", "cnn", "dscnn"]);
  const model = valid.has(state.modelExplain) ? state.modelExplain : "fnn";
  state.modelExplain = model;
  for (const pane of document.querySelectorAll("[data-model-visual]")) {
    pane.classList.toggle("active", pane.dataset.modelVisual === model);
  }
  for (const option of document.querySelectorAll("[data-model-explain]")) {
    option.classList.toggle("active", option.dataset.modelExplain === model);
    option.setAttribute("aria-pressed", option.dataset.modelExplain === model ? "true" : "false");
  }
}

function setModelExplain(model) {
  state.modelExplain = model;
  localStorage.setItem("digitnn-model-explain", state.modelExplain);
  applyModelExplain();
}

function appendLog(target, text) {
  if (!target) return;
  const prefix = new Date().toLocaleTimeString();
  target.textContent += `[${prefix}] ${text}\n`;
  target.scrollTop = target.scrollHeight;
}

function logLine(text, kind = "") {
  appendLog(els.serialLog, text);
  if (kind === "error") {
    els.modelStatus.textContent = text;
  }
}

function logSharedSerialLine(text, kind = "") {
  logLine(text, kind);
  appendLog(els.letterSerialLog, text);
}

function setStatus(text) {
  els.modelStatus.textContent = text;
}

function setSelectValueIfPresent(select, value) {
  if (!select || !value) return;
  if ([...select.options].some((option) => option.value === value)) {
    select.value = value;
  }
}

function fillSerialSelect(select, ports, selected) {
  if (!select) return;
  select.innerHTML = "";
  for (const port of ports) {
    const option = document.createElement("option");
    option.value = port.device;
    option.textContent = port.label || port.device;
    select.appendChild(option);
  }
  setSelectValueIfPresent(select, selected);
}

function expandSerialPorts(ports) {
  const merged = [];
  const seen = new Set();
  for (const port of ports || []) {
    const device = String(port.device || "").trim();
    if (!device || seen.has(device.toUpperCase())) continue;
    if (port.detected === false || String(port.hwid || "").toLowerCase() === "manual") continue;
    seen.add(device.toUpperCase());
    merged.push({ ...port, detected: port.detected !== false });
  }
  return merged.sort((a, b) => {
    const aCh340 = /CH340|CH341|USB-SERIAL|USB TO UART/i.test(`${a.label} ${a.hwid}`) ? 0 : 1;
    const bCh340 = /CH340|CH341|USB-SERIAL|USB TO UART/i.test(`${b.label} ${b.hwid}`) ? 0 : 1;
    if (aCh340 !== bCh340) return aCh340 - bCh340;
    return String(a.device).localeCompare(String(b.device), undefined, { numeric: true });
  });
}

function syncSerialSelectOptions(selected = els.serialPortSelect.value) {
  for (const select of els.sharedSerialSelects) {
    fillSerialSelect(select, state.serialPorts, selected);
  }
}

function syncSerialUi() {
  const label = state.serialConnected ? t("disconnect") : t("connectSerial");
  if (els.serialBtn) {
    els.serialBtn.textContent = label;
  }
  for (const button of els.sharedSerialButtons) {
    button.textContent = label;
  }
  const selected = els.serialPortSelect ? els.serialPortSelect.value : "";
  for (const select of els.sharedSerialSelects) {
    setSelectValueIfPresent(select, selected);
  }
}

function selectSerialPortFromElement(element) {
  const panel = element ? element.closest(".shared-serial-panel") : null;
  const select = panel ? panel.querySelector(".shared-serial-select") : null;
  if (select && select.value) {
    els.serialPortSelect.value = select.value;
    localStorage.setItem("digitnn-serial-port", select.value);
    syncSerialUi();
  }
}

function setSourceSegmentActive(button) {
  const group = button ? button.closest(".segmented") : null;
  if (!group) return;
  for (const item of group.querySelectorAll(".segment")) {
    item.classList.toggle("active", item === button);
  }
}

function sourcePanelFor(scope) {
  if (scope === "letter") return document.querySelector(".letter-source-panel");
  if (scope === "chinese") return document.querySelector(".chinese-source-panel");
  return null;
}

function isMcuSourceMode(scope) {
  const panel = sourcePanelFor(scope);
  return !!panel && panel.classList.contains("source-mode-mcu");
}

function applyNamedSourceMode(scope) {
  const mode = scope === "letter" ? state.letterSourceMode : state.chineseSourceMode;
  const safeMode = mode === "mcu" ? "mcu" : "browser";
  const panel = sourcePanelFor(scope);
  if (!panel) return;
  panel.classList.toggle("source-mode-browser", safeMode === "browser");
  panel.classList.toggle("source-mode-mcu", safeMode === "mcu");
  for (const button of panel.querySelectorAll("[data-source-toggle]")) {
    button.classList.toggle("active", button.dataset.sourceMode === safeMode);
  }
}

function setNamedSourceMode(scope, mode) {
  const safeMode = mode === "mcu" ? "mcu" : "browser";
  if (scope === "letter") {
    state.letterSourceMode = safeMode;
    localStorage.setItem("digitnn-letter-source-mode", safeMode);
  }
  if (scope === "chinese") {
    state.chineseSourceMode = safeMode;
    localStorage.setItem("digitnn-chinese-source-mode", safeMode);
  }
  applyNamedSourceMode(scope);
  if (safeMode === "mcu") {
    initBoardCanvas();
  }
}

function initCanvas() {
  ctx.fillStyle = "#000000";
  ctx.fillRect(0, 0, els.drawCanvas.width, els.drawCanvas.height);
  ctx.lineCap = "round";
  ctx.lineJoin = "round";
  ctx.strokeStyle = "#ffffff";
  ctx.fillStyle = "#ffffff";
}

function initChineseCanvas() {
  chineseCtx.fillStyle = "#0b1220";
  chineseCtx.fillRect(0, 0, els.chineseCanvas.width, els.chineseCanvas.height);
  chineseCtx.strokeStyle = "rgba(148, 163, 184, 0.18)";
  chineseCtx.lineWidth = 1;
  const grid = 40;
  for (let x = 0; x <= els.chineseCanvas.width; x += grid) {
    chineseCtx.beginPath();
    chineseCtx.moveTo(x, 0);
    chineseCtx.lineTo(x, els.chineseCanvas.height);
    chineseCtx.stroke();
  }
  for (let y = 0; y <= els.chineseCanvas.height; y += grid) {
    chineseCtx.beginPath();
    chineseCtx.moveTo(0, y);
    chineseCtx.lineTo(els.chineseCanvas.width, y);
    chineseCtx.stroke();
  }
  chineseCtx.lineCap = "round";
  chineseCtx.lineJoin = "round";
  chineseCtx.strokeStyle = "#f8fafc";
  chineseCtx.fillStyle = "#f8fafc";
  state.lastChinesePoint = null;
  renderChinesePreview();
}

function initChineseCanvasIfBlank() {
  const sample = chineseCtx.getImageData(0, 0, 1, 1).data;
  if (sample[0] === 0 && sample[1] === 0 && sample[2] === 0 && sample[3] === 0) {
    initChineseCanvas();
  }
}

function boardSurfaces() {
  return [{ canvas: els.boardCanvas, context: boardCtx }, ...sharedBoardContexts];
}

function syncSingleBoardCanvasSize(canvas) {
  const rect = canvas.getBoundingClientRect();
  const width = Math.max(380, Math.round(rect.width || 0));
  const height = Math.max(240, Math.round(rect.height || 0));
  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
  }
}

function syncBoardCanvasSize() {
  for (const surface of boardSurfaces()) {
    syncSingleBoardCanvasSize(surface.canvas);
  }
  if (boardInkCanvas.width !== els.boardCanvas.width || boardInkCanvas.height !== els.boardCanvas.height) {
    boardInkCanvas.width = els.boardCanvas.width;
    boardInkCanvas.height = els.boardCanvas.height;
  }
}

function resetBoardSurface(canvas, context) {
  context.fillStyle = "#0e1726";
  context.fillRect(0, 0, canvas.width, canvas.height);
  context.strokeStyle = "rgba(148, 163, 184, 0.18)";
  context.lineWidth = 1;
  const gridX = Math.max(32, Math.round(canvas.width / 10));
  const gridY = Math.max(28, Math.round(canvas.height / 8));
  for (let x = 0; x <= canvas.width; x += gridX) {
    context.beginPath();
    context.moveTo(x, 0);
    context.lineTo(x, canvas.height);
    context.stroke();
  }
  for (let y = 0; y <= canvas.height; y += gridY) {
    context.beginPath();
    context.moveTo(0, y);
    context.lineTo(canvas.width, y);
    context.stroke();
  }
  context.lineCap = "round";
  context.lineJoin = "round";
  context.strokeStyle = "#f8fafc";
  context.fillStyle = "#f8fafc";
}

function resetBoardInkCanvas() {
  boardInkCtx.clearRect(0, 0, boardInkCanvas.width, boardInkCanvas.height);
  boardInkCtx.lineCap = "round";
  boardInkCtx.lineJoin = "round";
  boardInkCtx.strokeStyle = "#f8fafc";
  boardInkCtx.fillStyle = "#f8fafc";
}

function initBoardCanvas() {
  syncBoardCanvasSize();
  for (const surface of boardSurfaces()) {
    resetBoardSurface(surface.canvas, surface.context);
  }
  resetBoardInkCanvas();
  state.boardLastPoint = null;
  state.boardPointCount = 0;
  state.boardStrokeCount = 0;
  state.lastBoardImage = null;
  state.boardImageSaved = false;
  window.clearTimeout(state.boardAutoSaveTimer);
  state.boardAutoSaveTimer = null;
  state.lastMcuResults = {};
  updateBoardStats("--", "--");
}

function scheduleBoardCanvasInit() {
  window.requestAnimationFrame(() => {
    initBoardCanvas();
    window.setTimeout(initBoardCanvas, 80);
  });
}

function canvasPoint(event) {
  const rect = els.drawCanvas.getBoundingClientRect();
  return {
    x: (event.clientX - rect.left) * (els.drawCanvas.width / rect.width),
    y: (event.clientY - rect.top) * (els.drawCanvas.height / rect.height),
  };
}

function drawDot(point) {
  const radius = Number(els.brushRange.value) / 2;
  ctx.beginPath();
  ctx.arc(point.x, point.y, radius, 0, Math.PI * 2);
  ctx.fill();
}

function drawLine(from, to) {
  ctx.lineWidth = Number(els.brushRange.value);
  ctx.beginPath();
  ctx.moveTo(from.x, from.y);
  ctx.lineTo(to.x, to.y);
  ctx.stroke();
  drawDot(to);
}

function clearBoardCanvas() {
  initBoardCanvas();
  setStatus(t("boardViewCleared"));
}

function updateBoardStats(x, y) {
  const text = t("boardStats", {
    points: state.boardPointCount,
    strokes: state.boardStrokeCount,
    x,
    y,
  });
  els.boardStats.textContent = text;
  for (const stats of els.sharedBoardStats) {
    stats.textContent = text;
  }
}

function scheduleInfer() {
  window.clearTimeout(state.inferTimer);
  state.inferTimer = window.setTimeout(runInfer, 160);
}

function clearCanvas() {
  initCanvas();
  state.lastPoint = null;
  updatePixels(new Array(784).fill(0));
  setStatus(t("canvasCleared"));
  for (const tile of els.pcResults.querySelectorAll(".result-tile")) {
    tile.querySelector(".result-label").textContent = "--";
    tile.querySelector(".result-meta").textContent = t("idle");
    tile.querySelector(".bar > div").style.width = "0%";
  }
}

function imagePayload() {
  return {
    image: els.drawCanvas.toDataURL("image/png"),
    thicken: els.thickenCheck.checked,
    deskew: els.deskewCheck.checked,
  };
}

async function apiJson(path, options = {}) {
  const response = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });
  const payload = await response.json();
  if (!response.ok || payload.ok === false) {
    const error = new Error(payload.error || t("requestFailed", { status: response.status }));
    error.payload = payload;
    error.status = response.status;
    throw error;
  }
  return payload;
}

function updatePixels(pixels, width = 28, height = 28) {
  if ((width !== state.pixelWidth) || (height !== state.pixelHeight) ||
      (els.pixelGrid.children.length !== width * height)) {
    initPixelGrid(width, height);
  }
  const cells = els.pixelGrid.children;
  let active = 0;
  let maxValue = 0;
  for (let index = 0; index < pixels.length; index += 1) {
    const value = Number(pixels[index] || 0);
    if (value > 0) active += 1;
    if (value > maxValue) maxValue = value;
    const hex = value.toString(16).padStart(2, "0");
    cells[index].style.background = value > 0 ? `#${hex}${hex}${hex}` : "#0c111c";
  }
  els.pixelStats.textContent = `${width} x ${height} | ${t("activePixels", { active, max: maxValue })}`;
}

function updatePcResults(results) {
  for (const result of results) {
    const tile = els.pcResults.querySelector(`[data-model="${result.model}"]`);
    if (!tile) continue;
    const label = tile.querySelector(".result-label");
    const meta = tile.querySelector(".result-meta");
    const bar = tile.querySelector(".bar > div");
    if (!result.available) {
      label.textContent = "--";
      meta.textContent = t("missing");
      bar.style.width = "0%";
      continue;
    }
    label.textContent = result.label;
    meta.textContent = `${result.confidence}%  ${t("alt", { label: result.second })}`;
    bar.style.width = `${result.confidence}%`;
  }
}

async function runInfer() {
  try {
    const payload = await apiJson("/api/infer", {
      method: "POST",
      body: JSON.stringify(imagePayload()),
    });
    updatePixels(payload.pixels);
    updatePcResults(payload.results);
    setStatus(t("inferenceUpdated"));
  } catch (error) {
    logLine(String(error.message || error), "error");
  }
}

async function runInferPixels(image) {
  try {
    const payload = await apiJson("/api/infer-pixels", {
      method: "POST",
      body: JSON.stringify({
        width: image.width,
        height: image.height,
        pixels: image.pixels,
      }),
    });
    updatePcResults(payload.results);
  } catch (error) {
    logLine(String(error.message || error), "error");
  }
}

async function saveSample() {
  try {
    const payload = await apiJson("/api/save-sample", {
      method: "POST",
      body: JSON.stringify({
        ...imagePayload(),
        label: els.labelSelect.value,
        outputDir: els.saveDirInput.value,
      }),
    });
    logLine(t("savedFile", { filename: payload.filename }));
    setStatus(t("sampleSaved"));
  } catch (error) {
    logLine(String(error.message || error), "error");
  }
}

function setBatchRunning(running) {
  if (els.runDigitBatchTestBtn) {
    els.runDigitBatchTestBtn.disabled = running;
    els.runDigitBatchTestBtn.textContent = running ? t("testRunning") : t("runDigitBatchTest");
  }
  if (els.runLetterBatchTestBtn) {
    els.runLetterBatchTestBtn.disabled = running;
    els.runLetterBatchTestBtn.textContent = running ? t("testRunning") : t("runLetterBatchTest");
  }
}

function datasetDisplayName(key, fallback) {
  if (key === "standard") return fallback ? `${t("standardSet")} / ${fallback}` : t("standardSet");
  if (key === "personal") return fallback ? `${t("personalSet")} / ${fallback}` : t("personalSet");
  return fallback || key;
}

function summarizeDataset(dataset) {
  const rows = dataset.results || [];
  if (!rows.length) return null;
  return rows.reduce((best, item) => {
    if (!best) return item;
    return Number(item.accuracy || 0) > Number(best.accuracy || 0) ? item : best;
  }, null);
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
}

function renderBatchResults(payload) {
  const rows = [];
  const confusionRows = [];
  for (const dataset of payload.datasets || []) {
    if (dataset.error) {
      rows.push(`
        <tr>
          <td>${datasetDisplayName(dataset.dataset, dataset.datasetName)}</td>
          <td>${escapeHtml(payload.domain || "--")}</td>
          <td colspan="4">${escapeHtml(dataset.error)}</td>
        </tr>
      `);
      continue;
    }
    for (const result of dataset.results || []) {
      const modelName = result.modelName || result.model || "--";
      if (result.error) {
        rows.push(`
          <tr>
            <td>${datasetDisplayName(dataset.dataset, dataset.datasetName)}</td>
            <td>${escapeHtml(modelName)}</td>
            <td colspan="4">${escapeHtml(result.error)}</td>
          </tr>
        `);
        continue;
      }
      rows.push(`
        <tr>
          <td>${datasetDisplayName(dataset.dataset, dataset.datasetName)}</td>
          <td>${escapeHtml(modelName)}</td>
          <td>${result.total}</td>
          <td>${result.correct}</td>
          <td>${(Number(result.accuracy || 0) * 100).toFixed(1)}%</td>
          <td>${Number(result.avgTimeUs || 0).toFixed(1)} us</td>
        </tr>
      `);
      for (const item of (result.confusions || []).slice(0, 5)) {
        confusionRows.push(`
          <tr>
            <td>${datasetDisplayName(dataset.dataset, dataset.datasetName)}</td>
            <td>${escapeHtml(modelName)}</td>
            <td><strong>${escapeHtml(item.truth)} -> ${escapeHtml(item.prediction)}</strong></td>
            <td>${Number(item.count || 0)}</td>
            <td>${(item.examples || []).map(escapeHtml).join("<br>") || "--"}</td>
          </tr>
        `);
      }
    }
  }
  els.batchResultBody.innerHTML = rows.length ? rows.join("") : `<tr><td colspan="6">${t("testEmpty")}</td></tr>`;
  els.confusionResultBody.innerHTML = confusionRows.length ? confusionRows.join("") : `<tr><td colspan="5">${t("noConfusions")}</td></tr>`;

  for (const dataset of payload.datasets || []) {
    const best = summarizeDataset(dataset);
    const accuracyText = best ? `${(Number(best.accuracy || 0) * 100).toFixed(1)}%` : "--";
    const timeText = best ? `${best.model} / ${Number(best.avgTimeUs || 0).toFixed(1)} us` : "--";
    if (dataset.dataset === "standard") {
      els.standardAccuracyText.textContent = accuracyText;
      els.standardTimeText.textContent = timeText;
    }
    if (dataset.dataset === "personal") {
      els.personalAccuracyText.textContent = accuracyText;
      els.personalTimeText.textContent = timeText;
    }
  }
}

async function runBatchTest(domain = "digit") {
  setBatchRunning(true);
  setStatus(t("testRunning"));
  try {
    const payload = await apiJson("/api/batch-test", {
      method: "POST",
      body: JSON.stringify({ domain, model: els.batchModelSelect.value }),
    });
    renderBatchResults(payload);
    setStatus(t("testFinished"));
  } catch (error) {
    logLine(String(error.message || error), "error");
  } finally {
    setBatchRunning(false);
  }
}

function setTfStatusChip(element, text, tone = "warn") {
  if (!element) return;
  element.textContent = text;
  element.classList.remove("ok", "warn", "danger");
  element.classList.add(tone);
}

function pickTfCardCandidate(payload) {
  const candidates = payload?.pc?.candidates || [];
  return (
    candidates.find((item) => item.available && item.kind === "mounted") ||
    candidates.find((item) => item.available && item.kind === "workspace") ||
    candidates.find((item) => item.available) ||
    candidates[0] ||
    null
  );
}

function renderTfPathList(candidates) {
  if (!els.tfPathList) return;
  const visibleCandidates = (candidates || []).filter((item) => item.available || item.kind === "workspace");
  if (!visibleCandidates.length) {
    els.tfPathList.innerHTML = `<div class="tf-path-item warn">没有可检测路径。</div>`;
    return;
  }
  els.tfPathList.innerHTML = visibleCandidates
    .map((item) => {
      const kind = item.kind === "workspace" ? "工程制卡镜像" : "本机挂载目录";
      const tone = item.available ? "ok" : "warn";
      const images = Number(item.totalImages || 0);
      const labels = Number(item.labelFiles || 0);
      return `
        <div class="tf-path-item ${tone}">
          <strong>${escapeHtml(kind)}</strong>
          <span>${escapeHtml(item.path || "--")}</span>
          <em>${item.available ? `${images} images / ${labels} label files` : "not found"}</em>
        </div>
      `;
    })
    .join("");
}

function renderTfDatasetTable(candidate) {
  if (!els.tfDatasetBody) return;
  const datasets = candidate?.datasets || [];
  if (!candidate?.available) {
    els.tfDatasetBody.innerHTML = `<tr><td colspan="5">没有发现可读取的 tf_card 目录。</td></tr>`;
    return;
  }
  if (!datasets.length) {
    els.tfDatasetBody.innerHTML = `<tr><td colspan="5">目录存在，但未在一级子目录中发现图片或 label.txt。</td></tr>`;
    return;
  }
  els.tfDatasetBody.innerHTML = datasets
    .map((dataset) => {
      const preview = (dataset.preview || []).slice(0, 4).map(escapeHtml).join("<br>") || "--";
      return `
        <tr>
          <td>${escapeHtml(dataset.name || "--")}</td>
          <td>${escapeHtml(dataset.path || "--")}</td>
          <td>${Number(dataset.images || 0)}</td>
          <td>${Number(dataset.labelFiles || 0)}</td>
          <td>${preview}</td>
        </tr>
      `;
    })
    .join("");
}

function renderTfEvidence(payload) {
  if (!els.tfEvidenceLog || !els.tfSerialStatus) return;
  const serial = payload?.mcu?.serial || {};
  const portText = serial.port || "no port";
  const openText = serial.open ? "open" : "closed";
  const errorText = serial.error ? ` / ${serial.error}` : "";
  els.tfSerialStatus.textContent = `${portText} / ${openText}${errorText}`;

  const frames = payload?.mcu?.tfFramesSeen || payload?.mcu?.sdFramesSeen || [];
  if (frames.length) {
    els.tfEvidenceLog.textContent = frames.map((item) => item.line || "").join("\n");
    return;
  }
  const capability = payload?.mcu?.capability || {};
  const files = capability.files || {};
  const fatfsFiles = Object.entries(files)
    .map(([name, paths]) => `${name}: ${(paths || []).length ? "found" : "missing"}`)
    .join("\n");
  els.tfEvidenceLog.textContent = [
    "No TF/FatFs frames were found in the current serial stream.",
    "Current firmware protocol is still handwriting-oriented: STATUS, POINT, STROKE, IMAGE, RESULT.",
    "",
    "To prove STM32-side TF-card reading, firmware should emit frames like:",
    "TF_STATUS,state=ready,path=/tf_card",
    "TF_DIR,path=/tf_card/mnist,count=1000",
    "TF_FILE,path=/tf_card/mnist/img_0001.bmp,label=7,size=1078",
    "",
    "Current Keil FatFs file check:",
    fatfsFiles || "No capability data.",
  ].join("\n");
}

function renderTfCardStatus(payload) {
  const candidate = pickTfCardCandidate(payload);
  const candidates = payload?.pc?.candidates || [];
  const mounted = candidates.find((item) => item.available && item.kind === "mounted");
  const workspace = candidates.find((item) => item.available && item.kind === "workspace");
  const drives = payload?.pc?.drives || [];

  if (mounted) {
    setTfStatusChip(els.tfPcStatus, "mounted mirror", "ok");
  } else if (workspace) {
    setTfStatusChip(els.tfPcStatus, "mirror ok", "ok");
  } else {
    setTfStatusChip(els.tfPcStatus, "mirror missing", "warn");
  }

  if (els.tfDriveText) els.tfDriveText.textContent = drives.length ? drives.join("  ") : "--";
  if (els.tfImageText) els.tfImageText.textContent = candidate?.available ? String(candidate.totalImages || 0) : "--";
  if (els.tfLabelText) els.tfLabelText.textContent = candidate?.available ? String(candidate.labelFiles || 0) : "--";
  if (els.tfSourceText) {
    els.tfSourceText.textContent = mounted ? "本机挂载目录" : workspace ? "工程本地制卡镜像" : "--";
  }

  const mcu = payload?.mcu || {};
  if (mcu.canConfirmRead) {
    setTfStatusChip(els.tfMcuStatus, "confirmed", "ok");
  } else if (mcu.firmwareFatfsReady) {
    setTfStatusChip(els.tfMcuStatus, "waiting frames", "warn");
  } else {
    setTfStatusChip(els.tfMcuStatus, "not wired", "danger");
  }
  if (els.tfMcuMessage) {
    if (mcu.canConfirmRead) {
      els.tfMcuMessage.textContent = "串口已出现 TF/FatFs 读卡帧，可以作为 STM32 读取 TF 卡的证据。";
    } else if (mcu.firmwareFatfsReady) {
      els.tfMcuMessage.textContent = "Keil 工程疑似已包含 FatFs 文件，但当前串口还没有 TF 读卡帧。";
    } else {
      els.tfMcuMessage.textContent =
        "当前 DigitNN 固件还没有接入 TF 卡读取逻辑，串口只能证明触摸采集和识别回传，不能证明板端读卡。";
    }
  }

  renderTfPathList(candidates);
  renderTfDatasetTable(candidate);
  renderTfEvidence(payload);
}

async function refreshTfCardStatus() {
  if (!els.tfPcStatus || !els.tfMcuStatus) return;
  setTfStatusChip(els.tfPcStatus, "checking", "warn");
  setTfStatusChip(els.tfMcuStatus, "checking", "warn");
  if (els.tfEvidenceLog) {
    els.tfEvidenceLog.textContent = "Checking TF card paths and serial evidence...";
  }
  try {
    const path = els.tfCardPathInput?.value?.trim() || "";
    const suffix = path ? `?path=${encodeURIComponent(path)}` : "";
    const payload = await apiJson(`/api/tf-card/status${suffix}`);
    renderTfCardStatus(payload);
  } catch (error) {
    setTfStatusChip(els.tfPcStatus, "error", "danger");
    setTfStatusChip(els.tfMcuStatus, "error", "danger");
    if (els.tfEvidenceLog) {
      els.tfEvidenceLog.textContent = String(error.message || error);
    }
  }
}

function initLetterCanvas() {
  letterCtx.fillStyle = "#000000";
  letterCtx.fillRect(0, 0, els.letterCanvas.width, els.letterCanvas.height);
  letterCtx.lineCap = "round";
  letterCtx.lineJoin = "round";
  letterCtx.strokeStyle = "#ffffff";
  letterCtx.fillStyle = "#ffffff";
  renderLetterPreview();
}

function initLetterPixelGrid() {
  if (!els.letterPixelGrid) return;
  els.letterPixelGrid.innerHTML = "";
  const available = Math.max(260, Math.min(560, els.letterPixelGrid.parentElement?.clientWidth || window.innerWidth - 64));
  const pixelSize = Math.max(8, Math.min(18, Math.floor((available - 22) / 28)));
  els.letterPixelGrid.style.setProperty("--pixel-size", `${pixelSize}px`);
  els.letterPixelGrid.style.gridTemplateColumns = "repeat(28, var(--pixel-size))";
  els.letterPixelGrid.style.gridTemplateRows = "repeat(28, var(--pixel-size))";
  for (let index = 0; index < 28 * 28; index += 1) {
    const cell = document.createElement("div");
    cell.className = "pixel";
    els.letterPixelGrid.appendChild(cell);
  }
}

function renderLetterPreview() {
  if (!els.letterPixelGrid) return;
  const offscreen = document.createElement("canvas");
  offscreen.width = 28;
  offscreen.height = 28;
  const previewCtx = offscreen.getContext("2d", { willReadFrequently: true });
  previewCtx.fillStyle = "#000000";
  previewCtx.fillRect(0, 0, 28, 28);
  previewCtx.drawImage(els.letterCanvas, 0, 0, 28, 28);
  const image = previewCtx.getImageData(0, 0, 28, 28).data;
  const cells = els.letterPixelGrid.children;
  let active = 0;
  let max = 0;
  for (let index = 0; index < 28 * 28; index += 1) {
    const value = image[index * 4];
    max = Math.max(max, value);
    if (value > 8) active += 1;
    const cell = cells[index];
    if (cell) {
      cell.style.background = `rgb(${value}, ${value}, ${value})`;
      cell.dataset.value = String(value);
    }
  }
  if (els.letterPixelStats) {
    els.letterPixelStats.textContent = `28 x 28 | ${active} px, max ${max}`;
  }
}

function updateLetterPixels(pixels, width = 28, height = 28) {
  if (!els.letterPixelGrid || width !== 28 || height !== 28) return;
  if (els.letterPixelGrid.children.length !== 28 * 28) {
    initLetterPixelGrid();
  }
  const cells = els.letterPixelGrid.children;
  let active = 0;
  let max = 0;
  for (let index = 0; index < 28 * 28; index += 1) {
    const value = Number(pixels[index] || 0);
    max = Math.max(max, value);
    if (value > 8) active += 1;
    const cell = cells[index];
    if (cell) {
      const hex = value.toString(16).padStart(2, "0");
      cell.style.background = value > 0 ? `#${hex}${hex}${hex}` : "#0c111c";
      cell.dataset.value = String(value);
    }
  }
  if (els.letterPixelStats) {
    els.letterPixelStats.textContent = `28 x 28 | ${active} px, max ${max}`;
  }
}

function renderLetterModelTiles() {
  if (!els.letterResults) return;
  const models = [
    { short: "P", name: "Letter-Perceptron" },
    { short: "F", name: "Letter-FNN" },
    { short: "C", name: "Letter-DS-CNN" },
  ];
  els.letterResults.innerHTML = "";
  for (const model of models) {
    const tile = document.createElement("div");
    tile.className = "result-tile";
    tile.dataset.model = model.short;
    tile.innerHTML = `
      <div class="result-name">${model.short} ${model.name}</div>
      <div class="result-label">--</div>
      <div class="result-meta">${t("letterModelPending")}</div>
      <div class="bar"><div></div></div>
    `;
    els.letterResults.appendChild(tile);
  }
}

function letterPoint(event) {
  const rect = els.letterCanvas.getBoundingClientRect();
  return {
    x: (event.clientX - rect.left) * (els.letterCanvas.width / rect.width),
    y: (event.clientY - rect.top) * (els.letterCanvas.height / rect.height),
  };
}

function drawLetterDot(point) {
  const radius = Number(els.letterBrushRange.value) / 2;
  letterCtx.beginPath();
  letterCtx.arc(point.x, point.y, radius, 0, Math.PI * 2);
  letterCtx.fill();
}

function drawLetterLine(from, to) {
  letterCtx.lineWidth = Number(els.letterBrushRange.value);
  letterCtx.beginPath();
  letterCtx.moveTo(from.x, from.y);
  letterCtx.lineTo(to.x, to.y);
  letterCtx.stroke();
  drawLetterDot(to);
}

function logLetterLine(text) {
  appendLog(els.letterSerialLog, text);
}

function clearLetterCanvas() {
  initLetterCanvas();
  state.lastLetterPoint = null;
  renderLetterPreview();
  logLetterLine(t("letterCanvasCleared"));
}

function chinesePoint(event) {
  const rect = els.chineseCanvas.getBoundingClientRect();
  return {
    x: (event.clientX - rect.left) * (els.chineseCanvas.width / rect.width),
    y: (event.clientY - rect.top) * (els.chineseCanvas.height / rect.height),
  };
}

function drawChineseDot(point) {
  chineseCtx.beginPath();
  chineseCtx.arc(point.x, point.y, 15, 0, Math.PI * 2);
  chineseCtx.fill();
  renderChinesePreview();
}

function drawChineseLine(from, to) {
  chineseCtx.lineWidth = 30;
  chineseCtx.beginPath();
  chineseCtx.moveTo(from.x, from.y);
  chineseCtx.lineTo(to.x, to.y);
  chineseCtx.stroke();
  drawChineseDot(to);
}

function logChineseLine(text) {
  const prefix = new Date().toLocaleTimeString();
  els.chineseLog.textContent += `[${prefix}] ${text}\n`;
  els.chineseLog.scrollTop = els.chineseLog.scrollHeight;
}

function clearChineseCanvas() {
  initChineseCanvas();
  els.chineseResultText.textContent = "--";
  els.chineseResultMeta.textContent = "等待输入";
  els.chineseConfidenceBar.style.width = "0%";
  els.chineseCandidateList.innerHTML = "";
  renderChinesePreview();
  logChineseLine("画布已清空");
}

function renderChinesePreview() {
  const size = els.chinesePreviewCanvas.width;
  chinesePreviewCtx.fillStyle = "#0b1220";
  chinesePreviewCtx.fillRect(0, 0, size, size);
  chinesePreviewCtx.drawImage(els.chineseCanvas, 0, 0, size, size);
  const image = chinesePreviewCtx.getImageData(0, 0, size, size);
  let active = 0;
  let maxValue = 0;
  for (let index = 0; index < image.data.length; index += 4) {
    const value = Math.max(image.data[index], image.data[index + 1], image.data[index + 2]);
    const ink = value > 210 ? 255 : 0;
    if (ink) active += 1;
    maxValue = Math.max(maxValue, ink);
    image.data[index] = ink;
    image.data[index + 1] = ink;
    image.data[index + 2] = ink;
    image.data[index + 3] = 255;
  }
  chinesePreviewCtx.putImageData(image, 0, 0);
  els.chinesePreviewStats.textContent = `224 x 224 | ${active} px, max ${maxValue}`;
}

function boardInkBounds() {
  if (!boardInkCanvas.width || !boardInkCanvas.height) return null;
  const image = boardInkCtx.getImageData(0, 0, boardInkCanvas.width, boardInkCanvas.height);
  let minX = boardInkCanvas.width;
  let minY = boardInkCanvas.height;
  let maxX = -1;
  let maxY = -1;
  for (let y = 0; y < boardInkCanvas.height; y += 1) {
    for (let x = 0; x < boardInkCanvas.width; x += 1) {
      const alpha = image.data[(y * boardInkCanvas.width + x) * 4 + 3];
      if (alpha > 16) {
        minX = Math.min(minX, x);
        minY = Math.min(minY, y);
        maxX = Math.max(maxX, x);
        maxY = Math.max(maxY, y);
      }
    }
  }
  if (maxX < minX || maxY < minY) return null;
  const pad = Math.max(16, Math.round(Math.max(maxX - minX, maxY - minY) * 0.18));
  minX = Math.max(0, minX - pad);
  minY = Math.max(0, minY - pad);
  maxX = Math.min(boardInkCanvas.width - 1, maxX + pad);
  maxY = Math.min(boardInkCanvas.height - 1, maxY + pad);
  return { x: minX, y: minY, width: maxX - minX + 1, height: maxY - minY + 1 };
}

function drawBoardImageFallback(targetCanvas, targetContext) {
  if (!state.lastBoardImage) return false;
  const { width, height, pixels } = state.lastBoardImage;
  const temp = document.createElement("canvas");
  temp.width = width;
  temp.height = height;
  const tempCtx = temp.getContext("2d", { willReadFrequently: true });
  const imageData = tempCtx.createImageData(width, height);
  for (let index = 0; index < pixels.length; index += 1) {
    const value = Math.max(0, Math.min(255, Number(pixels[index] || 0)));
    imageData.data[index * 4] = 248;
    imageData.data[index * 4 + 1] = 250;
    imageData.data[index * 4 + 2] = 252;
    imageData.data[index * 4 + 3] = value;
  }
  tempCtx.putImageData(imageData, 0, 0);
  const margin = Math.round(targetCanvas.width * 0.1);
  targetContext.imageSmoothingEnabled = false;
  targetContext.drawImage(temp, margin, margin, targetCanvas.width - margin * 2, targetCanvas.height - margin * 2);
  targetContext.imageSmoothingEnabled = true;
  return true;
}

function copyBoardInkToTarget(targetCanvas, targetContext) {
  const bounds = boardInkBounds();
  if (!bounds) {
    return drawBoardImageFallback(targetCanvas, targetContext);
  }
  const margin = Math.round(targetCanvas.width * 0.09);
  const size = targetCanvas.width - margin * 2;
  targetContext.drawImage(
    boardInkCanvas,
    bounds.x,
    bounds.y,
    bounds.width,
    bounds.height,
    margin,
    margin,
    size,
    size
  );
  return true;
}

function renderBoardInkToLetterInput(silent = false) {
  if (!boardInkBounds() && !state.lastBoardImage) {
    if (!silent) {
      logLetterLine("还没有板端轨迹，请先连接串口并在板上书写");
      setStatus("还没有板端轨迹");
    }
    return false;
  }
  initLetterCanvas();
  copyBoardInkToTarget(els.letterCanvas, letterCtx);
  renderLetterPreview();
  if (!silent) {
    logLetterLine("板端轨迹已同步到字母输入");
    setStatus("板端轨迹已同步到字母输入");
  }
  return true;
}

function renderBoardInkToChineseInput(silent = false) {
  if (!boardInkBounds() && !state.lastBoardImage) {
    if (!silent) {
      logChineseLine("还没有板端轨迹，请先连接串口并在板上书写");
      setStatus("还没有板端轨迹");
    }
    return false;
  }
  initChineseCanvas();
  copyBoardInkToTarget(els.chineseCanvas, chineseCtx);
  renderChinesePreview();
  if (!silent) {
    logChineseLine("板端轨迹已同步到中文输入");
    setStatus("板端轨迹已同步到中文输入");
  }
  return true;
}

function syncActiveMcuSourceInput() {
  if (state.currentView === "letters" && isMcuSourceMode("letter")) {
    if (state.lastBoardImage) {
      updateLetterPixels(state.lastBoardImage.pixels, state.lastBoardImage.width, state.lastBoardImage.height);
    }
  }
  if (state.currentView === "chinese" && isMcuSourceMode("chinese")) {
    renderBoardInkToChineseInput(true);
  }
}

function scheduleBoardSourceSync() {
  window.clearTimeout(state.boardSourceSyncTimer);
  state.boardSourceSyncTimer = window.setTimeout(syncActiveMcuSourceInput, 80);
}

function renderChineseCandidates(candidates) {
  els.chineseCandidateList.innerHTML = "";
  if (!Array.isArray(candidates) || candidates.length === 0) {
    const empty = document.createElement("div");
    empty.className = "candidate-empty";
    empty.textContent = "暂无候选";
    els.chineseCandidateList.appendChild(empty);
    return;
  }
  for (const candidate of candidates.slice(0, 8)) {
    const row = document.createElement("div");
    row.className = "candidate-row";
    const text = document.createElement("strong");
    text.textContent = candidate.text || "--";
    const confidence = document.createElement("span");
    const value = Number(candidate.confidence);
    const percent = value <= 1 ? value * 100 : value;
    confidence.textContent = Number.isFinite(value) ? `${percent.toFixed(value <= 1 ? 1 : 0)}%` : "confidence --";
    row.appendChild(text);
    row.appendChild(confidence);
    els.chineseCandidateList.appendChild(row);
  }
}

function renderChineseStatus(payload) {
  const remote = payload.remote || {};
  const configured = Boolean(remote.configured);
  const providerName = remote.provider === "aliyun" ? "Aliyun" : "CSU";
  els.chineseApiStatus.textContent = configured ? `${providerName} ready` : `${providerName} not configured`;
  els.chineseApiStatus.classList.toggle("danger-pill", !configured);
  els.chineseRemoteText.textContent = configured
    ? `${providerName} 已配置，默认模型 ${remote.model || "--"}`
    : `${providerName} 未配置，先运行对应 save_*_api_key.ps1`;
  els.chineseBaseUrlText.textContent = remote.baseUrl || "--";
  els.chineseTokenNameText.textContent = remote.tokenName || "fa_8202240417";
  if (remote.model && (!els.chineseModelInput.value.trim() || els.chineseProviderSelect.value)) {
    els.chineseModelInput.value = remote.model;
  }

  const runtimes = Array.isArray(payload.localRuntimes) ? payload.localRuntimes : [];
  const installed = runtimes.filter((item) => item.installed).length;
  if (els.chineseRuntimeStatus) {
    els.chineseRuntimeStatus.textContent = `${installed}/${runtimes.length} local`;
  }
  els.chineseRuntimeGrid.innerHTML = "";
  for (const item of runtimes) {
    const card = document.createElement("div");
    card.className = `runtime-card ${item.installed ? "installed" : ""}`;
    card.innerHTML = `
      <strong>${item.name}</strong>
      <span>${item.installed ? "已安装" : "未安装"}</span>
      <em>${item.role}</em>
    `;
    els.chineseRuntimeGrid.appendChild(card);
  }
}

async function loadChineseStatus() {
  try {
    const provider = els.chineseProviderSelect ? els.chineseProviderSelect.value : "";
    const suffix = provider ? `?provider=${encodeURIComponent(provider)}` : "";
    const payload = await apiJson(`/api/chinese/status${suffix}`);
    renderChineseStatus(payload);
  } catch (error) {
    els.chineseApiStatus.textContent = "status error";
    els.chineseApiStatus.classList.add("danger-pill");
    logChineseLine(String(error.message || error));
  }
}

async function runChineseRecognition() {
  try {
    els.runChineseBtn.disabled = true;
    renderChinesePreview();
    els.chineseResultText.textContent = "--";
    els.chineseResultMeta.textContent = "识别中...";
    els.chineseConfidenceBar.style.width = "0%";
    const payload = await apiJson("/api/chinese/infer", {
      method: "POST",
      body: JSON.stringify({
        image: els.chineseCanvas.toDataURL("image/png"),
        provider: els.chineseProviderSelect.value,
        model: els.chineseModelInput.value.trim(),
        prompt: els.chinesePromptInput.value.trim(),
      }),
    });
    const best = Array.isArray(payload.candidates) ? payload.candidates[0] : null;
    const confidence = best && Number.isFinite(Number(best.confidence)) ? Number(best.confidence) : null;
    const percent = confidence === null ? 0 : (confidence <= 1 ? confidence * 100 : confidence);
    els.chineseResultText.textContent = payload.text || payload.message || "无结果";
    els.chineseResultMeta.textContent = payload.model ? `${payload.provider || "API"} / ${payload.model}` : "识别完成";
    els.chineseConfidenceBar.style.width = `${Math.max(0, Math.min(percent, 100)).toFixed(1)}%`;
    renderChineseCandidates(payload.candidates);
    logChineseLine(payload.available ? `recognized by ${payload.model || "remote API"}` : payload.message);
    if (payload.rawText) {
      logChineseLine(payload.rawText);
    }
  } catch (error) {
    els.chineseResultText.textContent = "--";
    els.chineseResultMeta.textContent = "识别失败";
    els.chineseConfidenceBar.style.width = "0%";
    renderChineseCandidates([]);
    logChineseLine(String(error.message || error));
  } finally {
    els.runChineseBtn.disabled = false;
    loadChineseStatus();
  }
}

async function probeChineseModels() {
  try {
    els.probeChineseModelsBtn.disabled = true;
    logChineseLine("probing CSU vision models...");
    const manual = els.chineseModelInput.value
      .split(/[,\s]+/)
      .map((item) => item.trim())
      .filter(Boolean);
    const payload = await apiJson("/api/chinese/probe-models", {
      method: "POST",
      body: JSON.stringify({ provider: els.chineseProviderSelect.value, models: manual, limit: 18 }),
    });
    if (!payload.configured) {
      logChineseLine(payload.message || "API key is not configured");
      return;
    }
    for (const item of payload.results || []) {
      if (item.visionOk) {
        logChineseLine(`[OK] ${item.model}: ${item.reply || "vision request accepted"}`);
      } else {
        logChineseLine(`[--] ${item.model}: ${item.error || "not available"}`);
      }
    }
    if (payload.recommended) {
      els.chineseModelInput.value = payload.recommended;
      logChineseLine(`recommended vision model: ${payload.recommended}`);
    } else {
      logChineseLine("no image-capable model found in current candidates");
    }
  } catch (error) {
    logChineseLine(String(error.message || error));
  } finally {
    els.probeChineseModelsBtn.disabled = false;
  }
}

async function saveLetterSample() {
  try {
    const payload = await apiJson("/api/save-letter-sample", {
      method: "POST",
      body: JSON.stringify({
        image: els.letterCanvas.toDataURL("image/png"),
        label: els.letterLabelSelect.value,
        outputDir: els.letterSaveDirInput.value,
      }),
    });
    logLetterLine(t("savedFile", { filename: payload.filename }));
    setStatus(t("letterSampleSaved"));
  } catch (error) {
    logLetterLine(String(error.message || error));
    setStatus(String(error.message || error));
  }
}

async function saveBoardSample(auto = false) {
  if (!state.lastBoardImage) {
    if (!auto) {
      logLine(t("noBoardImage"), "error");
      setStatus(t("noBoardImage"));
    }
    return;
  }
  if (auto && state.boardImageSaved) {
    return;
  }

  try {
    const domain = state.currentView === "letters" && isMcuSourceMode("letter") ? "letter" : "digit";
    const label = domain === "letter" ? els.letterLabelSelect.value : els.labelSelect.value;
    const outputDir = domain === "letter" ? els.letterSaveDirInput.value : els.saveDirInput.value;
    const payload = await apiJson("/api/save-board-sample", {
      method: "POST",
      body: JSON.stringify({
        width: state.lastBoardImage.width,
        height: state.lastBoardImage.height,
        pixels: state.lastBoardImage.pixels,
        label,
        outputDir,
        results: state.lastMcuResults,
      }),
    });
    state.boardImageSaved = true;
    logLine(t("savedFile", { filename: payload.filename }));
    setStatus(t("boardSampleSaved"));
  } catch (error) {
    logLine(String(error.message || error), "error");
  }
}

function renderModelTiles(models) {
  els.pcResults.innerHTML = "";
  if (els.mcuResults) {
    els.mcuResults.innerHTML = "";
  }
  for (const model of models) {
    const pcTile = document.createElement("div");
    pcTile.className = "result-tile";
    pcTile.dataset.model = model.model;
    pcTile.innerHTML = `
      <div class="result-name">${model.short} ${model.name}</div>
      <div class="result-label">--</div>
      <div class="result-meta">${t("idle")}</div>
      <div class="bar"><div></div></div>
    `;
    els.pcResults.appendChild(pcTile);
  }
}

function formatBytes(bytes) {
  if (bytes === null || bytes === undefined || bytes === "") {
    return "--";
  }
  const value = Number(bytes);
  if (!Number.isFinite(value)) {
    return "--";
  }
  if (value >= 1024 * 1024) {
    return `${(value / (1024 * 1024)).toFixed(2)} MB`;
  }
  if (value >= 1024) {
    return `${(value / 1024).toFixed(1)} KB`;
  }
  return `${Math.round(value)} B`;
}

function setMeterWidth(element, value, limit) {
  if (!element) return;
  const numberValue = Number(value);
  const numberLimit = Number(limit);
  if (!Number.isFinite(numberValue) || !Number.isFinite(numberLimit) || numberLimit <= 0) {
    element.style.width = "0%";
    return;
  }
  const percent = Math.max(0, Math.min((numberValue / numberLimit) * 100, 100));
  element.style.width = `${percent.toFixed(1)}%`;
}

function setMeterPercent(element, percent) {
  if (!element) return;
  const numberPercent = Number(percent);
  if (!Number.isFinite(numberPercent)) {
    element.style.width = "0%";
    return;
  }
  element.style.width = `${Math.max(0, Math.min(numberPercent, 100)).toFixed(1)}%`;
}

function usageForDomain(data, domain) {
  if (!data) return null;
  const domains = data.domains || {};
  const domainUsage = domains[domain];
  if (domainUsage && domainUsage.available) {
    return domainUsage;
  }
  if (data.available && data.domain === domain) {
    return data;
  }
  return null;
}

function firstAvailableUsage(data) {
  if (!data) return null;
  if (data.available) return data;
  const domains = data.domains || {};
  return ["digit", "letter"].map((domain) => domains[domain]).find((entry) => entry && entry.available) || null;
}

function setUsageDisplay(textElement, meterElement, usage, kind) {
  if (!textElement || !meterElement) return;
  if (!usage || !usage.available) {
    textElement.textContent = "--";
    meterElement.style.width = "0%";
    return;
  }
  if (kind === "flash") {
    textElement.textContent = formatBytes(usage.flash);
    setMeterPercent(meterElement, usage.flashPercent);
  } else {
    textElement.textContent = formatBytes(usage.sram);
    setMeterPercent(meterElement, usage.sramPercent);
  }
}

function setUsageText(textElement, usage, kind) {
  if (!textElement) return;
  if (!usage || !usage.available) {
    textElement.textContent = "--";
    return;
  }
  textElement.textContent = formatBytes(kind === "flash" ? usage.flash : usage.sram);
}

function renderQuantModelStrip(profile) {
  els.quantModelStrip.innerHTML = "";
  const models = (profile && profile.models ? profile.models : []).filter((model) => model.available);
  const maxFloatBytes = Math.max(...models.map((model) => Number(model.floatBytes || 0)), 1);
  for (const model of models) {
    const floatBytes = Number(model.floatBytes || 0);
    const quantBytes = Number(model.quantBytes || 0);
    const row = document.createElement("div");
    row.className = "quant-model-row";
    row.innerHTML = `
      <div class="quant-model-head">
        <span class="quant-model-key">${model.short || model.model}</span>
        <span class="quant-model-name">${model.name || model.model}</span>
      </div>
      <div class="quant-model-track">
        <span class="float-model-fill"></span>
        <span class="int8-model-fill"></span>
      </div>
      <div class="quant-model-values">
        <span><strong>${formatBytes(floatBytes)}</strong>float32</span>
        <span><strong>${formatBytes(quantBytes)}</strong>int8</span>
      </div>
    `;
    row.querySelector(".float-model-fill").style.width = `${Math.max(3, (floatBytes / maxFloatBytes) * 100).toFixed(1)}%`;
    row.querySelector(".int8-model-fill").style.width = `${Math.max(3, (quantBytes / maxFloatBytes) * 100).toFixed(1)}%`;
    els.quantModelStrip.appendChild(row);
  }
}

function renderQuantizationProfile(profile = state.quantizationProfile) {
  state.quantizationProfile = profile;
  if (!profile || !profile.available) {
    els.floatParamText.textContent = "--";
    els.int8ParamText.textContent = "--";
    els.floatFirmwareText.textContent = "--";
    els.quantCompressionText.textContent = "--";
    els.quantSavingText.textContent = "--";
    els.quantFlowRatio.textContent = "--";
    if (els.quantDomainText) {
      els.quantDomainText.textContent = "Domain --";
    }
    els.modelStorageText.textContent = t("quantProfileUnavailable");
    setMeterWidth(els.floatParamBar, 0, 1);
    setMeterWidth(els.int8ParamBar, 0, 1);
    setMeterWidth(els.floatFirmwareBar, 0, 1);
    renderQuantModelStrip(null);
    return;
  }

  const totals = profile.totals || {};
  const firmware = profile.firmware || {};
  const limits = profile.limits || {};
  const domainName = profile.domainName || (profile.domain === "letter" ? "Letter" : "Digit");
  const floatBytes = Number(totals.floatBytes || 0);
  const quantBytes = Number(totals.quantBytes || 0);
  const savedBytes = Number(totals.savedBytes || 0);
  const compression = Number(totals.compression || 0);
  const savedPercent = floatBytes > 0 ? (savedBytes / floatBytes) * 100 : 0;

  els.floatParamText.textContent = formatBytes(floatBytes);
  els.int8ParamText.textContent = formatBytes(quantBytes);
  els.floatFirmwareText.textContent = formatBytes(firmware.estimatedFloatFlash);
  els.modelStorageText.textContent = t("modelStorageDynamic", {
    floatSize: formatBytes(floatBytes),
    int8Size: formatBytes(quantBytes),
    savedSize: formatBytes(savedBytes),
  });
  els.quantCompressionText.textContent = t("compressionDynamic", { ratio: compression.toFixed(1) });
  els.quantSavingText.textContent = t("savingDynamic", { percent: savedPercent.toFixed(1) });
  els.quantFlowRatio.textContent = t("compressionRatio", { ratio: compression.toFixed(1) });
  if (els.quantDomainText) {
    els.quantDomainText.textContent = t("activeFirmwareDomain", { domain: domainName });
  }

  setMeterWidth(els.floatParamBar, floatBytes, floatBytes || 1);
  setMeterWidth(els.int8ParamBar, quantBytes, floatBytes || 1);
  setMeterWidth(els.floatFirmwareBar, firmware.estimatedFloatFlash, limits.flash);
  if (firmware.quantizedFlash) {
    els.quantFlashText.textContent = formatBytes(firmware.quantizedFlash);
    setMeterWidth(els.quantFlashBar, firmware.quantizedFlash, limits.flash);
  }
  if (firmware.quantizedSram) {
    els.quantSramText.textContent = formatBytes(firmware.quantizedSram);
    setMeterWidth(els.quantSramBar, firmware.quantizedSram, limits.sram);
  }
  renderQuantModelStrip(profile);
}

async function refreshQuantizationProfile() {
  try {
    const profile = await apiJson("/api/quantization-profile");
    renderQuantizationProfile(profile);
  } catch (error) {
    logLine(String(error.message || error), "error");
    renderQuantizationProfile(null);
  }
}

async function refreshUsageOnly() {
  try {
    const usage = await apiJson("/api/usage");
    updateUsage(usage);
    if (state.currentView === "home") {
      refreshQuantizationProfile();
    }
  } catch (error) {
    logLine(String(error.message || error), "error");
  }
}

function updateUsage(usage) {
  const data = usage && usage.usage ? usage.usage : usage;
  const digitUsage = usageForDomain(data, "digit");
  const letterUsage = usageForDomain(data, "letter");
  const activeDomain = data && data.activeDomain;
  const activeUsage = activeDomain ? usageForDomain(data, activeDomain) : firstAvailableUsage(data);

  setUsageDisplay(els.flashText, els.flashMeter, digitUsage, "flash");
  setUsageDisplay(els.sramText, els.sramMeter, digitUsage, "sram");
  setUsageDisplay(els.letterFlashText, els.letterFlashBar, letterUsage, "flash");
  setUsageDisplay(els.letterSramText, els.letterSramBar, letterUsage, "sram");
  setUsageText(els.homeFlashText, activeUsage, "flash");
  setUsageText(els.homeSramText, activeUsage, "sram");
  setUsageDisplay(els.quantFlashText, els.quantFlashBar, activeUsage, "flash");
  setUsageDisplay(els.quantSramText, els.quantSramBar, activeUsage, "sram");
  setUsageDisplay(els.chineseFlashText, els.chineseFlashBar, activeUsage, "flash");
  setUsageDisplay(els.chineseSramText, els.chineseSramBar, activeUsage, "sram");
}

async function refreshStatus() {
  try {
    const payload = await apiJson("/api/status");
    state.labels = payload.labels;
    state.models = payload.models;
    els.labelSelect.innerHTML = "";
    for (const label of state.labels) {
      const option = document.createElement("option");
      option.value = label;
      option.textContent = label;
      els.labelSelect.appendChild(option);
    }
    if (!els.saveDirInput.value) {
      els.saveDirInput.value = payload.saveDir;
    }
    if (els.letterSaveDirInput && payload.letterSaveDir && !els.letterSaveDirInput.value) {
      els.letterSaveDirInput.value = payload.letterSaveDir;
    }
    if (!els.uv4Input.value && payload.uv4) {
      els.uv4Input.value = payload.uv4;
    }
    if (els.letterUv4Input && !els.letterUv4Input.value && payload.uv4) {
      els.letterUv4Input.value = payload.uv4;
    }
    renderModelTiles(state.models);
    renderLetterModelTiles();
    updateUsage(payload.usage);
    await refreshQuantizationProfile();
    const ready = state.models.filter((item) => item.available).length;
    setStatus(t("modelsReady", { ready, total: state.models.length }));
    await runInfer();
  } catch (error) {
    logLine(String(error.message || error), "error");
  }
}

function parseFrame(line) {
  const parts = line.trim().split(",");
  const frame = { type: parts[0] || "" };
  for (const part of parts.slice(1)) {
    const at = part.indexOf("=");
    if (at < 0) continue;
    frame[part.slice(0, at).trim()] = part.slice(at + 1).trim();
  }
  return frame;
}

function updateMcuResult(model, label, confidence, timeUs = "") {
  state.lastMcuResults[model] = {
    label: label || "",
    confidence: Number(confidence || 0),
    timeUs: timeUs || "",
  };
  const conf = Math.max(0, Math.min(100, Number(confidence || 0)));
  const digitModel = { P: "perceptron", F: "fnn", C: "cnn" }[model] || model;
  const tiles = [
    els.mcuResults?.querySelector(`[data-model="${model}"]`),
    els.pcResults?.querySelector(`[data-model="${digitModel}"]`),
    els.letterResults?.querySelector(`[data-model="${model}"]`),
  ].filter(Boolean);
  for (const tile of tiles) {
    tile.querySelector(".result-label").textContent = label || "--";
    tile.querySelector(".result-meta").textContent = `${conf}%${timeUs ? `  ${timeUs} us` : ""}`;
    tile.querySelector(".bar > div").style.width = `${conf}%`;
  }
}

function boardPointFromRaw(rawPoint, canvas) {
  const width = BOARD_LCD.right - BOARD_LCD.left;
  const height = BOARD_LCD.bottom - BOARD_LCD.top;
  const ratioX = Math.max(0, Math.min(1, (rawPoint.x - BOARD_LCD.left) / width));
  const ratioY = Math.max(0, Math.min(1, (rawPoint.y - BOARD_LCD.top) / height));
  return {
    x: ratioX * canvas.width,
    y: ratioY * canvas.height,
  };
}

function drawBoardPointOn(context, canvas, point) {
  context.beginPath();
  context.arc(point.x, point.y, Math.max(3.2, canvas.width / 220), 0, Math.PI * 2);
  context.fill();
}

function drawBoardLineOn(context, canvas, from, to) {
  context.lineWidth = Math.max(5.5, Math.min(8, canvas.width / 145));
  context.beginPath();
  context.moveTo(from.x, from.y);
  context.lineTo(to.x, to.y);
  context.stroke();
  drawBoardPointOn(context, canvas, to);
}

function handleBoardPoint(frame) {
  const rawX = Number(frame.x);
  const rawY = Number(frame.y);
  if (!Number.isFinite(rawX) || !Number.isFinite(rawY)) return;
  const rawPoint = { x: rawX, y: rawY };
  for (const surface of boardSurfaces()) {
    const point = boardPointFromRaw(rawPoint, surface.canvas);
    if (state.boardLastPoint) {
      drawBoardLineOn(
        surface.context,
        surface.canvas,
        boardPointFromRaw(state.boardLastPoint, surface.canvas),
        point
      );
    } else {
      drawBoardPointOn(surface.context, surface.canvas, point);
    }
  }
  const inkPoint = boardPointFromRaw(rawPoint, boardInkCanvas);
  if (state.boardLastPoint) {
    drawBoardLineOn(boardInkCtx, boardInkCanvas, boardPointFromRaw(state.boardLastPoint, boardInkCanvas), inkPoint);
  } else {
    drawBoardPointOn(boardInkCtx, boardInkCanvas, inkPoint);
  }
  state.boardLastPoint = rawPoint;
  state.boardPointCount += 1;
  updateBoardStats(rawX, rawY);
  scheduleBoardSourceSync();
}

function decodeImageFrame(frame) {
  const width = Number(frame.w || 28);
  const height = Number(frame.h || 28);
  const data = frame.data || "";
  const expected = width * height * 2;
  if (!Number.isFinite(width) || !Number.isFinite(height) || data.length < expected) {
    throw new Error(`bad IMAGE frame: ${width}x${height}, ${data.length} hex chars`);
  }
  const pixels = [];
  for (let index = 0; index < width * height; index += 1) {
    pixels.push(Number.parseInt(data.slice(index * 2, index * 2 + 2), 16));
  }
  return { width, height, pixels };
}

function boardAutoSaveEnabled() {
  if (state.currentView === "letters" && isMcuSourceMode("letter")) {
    return Boolean(els.letterAutoSaveBoardCheck && els.letterAutoSaveBoardCheck.checked);
  }
  if (state.currentView === "workspace" && state.sourceMode === "mcu") {
    return Boolean(els.autoSaveBoardCheck && els.autoSaveBoardCheck.checked);
  }
  return false;
}

function handleSerialLine(line) {
  if (!line) return;
  const frame = parseFrame(line);
  if (frame.type === "IMAGE") {
    logSharedSerialLine(`IMAGE,w=${frame.w || "?"},h=${frame.h || "?"},data=<${(frame.data || "").length} hex>`);
  } else if (frame.type !== "POINT") {
    logSharedSerialLine(line);
  }
  if (frame.type === "CLEAR") {
    clearBoardCanvas();
    return;
  }
  if (frame.type === "POINT") {
    handleBoardPoint(frame);
    return;
  }
  if (frame.type === "STROKE") {
    state.boardLastPoint = null;
    state.boardStrokeCount += 1;
    updateBoardStats("--", "--");
    return;
  }
  if (frame.type === "IMAGE") {
    try {
      const image = decodeImageFrame(frame);
      state.lastBoardImage = image;
      state.boardImageSaved = false;
      updatePixels(image.pixels, image.width, image.height);
      if (state.currentView === "letters" && isMcuSourceMode("letter")) {
        updateLetterPixels(image.pixels, image.width, image.height);
      }
      runInferPixels(image);
      scheduleBoardSourceSync();
      setStatus(t("boardImageUpdated"));
      if (boardAutoSaveEnabled()) {
        window.clearTimeout(state.boardAutoSaveTimer);
        state.boardAutoSaveTimer = window.setTimeout(() => saveBoardSample(true), 350);
      }
    } catch (error) {
      logSharedSerialLine(String(error.message || error), "error");
    }
    return;
  }
  if (frame.type === "RESULT") {
    updateMcuResult(frame.model, frame.label, frame.confidence, frame.time_us);
    return;
  }
  const legacy = line.match(/Perceptron=(\d+)\s+conf=(\d+),\s+FNN=(\d+)\s+conf=(\d+),\s+CNN=(\d+)\s+conf=(\d+)/);
  if (legacy) {
    updateMcuResult("P", legacy[1], legacy[2]);
    updateMcuResult("F", legacy[3], legacy[4]);
    updateMcuResult("C", legacy[5], legacy[6]);
  }
}

async function connectSerial() {
  if (state.serialConnected) {
    try {
      window.clearTimeout(state.serialPollTimer);
      state.serialPollTimer = null;
      await apiJson("/api/serial/disconnect", {
        method: "POST",
        body: JSON.stringify({}),
      });
    } catch (error) {
      logSharedSerialLine(String(error.message || error), "error");
    }
    state.serialConnected = false;
    state.serialLastId = 0;
    syncSerialUi();
    logSharedSerialLine(t("serialClosed"));
    return;
  }

  try {
    if (!els.serialPortSelect.value) {
      await loadSerialPorts();
    }
    const port = els.serialPortSelect.value;
    if (!port) {
      throw new Error(t("noSerialPorts"));
    }
    await apiJson("/api/serial/connect", {
      method: "POST",
      body: JSON.stringify({ port, baudrate: 115200 }),
    });
    state.serialConnected = true;
    state.serialLastId = 0;
    syncSerialUi();
    logSharedSerialLine(t("serialOpenedWithPort", { port }));
    pollSerialLoop();
  } catch (error) {
    logSharedSerialLine(String(error.message || error), "error");
  }
}

async function releaseSerialForDeploy(action) {
  if (!state.serialConnected || !String(action || "").includes("flash")) {
    return "";
  }
  const port = els.serialPortSelect.value;
  try {
    window.clearTimeout(state.serialPollTimer);
    state.serialPollTimer = null;
    await apiJson("/api/serial/disconnect", {
      method: "POST",
      body: JSON.stringify({}),
    });
    state.serialConnected = false;
    state.serialLastId = 0;
    syncSerialUi();
    logSharedSerialLine(t("serialReleasedForFlash", { port }));
    return port;
  } catch (error) {
    logSharedSerialLine(String(error.message || error), "error");
    return "";
  }
}

async function reconnectSerialAfterDeploy(port) {
  if (!port) return;
  await new Promise((resolve) => window.setTimeout(resolve, 900));
  if ([...els.serialPortSelect.options].some((option) => option.value === port)) {
    els.serialPortSelect.value = port;
  }
  logSharedSerialLine(t("serialReconnectAfterFlash", { port }));
  await connectSerial();
}

async function loadSerialPorts() {
  try {
    const previous = els.serialPortSelect.value || localStorage.getItem("digitnn-serial-port") || "";
    const payload = await apiJson("/api/serial/ports");
    const ports = expandSerialPorts(payload.ports || []);
    state.serialPorts = ports;
    fillSerialSelect(els.serialPortSelect, ports, previous);
    const preferred = ports.find((port) => /CH340|USB-SERIAL|USB TO UART/i.test(`${port.label} ${port.hwid}`));
    if (previous && ports.some((port) => port.device === previous)) {
      els.serialPortSelect.value = previous;
    } else if (preferred) {
      els.serialPortSelect.value = preferred.device;
    }
    syncSerialSelectOptions(els.serialPortSelect.value);
    syncSerialUi();
    if (els.serialPortSelect.value) {
      localStorage.setItem("digitnn-serial-port", els.serialPortSelect.value);
    }
    setStatus(ports.length ? t("serialPortsLoaded", { count: ports.length }) : t("noSerialPorts"));
  } catch (error) {
    logSharedSerialLine(String(error.message || error), "error");
  }
}

async function pollSerialLoop() {
  if (!state.serialConnected) return;
  try {
    const payload = await apiJson(`/api/serial/read?since=${state.serialLastId}`);
    for (const item of payload.lines || []) {
      state.serialLastId = Math.max(state.serialLastId, Number(item.id || 0));
      handleSerialLine(String(item.line || ""));
    }
    if (payload.status && payload.status.error) {
      logSharedSerialLine(String(payload.status.error), "error");
    }
    if (payload.status && payload.status.open === false) {
      state.serialConnected = false;
      syncSerialUi();
      return;
    }
  } catch (error) {
    logSharedSerialLine(String(error.message || error), "error");
  } finally {
    if (state.serialConnected) {
      state.serialPollTimer = window.setTimeout(pollSerialLoop, 200);
    }
  }
}

async function readSerialLoop() {
  if (state.serialConnected) {
    await pollSerialLoop();
    return;
  }
  if (!("serial" in navigator)) {
    logSharedSerialLine(t("webSerialUnavailable"), "error");
    return;
  }
  const decoder = new TextDecoder();
  while (state.serialPort && state.serialPort.readable) {
    state.serialReader = state.serialPort.readable.getReader();
    try {
      while (true) {
        const { value, done } = await state.serialReader.read();
        if (done) break;
        state.serialBuffer += decoder.decode(value, { stream: true });
        let newlineIndex = state.serialBuffer.indexOf("\n");
        while (newlineIndex >= 0) {
          const line = state.serialBuffer.slice(0, newlineIndex).trim();
          state.serialBuffer = state.serialBuffer.slice(newlineIndex + 1);
          handleSerialLine(line);
          newlineIndex = state.serialBuffer.indexOf("\n");
        }
      }
    } catch (error) {
      logSharedSerialLine(String(error.message || error), "error");
    } finally {
      state.serialReader.releaseLock();
    }
  }
}

async function runDeploy(action) {
  if (state.deployBusy) {
    logLine(t("deployAlreadyRunning"));
    return;
  }
  state.deployBusy = true;
  setDeployDisabled(true);
  setStatus(t("deployRunning", { action }));
  logLine(`deploy ${action}`);
  const reconnectPort = await releaseSerialForDeploy(action);
  try {
    const payload = await apiJson("/api/deploy", {
      method: "POST",
      body: JSON.stringify({
        action,
        domain: "digit",
        model: els.deployModel.value,
        epochs: Number(els.epochsInput.value),
        batchSize: Number(els.batchInput.value),
        uv4: els.uv4Input.value,
        augment: els.augmentCheck.checked,
      }),
    });
    if (payload.output) {
      logLine(payload.output.trim());
    }
    setStatus(t("deployFinished", { action }));
    await refreshStatus();
  } catch (error) {
    if (error.payload && error.payload.output) {
      logLine(String(error.payload.output).trim(), "error");
    }
    logLine(String(error.message || error), "error");
    setStatus(t("deployFailed", { action }));
  } finally {
    state.deployBusy = false;
    setDeployDisabled(false);
    if (reconnectPort) {
      await reconnectSerialAfterDeploy(reconnectPort);
    }
  }
}

async function runLetterDeploy(action) {
  if (state.deployBusy) {
    logLetterLine(t("deployAlreadyRunning"));
    return;
  }
  state.deployBusy = true;
  setDeployDisabled(true);
  setStatus(t("deployRunning", { action }));
  logLetterLine(`letter deploy ${action}`);
  const reconnectPort = await releaseSerialForDeploy(action);
  try {
    const payload = await apiJson("/api/deploy", {
      method: "POST",
      body: JSON.stringify({
        action,
        domain: "letter",
        model: els.letterDeployModel.value,
        epochs: Number(els.letterEpochsInput.value),
        batchSize: Number(els.letterBatchInput.value),
        uv4: els.letterUv4Input.value,
        augment: els.letterAugmentCheck.checked,
      }),
    });
    if (payload.output) {
      logLetterLine(payload.output.trim());
    }
    setStatus(t("deployFinished", { action }));
    await refreshStatus();
  } catch (error) {
    if (error.payload && error.payload.output) {
      logLetterLine(String(error.payload.output).trim());
    }
    logLetterLine(String(error.message || error));
    setStatus(t("deployFailed", { action }));
  } finally {
    state.deployBusy = false;
    setDeployDisabled(false);
    if (reconnectPort) {
      await reconnectSerialAfterDeploy(reconnectPort);
    }
  }
}

function setDeployDisabled(disabled) {
  for (const button of document.querySelectorAll(".deploy-action")) {
    button.disabled = disabled;
  }
  for (const button of document.querySelectorAll(".letter-deploy-action")) {
    button.disabled = disabled;
  }
}

function initLetterLabels() {
  els.letterLabelSelect.innerHTML = "";
  for (const label of LETTER_LABELS) {
    const option = document.createElement("option");
    option.value = label;
    option.textContent = label;
    els.letterLabelSelect.appendChild(option);
  }
}

function initPixelGrid(width = 28, height = 28) {
  state.pixelWidth = width;
  state.pixelHeight = height;
  els.pixelGrid.innerHTML = "";
  const available = Math.max(260, Math.min(560, els.pixelGrid.parentElement?.clientWidth || window.innerWidth - 64));
  const pixelSize = width <= 32 ? Math.max(8, Math.min(18, Math.floor((available - 22) / width))) : 8;
  els.pixelGrid.style.setProperty("--pixel-size", `${pixelSize}px`);
  els.pixelGrid.style.gridTemplateColumns = `repeat(${width}, var(--pixel-size))`;
  els.pixelGrid.style.gridTemplateRows = `repeat(${height}, var(--pixel-size))`;
  for (let index = 0; index < width * height; index += 1) {
    const cell = document.createElement("div");
    cell.className = "pixel";
    els.pixelGrid.appendChild(cell);
  }
}

function bindEvents() {
  els.drawCanvas.addEventListener("pointerdown", (event) => {
    els.drawCanvas.setPointerCapture(event.pointerId);
    state.drawing = true;
    const point = canvasPoint(event);
    state.lastPoint = point;
    drawDot(point);
    scheduleInfer();
  });
  els.drawCanvas.addEventListener("pointermove", (event) => {
    if (!state.drawing || !state.lastPoint) return;
    const point = canvasPoint(event);
    drawLine(state.lastPoint, point);
    state.lastPoint = point;
    scheduleInfer();
  });
  els.drawCanvas.addEventListener("pointerup", () => {
    state.drawing = false;
    state.lastPoint = null;
    scheduleInfer();
  });
  els.drawCanvas.addEventListener("pointercancel", () => {
    state.drawing = false;
    state.lastPoint = null;
  });

  els.letterCanvas.addEventListener("pointerdown", (event) => {
    els.letterCanvas.setPointerCapture(event.pointerId);
    state.letterDrawing = true;
    const point = letterPoint(event);
    state.lastLetterPoint = point;
    drawLetterDot(point);
    renderLetterPreview();
  });
  els.letterCanvas.addEventListener("pointermove", (event) => {
    if (!state.letterDrawing || !state.lastLetterPoint) return;
    const point = letterPoint(event);
    drawLetterLine(state.lastLetterPoint, point);
    state.lastLetterPoint = point;
    renderLetterPreview();
  });
  els.letterCanvas.addEventListener("pointerup", () => {
    state.letterDrawing = false;
    state.lastLetterPoint = null;
    renderLetterPreview();
  });
  els.letterCanvas.addEventListener("pointercancel", () => {
    state.letterDrawing = false;
    state.lastLetterPoint = null;
  });

  els.chineseCanvas.addEventListener("pointerdown", (event) => {
    els.chineseCanvas.setPointerCapture(event.pointerId);
    state.chineseDrawing = true;
    const point = chinesePoint(event);
    state.lastChinesePoint = point;
    drawChineseDot(point);
  });
  els.chineseCanvas.addEventListener("pointermove", (event) => {
    if (!state.chineseDrawing || !state.lastChinesePoint) return;
    const point = chinesePoint(event);
    drawChineseLine(state.lastChinesePoint, point);
    state.lastChinesePoint = point;
  });
  els.chineseCanvas.addEventListener("pointerup", () => {
    state.chineseDrawing = false;
    state.lastChinesePoint = null;
  });
  els.chineseCanvas.addEventListener("pointercancel", () => {
    state.chineseDrawing = false;
    state.lastChinesePoint = null;
  });

  els.clearBtn.addEventListener("click", clearCanvas);
  els.clearBoardBtn.addEventListener("click", clearBoardCanvas);
  els.clearLetterBtn.addEventListener("click", clearLetterCanvas);
  els.clearChineseBtn.addEventListener("click", clearChineseCanvas);
  els.runChineseBtn.addEventListener("click", runChineseRecognition);
  els.probeChineseModelsBtn.addEventListener("click", probeChineseModels);
  els.refreshChineseStatusBtn.addEventListener("click", loadChineseStatus);
  els.chineseProviderSelect.addEventListener("change", () => {
    els.chineseModelInput.value = "";
    loadChineseStatus();
  });
  els.saveLetterBtn.addEventListener("click", saveLetterSample);
  els.saveBoardBtn.addEventListener("click", () => saveBoardSample(false));
  if (els.letterSaveBoardBtn) {
    els.letterSaveBoardBtn.addEventListener("click", () => saveBoardSample(false));
  }
  els.penBtn.addEventListener("click", () => els.penBtn.classList.add("active"));
  els.inferBtn.addEventListener("click", runInfer);
  els.saveBtn.addEventListener("click", saveSample);
  els.refreshStatusBtn.addEventListener("click", refreshStatus);
  els.homeRefreshBtn.addEventListener("click", refreshStatus);
  els.runDigitBatchTestBtn.addEventListener("click", () => runBatchTest("digit"));
  els.runLetterBatchTestBtn.addEventListener("click", () => runBatchTest("letter"));
  if (els.refreshTfCardBtn) {
    els.refreshTfCardBtn.addEventListener("click", refreshTfCardStatus);
  }
  if (els.refreshTfCardBtn2) {
    els.refreshTfCardBtn2.addEventListener("click", refreshTfCardStatus);
  }
  els.refreshPortsBtn.addEventListener("click", loadSerialPorts);
  els.languageBtn.addEventListener("click", toggleLanguage);
  els.serialBtn.addEventListener("click", connectSerial);
  els.serialPortSelect.addEventListener("change", () => {
    localStorage.setItem("digitnn-serial-port", els.serialPortSelect.value);
    syncSerialSelectOptions(els.serialPortSelect.value);
    syncSerialUi();
  });
  for (const select of els.sharedSerialSelects) {
    select.addEventListener("change", () => {
      if (select.value) {
        els.serialPortSelect.value = select.value;
        localStorage.setItem("digitnn-serial-port", select.value);
      }
      syncSerialUi();
    });
  }
  for (const button of els.sharedSerialButtons) {
    button.addEventListener("click", () => {
      selectSerialPortFromElement(button);
      connectSerial();
    });
  }
  for (const button of els.sharedRefreshPortsButtons) {
    button.addEventListener("click", loadSerialPorts);
  }
  for (const button of els.sharedClearBoardButtons) {
    button.addEventListener("click", clearBoardCanvas);
  }
  for (const button of document.querySelectorAll("[data-source-toggle]")) {
    button.addEventListener("click", () => {
      setNamedSourceMode(button.dataset.sourceToggle, button.dataset.sourceMode);
    });
  }
  els.mcuModeBtn.addEventListener("click", () => setSourceMode("mcu"));
  els.inputModeBtn.addEventListener("click", () => setSourceMode("input"));
  els.deskewCheck.addEventListener("change", runInfer);
  els.thickenCheck.addEventListener("change", runInfer);
  for (const button of document.querySelectorAll(".deploy-action")) {
    button.addEventListener("click", () => runDeploy(button.dataset.action));
  }
  for (const button of document.querySelectorAll(".letter-deploy-action")) {
    button.addEventListener("click", () => runLetterDeploy(button.dataset.action));
  }
  for (const button of document.querySelectorAll("[data-view]")) {
    button.addEventListener("click", () => setView(button.dataset.view));
  }
  for (const button of document.querySelectorAll("[data-model-explain]")) {
    button.addEventListener("click", () => setModelExplain(button.dataset.modelExplain));
  }
  window.addEventListener("resize", () => {
    if (state.currentView === "workspace") {
      initPixelGrid(state.pixelWidth, state.pixelHeight);
      if (state.sourceMode === "input") {
        scheduleInfer();
      } else if (state.lastBoardImage) {
        updatePixels(state.lastBoardImage.pixels, state.lastBoardImage.width, state.lastBoardImage.height);
      }
    }
    if (els.letterPixelGrid) {
      initLetterPixelGrid();
      renderLetterPreview();
    }
    if ((state.currentView === "workspace" && state.sourceMode === "mcu") || state.currentView === "letters" || state.currentView === "chinese") {
      window.clearTimeout(state.boardResizeTimer);
      state.boardResizeTimer = window.setTimeout(initBoardCanvas, 120);
    }
  });
}

function boot() {
  applyLanguage();
  applyView();
  applySourceMode();
  applyNamedSourceMode("letter");
  applyNamedSourceMode("chinese");
  applyModelExplain();
  initCanvas();
  initLetterPixelGrid();
  initLetterCanvas();
  initChineseCanvas();
  renderLetterModelTiles();
  initLetterLabels();
  initBoardCanvas();
  window.setTimeout(initBoardCanvas, 80);
  initPixelGrid();
  bindEvents();
  loadSerialPorts();
  refreshStatus();
  loadChineseStatus();
  if (state.currentView === "tfCard") {
    refreshTfCardStatus();
  }
  if (!state.usagePollTimer) {
    state.usagePollTimer = window.setInterval(refreshUsageOnly, 5000);
  }
}

boot();
