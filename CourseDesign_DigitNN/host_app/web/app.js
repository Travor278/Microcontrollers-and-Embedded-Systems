const state = {
  labels: [],
  models: [],
  drawing: false,
  lastPoint: null,
  boardLastPoint: null,
  boardPointCount: 0,
  boardStrokeCount: 0,
  lastBoardImage: null,
  boardImageSaved: false,
  boardAutoSaveTimer: null,
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
  deployBusy: false,
  lang: localStorage.getItem("digitnn-lang") || "zh",
};

const els = {
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
  serialPortSelect: document.getElementById("serialPortSelect"),
  refreshPortsBtn: document.getElementById("refreshPortsBtn"),
  serialBtn: document.getElementById("serialBtn"),
  serialLog: document.getElementById("serialLog"),
  deployModel: document.getElementById("deployModel"),
  epochsInput: document.getElementById("epochsInput"),
  batchInput: document.getElementById("batchInput"),
  uv4Input: document.getElementById("uv4Input"),
  augmentCheck: document.getElementById("augmentCheck"),
};

const ctx = els.drawCanvas.getContext("2d", { willReadFrequently: true });
const boardCtx = els.boardCanvas.getContext("2d", { willReadFrequently: true });
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
  if (state.serialConnected) {
    els.serialBtn.textContent = t("disconnect");
  } else {
    els.serialBtn.textContent = t("connectSerial");
  }
}

function toggleLanguage() {
  state.lang = state.lang === "zh" ? "en" : "zh";
  localStorage.setItem("digitnn-lang", state.lang);
  applyLanguage();
  renderModelTiles(state.models);
  setStatus(t("modelsReady", { ready: state.models.filter((item) => item.available).length, total: state.models.length }));
  runInfer();
}

function logLine(text, kind = "") {
  const prefix = new Date().toLocaleTimeString();
  els.serialLog.textContent += `[${prefix}] ${text}\n`;
  els.serialLog.scrollTop = els.serialLog.scrollHeight;
  if (kind === "error") {
    els.modelStatus.textContent = text;
  }
}

function setStatus(text) {
  els.modelStatus.textContent = text;
}

function initCanvas() {
  ctx.fillStyle = "#000000";
  ctx.fillRect(0, 0, els.drawCanvas.width, els.drawCanvas.height);
  ctx.lineCap = "round";
  ctx.lineJoin = "round";
  ctx.strokeStyle = "#ffffff";
  ctx.fillStyle = "#ffffff";
}

