"""Realtime drawing, inference, serial, and sample-capture UI for DigitNN."""

from __future__ import annotations

import argparse
import csv
import os
import re
import subprocess
import sys
import threading
from datetime import datetime
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


ROOT_DIR = Path(__file__).resolve().parents[1]
TOOLS_DIR = ROOT_DIR / "tools"
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

from host_batch_test import default_quant_file, predict_scores  # noqa: E402


DRAW_SIZE = 320
DIGIT_SIZE = 28
PIXEL_CELL = 9
FLASH_BYTES = 512 * 1024
SRAM_BYTES = 64 * 1024
MODEL_SPECS = [
    ("perceptron", "P", "Perceptron"),
    ("fnn", "F", "FNN"),
    ("cnn", "C", "Tiny-CNN"),
]
CLASS_LABELS = [str(item) for item in range(10)] + [chr(ord("A") + item) for item in range(26)]


def parse_frame(line: str) -> dict[str, str]:
    parts = line.strip().split(",")
    frame: dict[str, str] = {"type": parts[0].strip() if parts else ""}
    for part in parts[1:]:
        if "=" not in part:
            continue
        key, value = part.split("=", maxsplit=1)
        frame[key.strip()] = value.strip()
    return frame


def confidence_from_scores(scores: np.ndarray) -> tuple[int, int, int]:
    if scores.size == 0:
        return 0, 0, 0

    order = np.argsort(scores)
    best_index = int(order[-1])
    second_index = int(order[-2]) if scores.size > 1 else best_index
    best_score = int(scores[best_index])
    second_score = int(scores[second_index])
    diff = max(best_score - second_score, 0)
    magnitude = max(abs(best_score), abs(second_score), 1)
    confidence = int(round((diff * 100.0) / (magnitude + diff)))
    return best_index, min(max(confidence, 0), 100), second_index


def load_quantized_models() -> dict[str, dict[str, np.ndarray]]:
    loaded: dict[str, dict[str, np.ndarray]] = {}
    for model, _, _ in MODEL_SPECS:
        path = default_quant_file(model)
        if not path.exists():
            continue
        with np.load(path) as data:
            loaded[model] = {key: np.array(data[key]) for key in data.files}
    return loaded


def parse_build_usage() -> dict[str, int] | None:
    build_log = ROOT_DIR / "keil_touch_digit_nn" / "Output" / "DigitNN_Touch.build_log.htm"
    if not build_log.exists():
        return None

    text = build_log.read_text(encoding="utf-8", errors="ignore")
    match = re.search(
        r"Program Size:\s*Code=(\d+)\s*RO-data=(\d+)\s*RW-data=(\d+)\s*ZI-data=(\d+)",
        text,
    )
    if not match:
        return None

    code, ro_data, rw_data, zi_data = (int(item) for item in match.groups())
    return {
        "code": code,
        "ro_data": ro_data,
        "rw_data": rw_data,
        "zi_data": zi_data,
        "flash": code + ro_data + rw_data,
        "sram": rw_data + zi_data,
    }


def center_and_deskew(image: Image.Image, enabled: bool) -> Image.Image:
    arr = np.asarray(image, dtype=np.float32)
    total = float(arr.sum())
    if total < 1.0:
        return image

    yy, xx = np.indices(arr.shape)
    center_x = float((xx * arr).sum() / total)
    center_y = float((yy * arr).sum() / total)
    shift_x = (DIGIT_SIZE - 1) / 2.0 - center_x
    shift_y = (DIGIT_SIZE - 1) / 2.0 - center_y

    skew = 0.0
    if enabled:
        x = xx - center_x
        y = yy - center_y
        mu02 = float((y * y * arr).sum() / total)
        mu11 = float((x * y * arr).sum() / total)
        if abs(mu02) > 1e-3:
            skew = mu11 / mu02
            skew = max(min(skew, 0.45), -0.45)

    return image.transform(
        (DIGIT_SIZE, DIGIT_SIZE),
        Image.Transform.AFFINE,
        (1.0, -skew, skew * (DIGIT_SIZE - 1) / 2.0 - shift_x, 0.0, 1.0, -shift_y),
        resample=Image.Resampling.BICUBIC,
        fillcolor=0,
    )