function initBoardCanvas() {
  boardCtx.fillStyle = "#0e1726";
  boardCtx.fillRect(0, 0, els.boardCanvas.width, els.boardCanvas.height);
  boardCtx.strokeStyle = "rgba(148, 163, 184, 0.18)";
  boardCtx.lineWidth = 1;
  for (let x = 0; x <= els.boardCanvas.width; x += 34) {
    boardCtx.beginPath();
    boardCtx.moveTo(x, 0);
    boardCtx.lineTo(x, els.boardCanvas.height);
    boardCtx.stroke();
  }
  for (let y = 0; y <= els.boardCanvas.height; y += 22) {
    boardCtx.beginPath();
    boardCtx.moveTo(0, y);
    boardCtx.lineTo(els.boardCanvas.width, y);
    boardCtx.stroke();
  }
  boardCtx.lineCap = "round";
  boardCtx.lineJoin = "round";
  boardCtx.strokeStyle = "#f8fafc";
  boardCtx.fillStyle = "#f8fafc";
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
  els.boardStats.textContent = t("boardStats", {
    points: state.boardPointCount,
    strokes: state.boardStrokeCount,
    x,
    y,
  });
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
    const payload = await apiJson("/api/save-board-sample", {
      method: "POST",
      body: JSON.stringify({
        width: state.lastBoardImage.width,
        height: state.lastBoardImage.height,
        pixels: state.lastBoardImage.pixels,
        label: els.labelSelect.value,
        outputDir: els.saveDirInput.value,
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
  els.mcuResults.innerHTML = "";
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

    const mcuTile = document.createElement("div");
    mcuTile.className = "mcu-tile";
    mcuTile.dataset.model = model.short;
    mcuTile.innerHTML = `
      <div class="result-name">${model.short}</div>
      <div class="result-label">--</div>
      <div class="result-meta">${t("mcuResult")}</div>
      <div class="bar"><div></div></div>
    `;
    els.mcuResults.appendChild(mcuTile);
  }
}

function updateUsage(usage) {
  const data = usage && usage.usage ? usage.usage : usage;
  if (!data || !data.available) {
    els.flashText.textContent = "--";
    els.sramText.textContent = "--";
    els.flashMeter.style.width = "0%";
    els.sramMeter.style.width = "0%";
    return;
  }
  els.flashText.textContent = `${(data.flash / 1024).toFixed(1)} KB`;
  els.sramText.textContent = `${(data.sram / 1024).toFixed(1)} KB`;
  els.flashMeter.style.width = `${Math.min(data.flashPercent, 100).toFixed(1)}%`;
  els.sramMeter.style.width = `${Math.min(data.sramPercent, 100).toFixed(1)}%`;
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
    if (!els.uv4Input.value && payload.uv4) {
      els.uv4Input.value = payload.uv4;
    }
    renderModelTiles(state.models);
    updateUsage(payload.usage);
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
  const tile = els.mcuResults.querySelector(`[data-model="${model}"]`);
  state.lastMcuResults[model] = {
    label: label || "",
    confidence: Number(confidence || 0),
    timeUs: timeUs || "",
  };
  if (!tile) return;
  const conf = Math.max(0, Math.min(100, Number(confidence || 0)));
  tile.querySelector(".result-label").textContent = label || "--";
  tile.querySelector(".result-meta").textContent = `${conf}%${timeUs ? `  ${timeUs} us` : ""}`;
  tile.querySelector(".bar > div").style.width = `${conf}%`;
}

function boardPointFromFrame(frame) {
  const rawX = Number(frame.x);
  const rawY = Number(frame.y);
  const width = BOARD_LCD.right - BOARD_LCD.left;
  const height = BOARD_LCD.bottom - BOARD_LCD.top;
  const ratioX = Math.max(0, Math.min(1, (rawX - BOARD_LCD.left) / width));
  const ratioY = Math.max(0, Math.min(1, (rawY - BOARD_LCD.top) / height));
  return {
    x: ratioX * els.boardCanvas.width,
    y: ratioY * els.boardCanvas.height,
  };
}

function drawBoardPoint(point) {
  boardCtx.beginPath();
  boardCtx.arc(point.x, point.y, 2.4, 0, Math.PI * 2);
  boardCtx.fill();
}

function drawBoardLine(from, to) {
  boardCtx.lineWidth = 4;
  boardCtx.beginPath();
  boardCtx.moveTo(from.x, from.y);
  boardCtx.lineTo(to.x, to.y);
  boardCtx.stroke();
  drawBoardPoint(to);
}

function handleBoardPoint(frame) {
  const point = boardPointFromFrame(frame);
  const rawX = Number(frame.x);
  const rawY = Number(frame.y);
  if (state.boardLastPoint) {
    drawBoardLine(state.boardLastPoint, point);
  } else {
    drawBoardPoint(point);
  }
  state.boardLastPoint = point;
  state.boardPointCount += 1;
  updateBoardStats(rawX, rawY);
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

function handleSerialLine(line) {
  if (!line) return;
  const frame = parseFrame(line);
  if (frame.type === "IMAGE") {
    logLine(`IMAGE,w=${frame.w || "?"},h=${frame.h || "?"},data=<${(frame.data || "").length} hex>`);
  } else if (frame.type !== "POINT") {
    logLine(line);
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
      setStatus(t("boardImageUpdated"));
      if (els.autoSaveBoardCheck.checked) {
        window.clearTimeout(state.boardAutoSaveTimer);
        state.boardAutoSaveTimer = window.setTimeout(() => saveBoardSample(true), 350);
      }
    } catch (error) {
      logLine(String(error.message || error), "error");
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
      logLine(String(error.message || error), "error");
    }
    state.serialConnected = false;
    state.serialLastId = 0;
    els.serialBtn.textContent = t("connectSerial");
    logLine(t("serialClosed"));
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
    els.serialBtn.textContent = t("disconnect");
    logLine(t("serialOpenedWithPort", { port }));
    pollSerialLoop();
  } catch (error) {
    logLine(String(error.message || error), "error");
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
    els.serialBtn.textContent = t("connectSerial");
    logLine(t("serialReleasedForFlash", { port }));
    return port;
  } catch (error) {
    logLine(String(error.message || error), "error");
    return "";
  }
}

async function reconnectSerialAfterDeploy(port) {
  if (!port) return;
  await new Promise((resolve) => window.setTimeout(resolve, 900));
  if ([...els.serialPortSelect.options].some((option) => option.value === port)) {
    els.serialPortSelect.value = port;
  }
  logLine(t("serialReconnectAfterFlash", { port }));
  await connectSerial();
}

async function loadSerialPorts() {
  try {
    const previous = els.serialPortSelect.value;
    const payload = await apiJson("/api/serial/ports");
    const ports = payload.ports || [];
    els.serialPortSelect.innerHTML = "";
    for (const port of ports) {
      const option = document.createElement("option");
      option.value = port.device;
      option.textContent = port.label || port.device;
      els.serialPortSelect.appendChild(option);
    }
    const preferred = ports.find((port) => /CH340|USB-SERIAL|USB TO UART/i.test(`${port.label} ${port.hwid}`));
    if (previous && ports.some((port) => port.device === previous)) {
      els.serialPortSelect.value = previous;
    } else if (preferred) {
      els.serialPortSelect.value = preferred.device;
    }
    setStatus(ports.length ? t("serialPortsLoaded", { count: ports.length }) : t("noSerialPorts"));
  } catch (error) {
    logLine(String(error.message || error), "error");
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
      logLine(String(payload.status.error), "error");
    }
    if (payload.status && payload.status.open === false) {
      state.serialConnected = false;
      els.serialBtn.textContent = t("connectSerial");
      return;
    }
  } catch (error) {
    logLine(String(error.message || error), "error");
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
    logLine(t("webSerialUnavailable"), "error");
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
      logLine(String(error.message || error), "error");
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

function setDeployDisabled(disabled) {
  for (const button of document.querySelectorAll(".deploy-action")) {
    button.disabled = disabled;
  }
}

function initPixelGrid(width = 28, height = 28) {
  state.pixelWidth = width;
  state.pixelHeight = height;
  els.pixelGrid.innerHTML = "";
  const pixelSize = width <= 32 ? 12 : 7;
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

  els.clearBtn.addEventListener("click", clearCanvas);
  els.clearBoardBtn.addEventListener("click", clearBoardCanvas);
  els.saveBoardBtn.addEventListener("click", () => saveBoardSample(false));
  els.penBtn.addEventListener("click", () => els.penBtn.classList.add("active"));
  els.inferBtn.addEventListener("click", runInfer);
  els.saveBtn.addEventListener("click", saveSample);
  els.refreshStatusBtn.addEventListener("click", refreshStatus);
  els.refreshPortsBtn.addEventListener("click", loadSerialPorts);
  els.languageBtn.addEventListener("click", toggleLanguage);
  els.serialBtn.addEventListener("click", connectSerial);
  els.deskewCheck.addEventListener("change", runInfer);
  els.thickenCheck.addEventListener("change", runInfer);
  for (const button of document.querySelectorAll(".deploy-action")) {
    button.addEventListener("click", () => runDeploy(button.dataset.action));
  }
}

function boot() {
  applyLanguage();
  initCanvas();
  initBoardCanvas();
  initPixelGrid();
  bindEvents();
  loadSerialPorts();
  refreshStatus();
}

boot();