def preprocess_canvas_image(raw: Image.Image, thicken: bool, deskew: bool) -> Image.Image:
    bbox = raw.getbbox()
    if bbox is None:
        return Image.new("L", (DIGIT_SIZE, DIGIT_SIZE), 0)

    digit = raw.crop(bbox)
    digit.thumbnail((20, 20), Image.Resampling.LANCZOS)

    canvas = Image.new("L", (DIGIT_SIZE, DIGIT_SIZE), 0)
    x_offset = (DIGIT_SIZE - digit.width) // 2
    y_offset = (DIGIT_SIZE - digit.height) // 2
    canvas.paste(digit, (x_offset, y_offset))

    canvas = center_and_deskew(canvas, deskew)
    if thicken:
        canvas = canvas.filter(ImageFilter.MaxFilter(3))
    return canvas


class RealtimeDigitUI:
    def __init__(self, root: tk.Tk, serial_port: str | None, baud: int) -> None:
        self.root = root
        self.root.title("DigitNN Realtime Dashboard")
        self.root.minsize(1100, 720)

        self.models = load_quantized_models()
        self.raw_image = Image.new("L", (DRAW_SIZE, DRAW_SIZE), 0)
        self.raw_draw = ImageDraw.Draw(self.raw_image)
        self.digit_image = Image.new("L", (DIGIT_SIZE, DIGIT_SIZE), 0)
        self.last_point: tuple[int, int] | None = None
        self.infer_after_id: str | None = None
        self.serial = None
        self.serial_buffer = ""
        self.serial_module = None

        self.brush_size = tk.IntVar(value=18)
        self.label_var = tk.StringVar(value="0")
        self.thicken_var = tk.BooleanVar(value=False)
        self.deskew_var = tk.BooleanVar(value=True)
        self.status_var = tk.StringVar(value="Ready")
        self.port_var = tk.StringVar(value=serial_port or "")
        self.baud_var = tk.IntVar(value=baud)
        self.save_dir_var = tk.StringVar(value=str(ROOT_DIR / "tf_card" / "ui_collected"))
        self.deploy_model_var = tk.StringVar(value="fnn")
        self.deploy_epochs_var = tk.IntVar(value=3)
        self.deploy_batch_var = tk.IntVar(value=512)
        self.uv4_path_var = tk.StringVar(value=os.environ.get("KEIL_UV4", ""))
        self.deploy_busy = False
        self.pc_result_vars: dict[str, dict[str, tk.StringVar | tk.DoubleVar]] = {}
        self.mcu_result_vars: dict[str, dict[str, tk.StringVar | tk.DoubleVar]] = {}
        self.usage_vars: dict[str, tk.StringVar | tk.DoubleVar] = {}
        self.pixel_cells: list[int] = []
        self.serial_log: tk.Text | None = None
        self.port_combo: ttk.Combobox | None = None

        self._configure_style()
        self._build_layout()
        self._refresh_ports()
        self._update_pixel_grid()
        self._update_usage()
        self._infer_current_digit()
        self._poll_serial()

    def _configure_style(self) -> None:
        style = ttk.Style()
        if "clam" in style.theme_names():
            style.theme_use("clam")
        style.configure("TFrame", background="#eef2f5")
        style.configure("Panel.TFrame", background="#ffffff", relief="solid", borderwidth=1)
        style.configure("TLabel", background="#eef2f5", foreground="#18202a")
        style.configure("Panel.TLabel", background="#ffffff", foreground="#18202a")
        style.configure("Title.TLabel", background="#ffffff", foreground="#111827", font=("Segoe UI", 13, "bold"))
        style.configure("Value.TLabel", background="#ffffff", foreground="#111827", font=("Consolas", 14, "bold"))
        style.configure("Small.TLabel", background="#ffffff", foreground="#526173")
        style.configure("TButton", padding=(10, 5))
        style.configure("Horizontal.TProgressbar", troughcolor="#d9e2ec", background="#2563eb")

    def _build_layout(self) -> None:
        outer = ttk.Frame(self.root, padding=12)
        outer.grid(row=0, column=0, sticky="nsew")
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        outer.columnconfigure(1, weight=1)
        outer.rowconfigure(0, weight=1)

        left = ttk.Frame(outer, style="Panel.TFrame", padding=12)
        left.grid(row=0, column=0, sticky="ns", padx=(0, 10))
        middle = ttk.Frame(outer, style="Panel.TFrame", padding=12)
        middle.grid(row=0, column=1, sticky="nsew", padx=(0, 10))
        right = ttk.Frame(outer, style="Panel.TFrame", padding=12)
        right.grid(row=0, column=2, sticky="ns")

        self._build_drawing_panel(left)
        self._build_pixel_panel(middle)
        self._build_result_panel(right)

    def _build_drawing_panel(self, parent: ttk.Frame) -> None:
        ttk.Label(parent, text="Draw Input", style="Title.TLabel").grid(row=0, column=0, columnspan=3, sticky="w")
        self.draw_canvas = tk.Canvas(parent, width=DRAW_SIZE, height=DRAW_SIZE, bg="#101820", highlightthickness=1, highlightbackground="#9aa7b3")
        self.draw_canvas.grid(row=1, column=0, columnspan=3, pady=(10, 8))
        self.draw_canvas.bind("<ButtonPress-1>", self._on_draw_start)
        self.draw_canvas.bind("<B1-Motion>", self._on_draw_motion)
        self.draw_canvas.bind("<ButtonRelease-1>", self._on_draw_end)

        ttk.Button(parent, text="Clear", command=self._clear_canvas).grid(row=2, column=0, sticky="ew", padx=(0, 6))
        ttk.Button(parent, text="Recognize", command=self._infer_current_digit).grid(row=2, column=1, sticky="ew", padx=3)
        ttk.Button(parent, text="Save Sample", command=self._save_sample).grid(row=2, column=2, sticky="ew", padx=(6, 0))

        ttk.Label(parent, text="Label", style="Panel.TLabel").grid(row=3, column=0, sticky="w", pady=(12, 0))
        ttk.Combobox(parent, textvariable=self.label_var, values=CLASS_LABELS, width=6, state="readonly").grid(
            row=3,
            column=1,
            sticky="ew",
            pady=(12, 0),
        )
        ttk.Label(parent, text="Brush", style="Panel.TLabel").grid(row=4, column=0, sticky="w", pady=(10, 0))
        ttk.Scale(parent, from_=8, to=34, variable=self.brush_size, orient="horizontal").grid(row=4, column=1, columnspan=2, sticky="ew", pady=(10, 0))

        ttk.Checkbutton(parent, text="Auto deskew", variable=self.deskew_var, command=self._refresh_from_options).grid(row=5, column=0, sticky="w", pady=(10, 0))
        ttk.Checkbutton(parent, text="Thicken stroke", variable=self.thicken_var, command=self._refresh_from_options).grid(row=5, column=1, columnspan=2, sticky="w", pady=(10, 0))

        ttk.Label(parent, text="Save directory", style="Panel.TLabel").grid(row=6, column=0, columnspan=3, sticky="w", pady=(12, 0))
        ttk.Entry(parent, textvariable=self.save_dir_var, width=42).grid(row=7, column=0, columnspan=3, sticky="ew", pady=(4, 0))
        ttk.Label(parent, textvariable=self.status_var, style="Small.TLabel", wraplength=320).grid(row=8, column=0, columnspan=3, sticky="ew", pady=(12, 0))

        for column in range(3):
            parent.columnconfigure(column, weight=1)

    def _build_pixel_panel(self, parent: ttk.Frame) -> None:
        parent.columnconfigure(0, weight=1)
        ttk.Label(parent, text="28x28 Pixel View", style="Title.TLabel").grid(row=0, column=0, sticky="w")
        self.pixel_canvas = tk.Canvas(
            parent,
            width=DIGIT_SIZE * PIXEL_CELL,
            height=DIGIT_SIZE * PIXEL_CELL,
            bg="#0d1321",
            highlightthickness=1,
            highlightbackground="#9aa7b3",
        )
        self.pixel_canvas.grid(row=1, column=0, pady=(10, 12))
        for y in range(DIGIT_SIZE):
            for x in range(DIGIT_SIZE):
                rect = self.pixel_canvas.create_rectangle(
                    x * PIXEL_CELL,
                    y * PIXEL_CELL,
                    (x + 1) * PIXEL_CELL,
                    (y + 1) * PIXEL_CELL,
                    fill="#0d1321",
                    outline="#172033",
                )
                self.pixel_cells.append(rect)

        ttk.Label(parent, text="Serial Monitor", style="Title.TLabel").grid(row=2, column=0, sticky="w", pady=(6, 0))
        serial_frame = ttk.Frame(parent, style="Panel.TFrame")
        serial_frame.grid(row=3, column=0, sticky="ew", pady=(8, 6))
        serial_frame.columnconfigure(1, weight=1)

        ttk.Button(serial_frame, text="Refresh", command=self._refresh_ports).grid(row=0, column=0, padx=(0, 6))
        self.port_combo = ttk.Combobox(serial_frame, textvariable=self.port_var, width=12)
        self.port_combo.grid(row=0, column=1, sticky="ew", padx=(0, 6))
        ttk.Entry(serial_frame, textvariable=self.baud_var, width=8).grid(row=0, column=2, padx=(0, 6))
        ttk.Button(serial_frame, text="Open/Close", command=self._toggle_serial).grid(row=0, column=3)

        command_frame = ttk.Frame(parent, style="Panel.TFrame")
        command_frame.grid(row=4, column=0, sticky="ew")
        for index, command in enumerate(["CMD,INFO", "CMD,CLEAR", "CMD,MODEL,P", "CMD,MODEL,F", "CMD,MODEL,C"]):
            ttk.Button(command_frame, text=command.replace("CMD,", ""), command=lambda value=command: self._send_serial(value)).grid(
                row=0,
                column=index,
                sticky="ew",
                padx=2,
            )
            command_frame.columnconfigure(index, weight=1)

        self.serial_log = tk.Text(parent, height=9, width=52, bg="#111827", fg="#d8dee9", insertbackground="#ffffff", relief="flat")
        self.serial_log.grid(row=5, column=0, sticky="nsew", pady=(8, 0))
        parent.rowconfigure(5, weight=1)

    def _build_result_panel(self, parent: ttk.Frame) -> None:
        parent.columnconfigure(0, weight=1)
        ttk.Label(parent, text="Recognition Results", style="Title.TLabel").grid(row=0, column=0, sticky="w")

        table = ttk.Frame(parent, style="Panel.TFrame")
        table.grid(row=1, column=0, sticky="ew", pady=(10, 14))
        for column, title in enumerate(["Model", "PC Quant", "MCU Serial"]):
            ttk.Label(table, text=title, style="Small.TLabel").grid(row=0, column=column, sticky="w", padx=4, pady=(0, 6))

        for row, (model, short_name, display_name) in enumerate(MODEL_SPECS, start=1):
            ttk.Label(table, text=f"{short_name} {display_name}", style="Panel.TLabel").grid(row=row, column=0, sticky="w", padx=4, pady=4)
            pc_label = tk.StringVar(value="--")
            pc_conf = tk.DoubleVar(value=0.0)
            mcu_label = tk.StringVar(value="--")
            mcu_conf = tk.DoubleVar(value=0.0)
            self.pc_result_vars[model] = {"label": pc_label, "conf": pc_conf}
            self.mcu_result_vars[short_name] = {"label": mcu_label, "conf": mcu_conf}
            self._result_cell(table, row, 1, pc_label, pc_conf)
            self._result_cell(table, row, 2, mcu_label, mcu_conf)

        ttk.Label(parent, text="MCU Resource", style="Title.TLabel").grid(row=2, column=0, sticky="w")
        usage = ttk.Frame(parent, style="Panel.TFrame")
        usage.grid(row=3, column=0, sticky="ew", pady=(10, 14))
        self._usage_row(usage, 0, "Flash", "flash")
        self._usage_row(usage, 1, "SRAM", "sram")
        ttk.Button(usage, text="Reload Build Usage", command=self._update_usage).grid(row=2, column=0, columnspan=3, sticky="ew", pady=(8, 0))

        ttk.Label(parent, text="Firmware Deploy", style="Title.TLabel").grid(row=4, column=0, sticky="w")
        deploy = ttk.Frame(parent, style="Panel.TFrame")
        deploy.grid(row=5, column=0, sticky="ew", pady=(10, 14))
        self._build_deploy_panel(deploy)

        ttk.Label(parent, text="Task Evidence", style="Title.TLabel").grid(row=6, column=0, sticky="w")
        evidence = ttk.Frame(parent, style="Panel.TFrame")
        evidence.grid(row=7, column=0, sticky="ew", pady=(10, 14))
        items = [
            ("Digits 0-9", "Enabled"),
            ("Quantized P/F/C", "Enabled"),
            ("TF-card sets", "210 BMPs"),
            ("Realtime capture", "Enabled"),
            ("Serial RESULT frames", "Enabled"),
            ("Letters", "PC prototype"),
        ]
        for row, (name, value) in enumerate(items):
            ttk.Label(evidence, text=name, style="Panel.TLabel").grid(row=row, column=0, sticky="w", padx=4, pady=3)
            ttk.Label(evidence, text=value, style="Value.TLabel").grid(row=row, column=1, sticky="e", padx=4, pady=3)
        evidence.columnconfigure(1, weight=1)

        ttk.Label(
            parent,
            text="PC confidence uses the same score-margin idea as firmware. Serial values are board-reported when connected.",
            style="Small.TLabel",
            wraplength=330,
        ).grid(row=8, column=0, sticky="ew")

    def _build_deploy_panel(self, parent: ttk.Frame) -> None:
        parent.columnconfigure(1, weight=1)
        ttk.Label(parent, text="Model", style="Panel.TLabel").grid(row=0, column=0, sticky="w", padx=4, pady=3)
        ttk.Combobox(
            parent,
            textvariable=self.deploy_model_var,
            values=["perceptron", "fnn", "cnn"],
            state="readonly",
            width=12,
        ).grid(row=0, column=1, columnspan=2, sticky="ew", padx=4, pady=3)

        ttk.Label(parent, text="Epochs", style="Panel.TLabel").grid(row=1, column=0, sticky="w", padx=4, pady=3)
        ttk.Spinbox(parent, from_=1, to=30, textvariable=self.deploy_epochs_var, width=7).grid(row=1, column=1, sticky="w", padx=4, pady=3)
        ttk.Label(parent, text="Batch", style="Panel.TLabel").grid(row=1, column=2, sticky="e", padx=4, pady=3)
        ttk.Spinbox(parent, from_=64, to=2048, increment=64, textvariable=self.deploy_batch_var, width=7).grid(row=1, column=3, sticky="e", padx=4, pady=3)

        ttk.Label(parent, text="UV4.exe", style="Panel.TLabel").grid(row=2, column=0, sticky="w", padx=4, pady=3)
        ttk.Entry(parent, textvariable=self.uv4_path_var, width=28).grid(row=2, column=1, columnspan=2, sticky="ew", padx=4, pady=3)
        ttk.Button(parent, text="Browse", command=self._browse_uv4).grid(row=2, column=3, sticky="ew", padx=4, pady=3)

        ttk.Button(parent, text="Export", command=lambda: self._run_deploy("export-model")).grid(row=3, column=0, sticky="ew", padx=3, pady=(8, 3))
        ttk.Button(parent, text="Build", command=lambda: self._run_deploy("build")).grid(row=3, column=1, sticky="ew", padx=3, pady=(8, 3))
        ttk.Button(parent, text="Flash", command=lambda: self._run_deploy("flash")).grid(row=3, column=2, sticky="ew", padx=3, pady=(8, 3))
        ttk.Button(parent, text="Export+Flash", command=lambda: self._run_deploy("export-build-flash")).grid(row=3, column=3, sticky="ew", padx=3, pady=(8, 3))

        ttk.Label(
            parent,
            text="Export refreshes selected model weights, then Keil build/flash downloads the full firmware.",
            style="Small.TLabel",
            wraplength=320,
        ).grid(row=4, column=0, columnspan=4, sticky="ew", padx=4, pady=(6, 2))

    def _result_cell(self, parent: ttk.Frame, row: int, column: int, label_var: tk.StringVar, conf_var: tk.DoubleVar) -> None:
        cell = ttk.Frame(parent, style="Panel.TFrame")
        cell.grid(row=row, column=column, sticky="ew", padx=4, pady=4)
        ttk.Label(cell, textvariable=label_var, style="Value.TLabel").grid(row=0, column=0, sticky="w")
        ttk.Progressbar(cell, variable=conf_var, maximum=100.0, length=96).grid(row=1, column=0, sticky="ew", pady=(3, 0))
        cell.columnconfigure(0, weight=1)
        parent.columnconfigure(column, weight=1)

    def _usage_row(self, parent: ttk.Frame, row: int, title: str, key: str) -> None:
        text_var = tk.StringVar(value="--")
        value_var = tk.DoubleVar(value=0.0)
        self.usage_vars[f"{key}_text"] = text_var
        self.usage_vars[f"{key}_value"] = value_var
        ttk.Label(parent, text=title, style="Panel.TLabel").grid(row=row, column=0, sticky="w", padx=4, pady=4)
        ttk.Progressbar(parent, variable=value_var, maximum=100.0, length=180).grid(row=row, column=1, sticky="ew", padx=6, pady=4)
        ttk.Label(parent, textvariable=text_var, style="Small.TLabel").grid(row=row, column=2, sticky="e", padx=4, pady=4)
        parent.columnconfigure(1, weight=1)

    def _on_draw_start(self, event: tk.Event) -> None:
        self.last_point = (int(event.x), int(event.y))
        self._draw_dot(int(event.x), int(event.y))
        self._refresh_from_options()

    def _on_draw_motion(self, event: tk.Event) -> None:
        point = (int(event.x), int(event.y))
        if self.last_point is None:
            self.last_point = point
        width = int(self.brush_size.get())
        self.draw_canvas.create_line(*self.last_point, *point, fill="#f8fafc", width=width, capstyle=tk.ROUND, smooth=True)
        self.raw_draw.line([self.last_point, point], fill=255, width=width)
        self._draw_dot(point[0], point[1])
        self.last_point = point
        self._refresh_from_options(debounce=True)

    def _on_draw_end(self, _event: tk.Event) -> None:
        self.last_point = None
        self._refresh_from_options()

    def _draw_dot(self, x: int, y: int) -> None:
        radius = max(2, int(self.brush_size.get()) // 2)
        self.draw_canvas.create_oval(x - radius, y - radius, x + radius, y + radius, fill="#f8fafc", outline="")
        self.raw_draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=255)

    def _refresh_from_options(self, debounce: bool = False) -> None:
        self.digit_image = preprocess_canvas_image(self.raw_image, self.thicken_var.get(), self.deskew_var.get())
        self._update_pixel_grid()
        if debounce:
            self._schedule_infer()
        else:
            self._infer_current_digit()

    def _schedule_infer(self) -> None:
        if self.infer_after_id is not None:
            self.root.after_cancel(self.infer_after_id)
        self.infer_after_id = self.root.after(180, self._infer_current_digit)

    def _clear_canvas(self) -> None:
        self.draw_canvas.delete("all")
        self.raw_image = Image.new("L", (DRAW_SIZE, DRAW_SIZE), 0)
        self.raw_draw = ImageDraw.Draw(self.raw_image)
        self.digit_image = Image.new("L", (DIGIT_SIZE, DIGIT_SIZE), 0)
        self._update_pixel_grid()
        for variables in self.pc_result_vars.values():
            variables["label"].set("--")  # type: ignore[union-attr]
            variables["conf"].set(0.0)  # type: ignore[union-attr]
        self.status_var.set("Canvas cleared")

    def _update_pixel_grid(self) -> None:
        arr = np.asarray(self.digit_image, dtype=np.uint8)
        for index, value in enumerate(arr.reshape(-1)):
            level = int(value)
            if level <= 0:
                color = "#0d1321"
            else:
                color = f"#{level:02x}{level:02x}{level:02x}"
            self.pixel_canvas.itemconfigure(self.pixel_cells[index], fill=color)

    def _infer_current_digit(self) -> None:
        self.infer_after_id = None
        pixels = np.asarray(self.digit_image, dtype=np.uint8).reshape(-1)
        if int(pixels.max()) == 0:
            self.status_var.set("Draw a digit to run realtime inference")
            return

        if not self.models:
            self.status_var.set("No quantized model files found in CourseDesign_DigitNN/models")
            return

        for model, short_name, _display_name in MODEL_SPECS:
            variables = self.pc_result_vars[model]
            data = self.models.get(model)
            if data is None:
                variables["label"].set(f"{short_name}: missing")  # type: ignore[union-attr]
                variables["conf"].set(0.0)  # type: ignore[union-attr]
                continue
            try:
                scores = predict_scores(model, pixels, data)  # type: ignore[arg-type]
                label, confidence, second = confidence_from_scores(scores)
                variables["label"].set(f"{label}  {confidence}%  alt:{second}")  # type: ignore[union-attr]
                variables["conf"].set(float(confidence))  # type: ignore[union-attr]
            except Exception as exc:  # pragma: no cover - UI guard
                variables["label"].set(f"error: {exc}")  # type: ignore[union-attr]
                variables["conf"].set(0.0)  # type: ignore[union-attr]
        self.status_var.set("Realtime inference updated")

    def _save_sample(self) -> None:
        label = self.label_var.get().strip()
        if label not in set(CLASS_LABELS):
            messagebox.showerror("Invalid label", "Choose a label from 0-9 or A-Z.")
            return

        self._refresh_from_options()
        if int(np.asarray(self.digit_image, dtype=np.uint8).max()) == 0:
            messagebox.showwarning("Empty input", "Draw a digit before saving.")
            return

        output_dir = Path(self.save_dir_var.get()).expanduser()
        output_dir.mkdir(parents=True, exist_ok=True)
        stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        bmp_name = f"ui_{stamp}_{label}.bmp"
        raw_name = f"ui_{stamp}_{label}_raw.png"
        self.digit_image.save(output_dir / bmp_name)
        self.raw_image.save(output_dir / raw_name)

        label_file = output_dir / "label.txt"
        with label_file.open("a", encoding="utf-8", newline="") as handle:
            handle.write(f"{bmp_name},{label}\n")

        log_file = output_dir / "capture_log.csv"
        is_new = not log_file.exists()
        with log_file.open("a", newline="", encoding="utf-8") as handle:
            writer = csv.DictWriter(handle, fieldnames=["time", "filename", "raw_filename", "label", "pc_p", "pc_f", "pc_c"])
            if is_new:
                writer.writeheader()
            writer.writerow(
                {
                    "time": datetime.now().isoformat(timespec="seconds"),
                    "filename": bmp_name,
                    "raw_filename": raw_name,
                    "label": label,
                    "pc_p": self.pc_result_vars["perceptron"]["label"].get(),  # type: ignore[union-attr]
                    "pc_f": self.pc_result_vars["fnn"]["label"].get(),  # type: ignore[union-attr]
                    "pc_c": self.pc_result_vars["cnn"]["label"].get(),  # type: ignore[union-attr]
                }
            )
        self.status_var.set(f"Saved {bmp_name} and label.txt entry")

    def _refresh_ports(self) -> None:
        ports: list[str] = []
        try:
            import serial
            import serial.tools.list_ports

            self.serial_module = serial
            ports = [port.device for port in serial.tools.list_ports.comports()]
        except Exception:
            self.serial_module = None

        if self.port_combo is not None:
            self.port_combo["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def _toggle_serial(self) -> None:
        if self.serial is not None:
            self.serial.close()
            self.serial = None
            self._append_serial_log("Serial closed")
            return

        if self.serial_module is None:
            messagebox.showerror("pyserial missing", "Install pyserial with: python -m pip install pyserial")
            return

        try:
            self.serial = self.serial_module.Serial(self.port_var.get(), int(self.baud_var.get()), timeout=0)
            self._append_serial_log(f"Opened {self.port_var.get()} at {self.baud_var.get()}")
        except Exception as exc:
            messagebox.showerror("Serial open failed", str(exc))

    def _poll_serial(self) -> None:
        if self.serial is not None:
            try:
                chunk = self.serial.read(4096)
                if chunk:
                    self.serial_buffer += chunk.decode("utf-8", errors="replace")
                    while "\n" in self.serial_buffer:
                        line, self.serial_buffer = self.serial_buffer.split("\n", maxsplit=1)
                        self._handle_serial_line(line.strip())
            except Exception as exc:
                self._append_serial_log(f"Serial error: {exc}")
                try:
                    self.serial.close()
                except Exception:
                    pass
                self.serial = None
        self.root.after(80, self._poll_serial)

    def _send_serial(self, command: str) -> None:
        if self.serial is None:
            self._append_serial_log(f"Not connected: {command}")
            return
        self.serial.write((command + "\r\n").encode("ascii"))
        self._append_serial_log(f"> {command}")

    def _browse_uv4(self) -> None:
        path = filedialog.askopenfilename(
            title="Select UV4.exe",
            filetypes=[("Keil uVision", "UV4.exe"), ("Executable", "*.exe"), ("All files", "*.*")],
        )
        if path:
            self.uv4_path_var.set(path)

    def _run_deploy(self, action: str) -> None:
        if self.deploy_busy:
            self._append_serial_log("Deploy task is already running")
            return

        command = [
            sys.executable,
            str(ROOT_DIR / "tools" / "keil_flash.py"),
            "--action",
            action,
            "--model",
            self.deploy_model_var.get(),
            "--epochs",
            str(int(self.deploy_epochs_var.get())),
            "--batch-size",
            str(int(self.deploy_batch_var.get())),
            "--augment",
        ]
        uv4_path = self.uv4_path_var.get().strip()
        if uv4_path:
            command.extend(["--uv4", uv4_path])

        self.deploy_busy = True
        self.status_var.set(f"Running deploy action: {action}")
        self._append_serial_log("> " + " ".join(f'"{item}"' if " " in item else item for item in command))
        worker = threading.Thread(target=self._deploy_worker, args=(command, action), daemon=True)
        worker.start()

    def _deploy_worker(self, command: list[str], action: str) -> None:
        exit_code = 1
        try:
            process = subprocess.Popen(
                command,
                cwd=ROOT_DIR,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding="utf-8",
                errors="replace",
            )
            assert process.stdout is not None
            for line in process.stdout:
                self.root.after(0, self._append_serial_log, line.rstrip())
            exit_code = int(process.wait())
        except Exception as exc:  # pragma: no cover - UI guard
            self.root.after(0, self._append_serial_log, f"Deploy error: {exc}")
        finally:
            self.root.after(0, self._finish_deploy, action, exit_code)

    def _finish_deploy(self, action: str, exit_code: int) -> None:
        self.deploy_busy = False
        if exit_code == 0:
            self.status_var.set(f"Deploy action finished: {action}")
            self._append_serial_log(f"{action} finished")
            if "export" in action:
                self.models = load_quantized_models()
                self._infer_current_digit()
            if "build" in action:
                self._update_usage()
        else:
            self.status_var.set(f"Deploy action failed: {action}, exit={exit_code}")
            self._append_serial_log(f"{action} failed, exit={exit_code}")

    def _handle_serial_line(self, line: str) -> None:
        if not line:
            return
        self._append_serial_log(line)
        frame = parse_frame(line)
        if frame.get("type") == "RESULT":
            model = frame.get("model", "")
            label = frame.get("label", "--")
            confidence = frame.get("confidence", "0")
            time_us = frame.get("time_us", "")
            self._update_mcu_result(model, label, confidence, time_us)
            return

        legacy = re.search(
            r"Perceptron=(\d+)\s+conf=(\d+),\s+FNN=(\d+)\s+conf=(\d+),\s+CNN=(\d+)\s+conf=(\d+)",
            line,
        )
        if legacy:
            self._update_mcu_result("P", legacy.group(1), legacy.group(2), "")
            self._update_mcu_result("F", legacy.group(3), legacy.group(4), "")
            self._update_mcu_result("C", legacy.group(5), legacy.group(6), "")

    def _update_mcu_result(self, model: str, label: str, confidence: str, time_us: str) -> None:
        variables = self.mcu_result_vars.get(model)
        if variables is None:
            return
        try:
            conf_value = max(min(float(confidence), 100.0), 0.0)
        except ValueError:
            conf_value = 0.0
        suffix = f"  {int(conf_value)}%"
        if time_us:
            suffix += f"  {time_us}us"
        variables["label"].set(f"{label}{suffix}")  # type: ignore[union-attr]
        variables["conf"].set(conf_value)  # type: ignore[union-attr]

    def _append_serial_log(self, line: str) -> None:
        if self.serial_log is None:
            return
        self.serial_log.insert("end", line + "\n")
        self.serial_log.see("end")

    def _update_usage(self) -> None:
        usage = parse_build_usage()
        if usage is None:
            self.usage_vars["flash_text"].set("Build in Keil first")  # type: ignore[union-attr]
            self.usage_vars["sram_text"].set("Build in Keil first")  # type: ignore[union-attr]
            self.usage_vars["flash_value"].set(0.0)  # type: ignore[union-attr]
            self.usage_vars["sram_value"].set(0.0)  # type: ignore[union-attr]
            return

        flash_percent = usage["flash"] * 100.0 / FLASH_BYTES
        sram_percent = usage["sram"] * 100.0 / SRAM_BYTES
        self.usage_vars["flash_text"].set(f"{usage['flash'] / 1024:.1f} KB / 512 KB ({flash_percent:.1f}%)")  # type: ignore[union-attr]
        self.usage_vars["sram_text"].set(f"{usage['sram'] / 1024:.1f} KB / 64 KB ({sram_percent:.1f}%)")  # type: ignore[union-attr]
        self.usage_vars["flash_value"].set(flash_percent)  # type: ignore[union-attr]
        self.usage_vars["sram_value"].set(sram_percent)  # type: ignore[union-attr]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default=None, help="Optional serial port, for example COM3.")
    parser.add_argument("--baud", type=int, default=115200)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    root = tk.Tk()
    RealtimeDigitUI(root, args.port, args.baud)
    root.mainloop()


if __name__ == "__main__":
    main()
