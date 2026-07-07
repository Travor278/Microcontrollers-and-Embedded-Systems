"""Local web dashboard for DigitNN drawing, inference, capture, and flashing."""

from __future__ import annotations

import argparse
import base64
import csv
import json
import mimetypes
import os
import re
import shutil
import subprocess
import sys
import threading
import importlib.util
from datetime import datetime
from collections import deque
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from io import BytesIO
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlparse
from urllib import error as urlerror
from urllib import request as urlrequest

import numpy as np
from PIL import Image, ImageDraw

try:
    import serial
    from serial.tools import list_ports
except ImportError:  # pragma: no cover - handled in API responses.
    serial = None
    list_ports = None


ROOT_DIR = Path(__file__).resolve().parents[1]
HOST_APP_DIR = ROOT_DIR / "host_app"
WEB_DIR = HOST_APP_DIR / "web"
TOOLS_DIR = ROOT_DIR / "tools"
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))
if str(HOST_APP_DIR) not in sys.path:
    sys.path.insert(0, str(HOST_APP_DIR))

from host_batch_test import default_quant_file, predict_scores, run_batch  # noqa: E402
from realtime_digit_ui import (  # noqa: E402
    CLASS_LABELS,
    FLASH_BYTES,
    MODEL_SPECS,
    SRAM_BYTES,
    confidence_from_scores,
    parse_build_usage,
    preprocess_canvas_image,
)
from keil_flash import find_uv4  # noqa: E402


MODEL_CACHE: dict[str, dict[str, np.ndarray]] = {}
LETTER_MODEL_CHOICES = {
    "letter_perceptron": "Letter-Perceptron",
    "letter_fnn": "Letter-FNN",
    "letter_ds_cnn": "Letter-DS-CNN",
}
LETTER_MODEL_SHORT = {
    "letter_perceptron": "P",
    "letter_fnn": "F",
    "letter_ds_cnn": "C",
}
GENERATED_DOMAIN_HEADER = ROOT_DIR / "keil_touch_digit_nn" / "User" / "digit_nn" / "generated" / "RecognitionDomain.h"
BUILD_LOG = ROOT_DIR / "keil_touch_digit_nn" / "Output" / "DigitNN_Touch.build_log.htm"
USAGE_CACHE_FILE = ROOT_DIR / "keil_touch_digit_nn" / "Output" / "firmware_usage.json"
LOCAL_ENV_FILE = ROOT_DIR / ".env.local"
DEFAULT_CSU_BASE_URL = "https://api.chat.csu.edu.cn/v1"
DEFAULT_CSU_TOKEN_NAME = "fa_8202240417"
DEFAULT_CSU_MODEL = "qwen-vl-plus"
DEFAULT_ALIYUN_BASE_URL = "https://dashscope.aliyuncs.com/compatible-mode/v1"
DEFAULT_ALIYUN_MODEL = "qwen-vl-max"
DEFAULT_CSU_MODEL_CANDIDATES = [
    "qwen3.6-flash",
    "qwen3.6-plus",
    "qwen3.6-max-preview",
    "qwen3.6-27b",
    "qwen3.6-35b-a3b",
    "qwen-vl-plus",
    "qwen-vl-max",
    "qwen2.5-vl-72b-instruct",
    "qwen2.5-vl-32b-instruct",
    "Qwen3-32B-FP8",
    "Qwen3-32B",
    "deepseek-v3",
    "QwQ-32B",
    "DeepSeek-Coder-V2-Lite-Instruct",
]
DEFAULT_ALIYUN_MODEL_CANDIDATES = [
    "qwen-vl-plus",
    "qwen-vl-max",
    "qwen2.5-vl-72b-instruct",
    "qwen2.5-vl-32b-instruct",
    "qwen2.5-vl-7b-instruct",
    "qwen3-vl-plus",
    "qwen3-vl-max",
    "qwen3.6-flash",
    "qwen3.6-plus",
    "qwen-plus",
]


class SerialBridge:
    """Small server-side serial reader for browsers without explicit COM selection."""

    def __init__(self) -> None:
        self._lock = threading.RLock()
        self._lines: deque[dict[str, object]] = deque(maxlen=400)
        self._next_id = 1
        self._port = None
        self._thread: threading.Thread | None = None
        self._stop_event = threading.Event()
        self._port_name = ""
        self._baudrate = 115200
        self._error = ""

    def list_ports(self) -> dict[str, object]:
        if list_ports is None:
            return {"ok": False, "error": "pyserial is not installed", "ports": []}

        ports = []
        for port in list_ports.comports():
            ports.append(
                {
                    "device": port.device,
                    "description": port.description,
                    "hwid": port.hwid,
                    "label": f"{port.device} - {port.description}",
                    "detected": True,
                }
            )
        return {"ok": True, "ports": ports, "connected": self.status()}

    def status(self) -> dict[str, object]:
        with self._lock:
            return {
                "open": self._port is not None,
                "port": self._port_name,
                "baudrate": self._baudrate,
                "error": self._error,
                "lastId": self._next_id - 1,
            }

    def connect(self, port_name: str, baudrate: int = 115200) -> dict[str, object]:
        if serial is None:
            raise RuntimeError("pyserial is not installed")
        port_name = port_name.strip()
        if not port_name:
            raise ValueError("serial port is required")

        self.disconnect()
        self._stop_event.clear()
        try:
            port = serial.Serial(port_name, baudrate=baudrate, timeout=0.1, rtscts=False, dsrdtr=False)
            try:
                port.dtr = False
                port.rts = False
            except Exception:
                pass
        except Exception as exc:
            with self._lock:
                self._error = str(exc)
            raise

        with self._lock:
            self._port = port
            self._port_name = port_name
            self._baudrate = baudrate
            self._error = ""
            self._lines.clear()
            self._next_id = 1

        self._thread = threading.Thread(target=self._read_loop, name="DigitNNSerialBridge", daemon=True)
        self._thread.start()
        return {"ok": True, "status": self.status()}

    def disconnect(self) -> dict[str, object]:
        self._stop_event.set()
        with self._lock:
            port = self._port
            self._port = None
        if port is not None:
            try:
                port.close()
            except Exception:
                pass
        thread = self._thread
        if thread is not None and thread.is_alive():
            thread.join(timeout=0.5)
        self._thread = None
        return {"ok": True, "status": self.status()}

    def read_since(self, since: int) -> dict[str, object]:
        with self._lock:
            lines = [item for item in self._lines if int(item["id"]) > since]
            status = self.status()
        return {"ok": True, "lines": lines, "status": status}

    def _append_line(self, text: str) -> None:
        if not text:
            return
        with self._lock:
            self._lines.append({"id": self._next_id, "line": text})
            self._next_id += 1

    def _read_loop(self) -> None:
        buffer = b""
        while not self._stop_event.is_set():
            with self._lock:
                port = self._port
            if port is None:
                break
            try:
                chunk = port.read(256)
                if not chunk:
                    continue
                buffer += chunk
                while b"\n" in buffer:
                    raw, buffer = buffer.split(b"\n", 1)
                    line = raw.decode("utf-8", errors="replace").strip()
                    self._append_line(line)
            except Exception as exc:
                with self._lock:
                    self._error = str(exc)
                    self._port = None
                try:
                    port.close()
                except Exception:
                    pass
                self._append_line(f"STATUS,state=serial,message={exc}")
                break


SERIAL_BRIDGE = SerialBridge()


def load_models() -> dict[str, dict[str, np.ndarray]]:
    global MODEL_CACHE
    if MODEL_CACHE:
        return MODEL_CACHE

    for model, _short_name, _display_name in MODEL_SPECS:
        path = default_quant_file(model)
        if not path.exists():
            continue
        with np.load(path) as data:
            MODEL_CACHE[model] = {key: np.array(data[key]) for key in data.files}
    return MODEL_CACHE


def reload_models() -> dict[str, dict[str, np.ndarray]]:
    MODEL_CACHE.clear()
    return load_models()


def json_response(handler: BaseHTTPRequestHandler, payload: object, status: HTTPStatus = HTTPStatus.OK) -> None:
    body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def error_response(handler: BaseHTTPRequestHandler, message: str, status: HTTPStatus = HTTPStatus.BAD_REQUEST) -> None:
    json_response(handler, {"ok": False, "error": message}, status)


def read_json(handler: BaseHTTPRequestHandler) -> dict[str, object]:
    length = int(handler.headers.get("Content-Length", "0"))
    if length <= 0:
        return {}
    raw = handler.rfile.read(length).decode("utf-8")
    return json.loads(raw)


def image_from_data_url(data_url: str) -> Image.Image:
    if "," in data_url:
        _header, encoded = data_url.split(",", maxsplit=1)
    else:
        encoded = data_url
    raw = base64.b64decode(encoded)
    image = Image.open(BytesIO(raw)).convert("RGBA")

    alpha = image.getchannel("A")
    background = Image.new("RGBA", image.size, (0, 0, 0, 255))
    background.alpha_composite(image)
    gray = background.convert("L")
    arr = np.asarray(gray, dtype=np.uint8)
    arr = np.where(np.asarray(alpha, dtype=np.uint8) == 0, 0, arr).astype(np.uint8)
    return Image.fromarray(arr, mode="L")


def infer_image(raw: Image.Image, thicken: bool, deskew: bool) -> dict[str, object]:
    digit = preprocess_canvas_image(raw, thicken=thicken, deskew=deskew)
    pixels = np.asarray(digit, dtype=np.uint8).reshape(-1)
    return infer_pixels_array(pixels)


def infer_pixels_array(pixels: np.ndarray) -> dict[str, object]:
    results: list[dict[str, object]] = []

    for model, short_name, display_name in MODEL_SPECS:
        data = load_models().get(model)
        if data is None:
            results.append({"model": model, "short": short_name, "name": display_name, "available": False})
            continue
        scores = predict_scores(model, pixels, data)  # type: ignore[arg-type]
        label, confidence, second = confidence_from_scores(scores)
        results.append(
            {
                "model": model,
                "short": short_name,
                "name": display_name,
                "available": True,
                "label": str(label),
                "confidence": confidence,
                "second": str(second),
                "scores": [int(value) for value in scores.tolist()],
            }
        )

    return {
        "ok": True,
        "pixels": [int(value) for value in pixels.tolist()],
        "results": results,
    }


def infer_pixels_payload(payload: dict[str, object]) -> dict[str, object]:
    width = int(payload.get("width", 28))
    height = int(payload.get("height", 28))
    raw_pixels = payload.get("pixels", [])
    if width != 28 or height != 28:
        raise ValueError("pixel inference expects 28x28 input")
    if not isinstance(raw_pixels, list) or len(raw_pixels) != width * height:
        raise ValueError("pixels length does not match 28x28 input")

    pixels = np.array([int(value) for value in raw_pixels], dtype=np.int16)
    pixels = np.clip(pixels, 0, 255).astype(np.uint8).reshape(-1)
    return infer_pixels_array(pixels)


def save_sample(raw: Image.Image, label: str, thicken: bool, deskew: bool, output_dir: Path) -> dict[str, object]:
    if label not in set(CLASS_LABELS):
        raise ValueError("label must be one of 0-9A-Z")

    digit = preprocess_canvas_image(raw, thicken=thicken, deskew=deskew)
    if int(np.asarray(digit, dtype=np.uint8).max()) == 0:
        raise ValueError("empty drawing")

    output_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bmp_name = f"web_{stamp}_{label}.bmp"
    raw_name = f"web_{stamp}_{label}_raw.png"
    digit.save(output_dir / bmp_name)
    raw.save(output_dir / raw_name)

    with (output_dir / "label.txt").open("a", encoding="utf-8", newline="") as handle:
        handle.write(f"{bmp_name},{label}\n")

    log_path = output_dir / "capture_log.csv"
    is_new = not log_path.exists()
    infer_result = infer_image(raw, thicken=thicken, deskew=deskew)
    compact = {
        result["short"]: f"{result.get('label', '')}:{result.get('confidence', '')}"
        for result in infer_result["results"]  # type: ignore[index]
        if result.get("available")
    }
    with log_path.open("a", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=["time", "filename", "raw_filename", "label", "pc_p", "pc_f", "pc_c"])
        if is_new:
            writer.writeheader()
        writer.writerow(
            {
                "time": datetime.now().isoformat(timespec="seconds"),
                "filename": bmp_name,
                "raw_filename": raw_name,
                "label": label,
                "pc_p": compact.get("P", ""),
                "pc_f": compact.get("F", ""),
                "pc_c": compact.get("C", ""),
            }
        )

    return {
        "ok": True,
        "filename": bmp_name,
        "rawFilename": raw_name,
        "labelFile": str(output_dir / "label.txt"),
    }


def save_letter_sample(raw: Image.Image, label: str, output_dir: Path) -> dict[str, object]:
    if label not in set("ABCDEFGHIJKLMNOPQRSTUVWXYZ"):
        raise ValueError("letter label must be one of A-Z")

    letter = preprocess_canvas_image(raw, thicken=False, deskew=True)
    if int(np.asarray(letter, dtype=np.uint8).max()) == 0:
        raise ValueError("empty drawing")

    output_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bmp_name = f"letter_{stamp}_{label}.bmp"
    raw_name = f"letter_{stamp}_{label}_raw.png"
    letter.save(output_dir / bmp_name)
    raw.save(output_dir / raw_name)

    with (output_dir / "label.txt").open("a", encoding="utf-8", newline="") as handle:
        handle.write(f"{bmp_name},{label}\n")

    log_path = output_dir / "capture_log.csv"
    is_new = not log_path.exists()
    with log_path.open("a", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=["time", "filename", "raw_filename", "label", "domain"])
        if is_new:
            writer.writeheader()
        writer.writerow(
            {
                "time": datetime.now().isoformat(timespec="seconds"),
                "filename": bmp_name,
                "raw_filename": raw_name,
                "label": label,
                "domain": "letters",
            }
        )

    return {
        "ok": True,
        "filename": bmp_name,
        "rawFilename": raw_name,
        "labelFile": str(output_dir / "label.txt"),
    }


def save_board_sample(
    pixels: object,
    width: int,
    height: int,
    label: str,
    output_dir: Path,
    results: object | None = None,
) -> dict[str, object]:
    if label not in set(CLASS_LABELS):
        raise ValueError("label must be one of 0-9A-Z")
    if width <= 0 or height <= 0 or width * height > 4096:
        raise ValueError("invalid image dimensions")
    if not isinstance(pixels, list) or len(pixels) != width * height:
        raise ValueError("pixels length does not match image dimensions")

    arr = np.array([int(value) for value in pixels], dtype=np.int16)
    if int(arr.max(initial=0)) <= 0:
        raise ValueError("empty board image")
    arr = np.clip(arr, 0, 255).astype(np.uint8).reshape((height, width))

    output_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    bmp_name = f"board_{stamp}_{label}.bmp"
    preview_name = f"board_{stamp}_{label}_preview.png"
    image = Image.fromarray(arr, mode="L")
    image.save(output_dir / bmp_name)
    image.resize((width * 10, height * 10), Image.Resampling.NEAREST).save(output_dir / preview_name)

    with (output_dir / "label.txt").open("a", encoding="utf-8", newline="") as handle:
        handle.write(f"{bmp_name},{label}\n")

    compact_results = ""
    if isinstance(results, dict):
        compact_results = json.dumps(results, ensure_ascii=False, separators=(",", ":"))

    log_path = output_dir / "capture_log.csv"
    is_new = not log_path.exists()
    with log_path.open("a", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "time",
                "source",
                "filename",
                "preview_filename",
                "label",
                "width",
                "height",
                "active_pixels",
                "max_pixel",
                "mcu_results",
            ],
        )
        if is_new:
            writer.writeheader()
        writer.writerow(
            {
                "time": datetime.now().isoformat(timespec="seconds"),
                "source": "stm32",
                "filename": bmp_name,
                "preview_filename": preview_name,
                "label": label,
                "width": width,
                "height": height,
                "active_pixels": int(np.count_nonzero(arr)),
                "max_pixel": int(arr.max()),
                "mcu_results": compact_results,
            }
        )

    return {
        "ok": True,
        "filename": bmp_name,
        "previewFilename": preview_name,
        "labelFile": str(output_dir / "label.txt"),
    }


def read_usage_cache() -> dict[str, object]:
    if not USAGE_CACHE_FILE.exists():
        return {"domains": {}}
    try:
        payload = json.loads(USAGE_CACHE_FILE.read_text(encoding="utf-8", errors="replace"))
    except (OSError, json.JSONDecodeError):
        return {"domains": {}}
    if not isinstance(payload, dict):
        return {"domains": {}}
    if not isinstance(payload.get("domains"), dict):
        payload["domains"] = {}
    return payload


def usage_entry_payload(raw: dict[str, object] | None, domain: str, source: str) -> dict[str, object]:
    if not raw:
        return {"available": False, "domain": domain, "source": source}
    try:
        usage = {
            "code": int(raw["code"]),
            "ro_data": int(raw["ro_data"]),
            "rw_data": int(raw["rw_data"]),
            "zi_data": int(raw["zi_data"]),
            "flash": int(raw["flash"]),
            "sram": int(raw["sram"]),
        }
    except (KeyError, TypeError, ValueError):
        return {"available": False, "domain": domain, "source": source}
    payload: dict[str, object] = {
        "available": True,
        "domain": domain,
        "source": source,
        "flash": usage["flash"],
        "sram": usage["sram"],
        "flashPercent": usage["flash"] * 100.0 / FLASH_BYTES,
        "sramPercent": usage["sram"] * 100.0 / SRAM_BYTES,
        "detail": usage,
    }
    if "updatedAt" in raw:
        payload["updatedAt"] = str(raw["updatedAt"])
    if "mtime" in raw:
        try:
            payload["mtime"] = float(raw["mtime"])
        except (TypeError, ValueError):
            pass
    return payload


def usage_domains_payload() -> dict[str, dict[str, object]]:
    cache = read_usage_cache()
    cached_domains = cache.get("domains", {})
    domains = {
        "digit": usage_entry_payload(
            cached_domains.get("digit") if isinstance(cached_domains, dict) else None,
            "digit",
            "cache",
        ),
        "letter": usage_entry_payload(
            cached_domains.get("letter") if isinstance(cached_domains, dict) else None,
            "letter",
            "cache",
        ),
    }

    active_domain = active_firmware_domain()
    build_usage = parse_build_usage()
    if build_usage is not None and BUILD_LOG.exists():
        build_mtime = BUILD_LOG.stat().st_mtime
        generated_mtime = GENERATED_DOMAIN_HEADER.stat().st_mtime if GENERATED_DOMAIN_HEADER.exists() else 0.0
        cached_entry = domains.get(active_domain, {})
        cached_mtime = 0.0
        if isinstance(cached_entry, dict):
            try:
                cached_mtime = float(cached_entry.get("mtime", 0.0))
            except (TypeError, ValueError):
                cached_mtime = 0.0
        if build_mtime >= generated_mtime - 1.0 and build_mtime >= cached_mtime:
            domains[active_domain] = usage_entry_payload(
                {**build_usage, "mtime": build_mtime},
                active_domain,
                "build_log",
            )
    return domains


def usage_payload(domain: str | None = None) -> dict[str, object]:
    active_domain = active_firmware_domain()
    selected_domain = domain if domain in {"digit", "letter"} else active_domain
    domains = usage_domains_payload()
    selected = domains.get(selected_domain, {"available": False, "domain": selected_domain})
    payload = {
        "ok": True,
        "activeDomain": active_domain,
        "domain": selected_domain,
        "domains": domains,
    }
    if not selected.get("available"):
        return {**payload, "available": False}
    return {**payload, **selected, "ok": True, "activeDomain": active_domain, "domains": domains}


def drive_roots() -> list[Path]:
    if os.name == "nt":
        roots: list[Path] = []
        for code in range(ord("C"), ord("Z") + 1):
            root = Path(f"{chr(code)}:/")
            try:
                if root.exists():
                    roots.append(root)
            except OSError:
                continue
        return roots
    return [Path("/")]


def path_usage(path: Path) -> dict[str, object]:
    try:
        usage = shutil.disk_usage(path)
    except OSError:
        return {"available": False}
    return {
        "available": True,
        "total": usage.total,
        "used": usage.used,
        "free": usage.free,
        "usedPercent": (usage.used * 100.0 / usage.total) if usage.total else 0.0,
    }


def scan_tf_card_path(path: Path, max_preview: int = 8) -> dict[str, object]:
    path = path.expanduser()
    try:
        resolved = path.resolve()
    except OSError:
        resolved = path
    if not path.exists() or not path.is_dir():
        return {
            "available": False,
            "path": str(path),
            "datasets": [],
            "totalImages": 0,
            "labelFiles": 0,
            "preview": [],
        }

    datasets: list[dict[str, object]] = []
    preview: list[dict[str, object]] = []
    total_images = 0
    label_files = 0
    image_exts = {".bmp", ".png", ".jpg", ".jpeg"}
    for child in sorted(path.iterdir(), key=lambda item: item.name.lower()):
        if not child.is_dir():
            if child.name.lower() == "label.txt":
                label_files += 1
            continue
        image_count = 0
        child_label_files = 0
        child_bytes = 0
        child_preview: list[str] = []
        try:
            files = list(child.iterdir())
        except OSError:
            files = []
        for file_path in files:
            if not file_path.is_file():
                continue
            suffix = file_path.suffix.lower()
            if suffix in image_exts:
                image_count += 1
                try:
                    child_bytes += file_path.stat().st_size
                except OSError:
                    pass
                if len(child_preview) < 4:
                    child_preview.append(file_path.name)
                if len(preview) < max_preview:
                    preview.append({"dataset": child.name, "name": file_path.name, "path": str(file_path)})
            elif file_path.name.lower() == "label.txt":
                child_label_files += 1

        if image_count or child_label_files:
            datasets.append(
                {
                    "name": child.name,
                    "path": str(child),
                    "images": image_count,
                    "labelFiles": child_label_files,
                    "bytes": child_bytes,
                    "preview": child_preview,
                }
            )
        total_images += image_count
        label_files += child_label_files

    root = Path(str(resolved.anchor)) if resolved.anchor else resolved
    return {
        "available": True,
        "path": str(resolved),
        "root": str(root),
        "disk": path_usage(root),
        "datasets": datasets,
        "totalImages": total_images,
        "labelFiles": label_files,
        "preview": preview,
    }


def keil_sd_capability_payload() -> dict[str, object]:
    user_dir = ROOT_DIR / "keil_touch_digit_nn" / "User"
    expected = {
        "ff.c": list(user_dir.rglob("ff.c")) if user_dir.exists() else [],
        "diskio.c": list(user_dir.rglob("diskio.c")) if user_dir.exists() else [],
        "bsp_sdio_sdcard.c": list(user_dir.rglob("bsp_sdio_sdcard.c")) if user_dir.exists() else [],
    }
    firmware_stub = ROOT_DIR / "firmware" / "src" / "drivers" / "sd_testset.c"
    stub_text = firmware_stub.read_text(encoding="utf-8", errors="replace") if firmware_stub.exists() else ""
    return {
        "fatfsReady": all(bool(paths) for paths in expected.values()),
        "files": {name: [str(path) for path in paths[:3]] for name, paths in expected.items()},
        "sdTestsetStub": "STATUS_ERROR_NOT_READY" in stub_text,
        "referenceExamples": [
            "1-书籍配套例程-F103VE指南者_20240202/36-SDIO-SD卡读写测试",
            "1-书籍配套例程-F103VE指南者_20240202/37-SDIO-FatFs移植与读写测试",
        ],
    }


def sd_card_status_payload(path_hint: str | None = None) -> dict[str, object]:
    requested_paths: list[Path] = []
    if path_hint:
        requested_paths.append(Path(path_hint))
    requested_paths.extend(root / "tf_card" for root in drive_roots())
    requested_paths.append(ROOT_DIR / "tf_card")

    seen: set[str] = set()
    candidates: list[dict[str, object]] = []
    for path in requested_paths:
        key = str(path).lower()
        if key in seen:
            continue
        seen.add(key)
        scan = scan_tf_card_path(path)
        try:
            is_workspace = path.resolve() == (ROOT_DIR / "tf_card").resolve()
        except OSError:
            is_workspace = False
        scan["kind"] = "workspace" if is_workspace else "mounted"
        candidates.append(scan)

    serial_lines = SERIAL_BRIDGE.read_since(0).get("lines", [])
    tf_lines: list[dict[str, object]] = []
    for item in serial_lines:
        line = str(item.get("line", ""))
        if re.search(r"^(SD|TF|FATFS|CARD|DIR|FILE)[,_]", line, flags=re.IGNORECASE) or re.search(
            r"\b(sd|tf|fatfs)\b", line, flags=re.IGNORECASE
        ):
            tf_lines.append(item)

    capability = keil_sd_capability_payload()
    mounted_available = any(item.get("available") and item.get("kind") == "mounted" for item in candidates)
    workspace = next((item for item in candidates if item.get("kind") == "workspace"), None)
    return {
        "ok": True,
        "checkedAt": datetime.now().isoformat(timespec="seconds"),
        "pc": {
            "drives": [str(root) for root in drive_roots()],
            "mountedTfCardAvailable": mounted_available,
            "candidates": candidates,
            "workspaceTfCard": workspace,
        },
        "mcu": {
            "serial": SERIAL_BRIDGE.status(),
            "tfFramesSeen": tf_lines[-20:],
            "sdFramesSeen": tf_lines[-20:],
            "tfFrameCount": len(tf_lines),
            "sdFrameCount": len(tf_lines),
            "protocolSupportedNow": bool(tf_lines),
            "firmwareFatfsReady": bool(capability.get("fatfsReady")),
            "canConfirmRead": bool(tf_lines),
            "capability": capability,
            "message": (
                "MCU has reported TF/FatFs frames."
                if tf_lines
                else "No TF/FatFs frames have been seen on the current serial protocol; current firmware cannot confirm MCU-side TF-card reading."
            ),
        },
    }

def active_firmware_domain() -> str:
    if not GENERATED_DOMAIN_HEADER.exists():
        return "digit"
    text = GENERATED_DOMAIN_HEADER.read_text(encoding="utf-8", errors="replace")
    match = re.search(r"#define\s+RECOGNITION_DOMAIN\s+RECOGNITION_DOMAIN_(DIGIT|LETTER)", text)
    return match.group(1).lower() if match else "digit"


def quant_file_for_model(domain: str, model: str) -> Path:
    if domain == "letter":
        return ROOT_DIR / "models" / f"{model}_quant.npz"
    return default_quant_file(model)


def quantization_models_for_domain(domain: str) -> list[tuple[str, str, str]]:
    if domain == "letter":
        return [
            (model, LETTER_MODEL_SHORT.get(model, "?"), name)
            for model, name in LETTER_MODEL_CHOICES.items()
        ]
    return list(MODEL_SPECS)


def quantization_profile_payload() -> dict[str, object]:
    domain = active_firmware_domain()
    models: list[dict[str, object]] = []
    total_quant_bytes = 0
    total_float_bytes = 0
    total_param_count = 0

    for model, short_name, display_name in quantization_models_for_domain(domain):
        quant_file = quant_file_for_model(domain, model)
        entry: dict[str, object] = {
            "model": model,
            "short": short_name,
            "name": display_name,
            "available": quant_file.exists(),
            "file": str(quant_file),
        }
        if not quant_file.exists():
            models.append(entry)
            continue

        quant_param_bytes = 0
        float_param_bytes = 0
        metadata_bytes = 0
        parameter_count = 0
        arrays: list[dict[str, object]] = []
        with np.load(quant_file) as data:
            for key in data.files:
                array = data[key]
                byte_count = int(array.nbytes)
                element_count = int(array.size)
                kind = "metadata"
                is_parameter = False
                if "weight" in key:
                    kind = "weight"
                    is_parameter = True
                elif "bias" in key:
                    kind = "bias"
                    is_parameter = True

                if is_parameter:
                    quant_param_bytes += byte_count
                    float_param_bytes += element_count * 4
                    parameter_count += element_count
                else:
                    metadata_bytes += byte_count

                arrays.append(
                    {
                        "name": key,
                        "kind": kind,
                        "shape": list(array.shape),
                        "dtype": str(array.dtype),
                        "bytes": byte_count,
                        "elements": element_count,
                    }
                )

        quant_total_bytes = quant_param_bytes + metadata_bytes
        saved_bytes = max(float_param_bytes - quant_total_bytes, 0)
        total_quant_bytes += quant_total_bytes
        total_float_bytes += float_param_bytes
        total_param_count += parameter_count
        entry.update(
            {
                "quantBytes": quant_total_bytes,
                "quantParamBytes": quant_param_bytes,
                "metadataBytes": metadata_bytes,
                "floatBytes": float_param_bytes,
                "savedBytes": saved_bytes,
                "compression": (float_param_bytes / quant_total_bytes) if quant_total_bytes else 0.0,
                "parameterCount": parameter_count,
                "arrays": arrays,
            }
        )
        models.append(entry)

    usage = usage_payload()
    quantized_flash = usage.get("flash") if usage.get("available") else None
    quantized_sram = usage.get("sram") if usage.get("available") else None
    saved_bytes = max(total_float_bytes - total_quant_bytes, 0)
    estimated_float_flash = None
    if isinstance(quantized_flash, int):
        estimated_float_flash = quantized_flash + saved_bytes

    return {
        "ok": True,
        "domain": domain,
        "domainName": "Letter" if domain == "letter" else "Digit",
        "available": any(bool(model.get("available")) for model in models),
        "models": models,
        "totals": {
            "quantBytes": total_quant_bytes,
            "floatBytes": total_float_bytes,
            "savedBytes": saved_bytes,
            "compression": (total_float_bytes / total_quant_bytes) if total_quant_bytes else 0.0,
            "parameterCount": total_param_count,
        },
        "firmware": {
            "quantizedFlash": quantized_flash,
            "quantizedSram": quantized_sram,
            "estimatedFloatFlash": estimated_float_flash,
            "estimated": True,
        },
        "limits": {
            "flash": FLASH_BYTES,
            "sram": SRAM_BYTES,
        },
        "note": "Current firmware uses int8 weights and int32 accumulators; float firmware size is estimated from actual model arrays.",
    }


def read_local_env() -> dict[str, str]:
    values: dict[str, str] = {}
    if LOCAL_ENV_FILE.exists():
        for raw_line in LOCAL_ENV_FILE.read_text(encoding="utf-8", errors="replace").splitlines():
            line = raw_line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, value = line.split("=", maxsplit=1)
            values[key.strip()] = value.strip().strip('"').strip("'")
    for key in (
        "CHINESE_API_PROVIDER",
        "CSU_API_KEY",
        "CSU_API_TOKEN_NAME",
        "CSU_API_BASE_URL",
        "CSU_API_MODEL",
        "ALIYUN_API_KEY",
        "DASHSCOPE_API_KEY",
        "ALIYUN_API_BASE_URL",
        "ALIYUN_API_MODEL",
    ):
        if os.environ.get(key):
            values[key] = os.environ[key]
    return values


def csu_api_config() -> dict[str, object]:
    return chinese_api_config("csu")


def chinese_api_config(provider: str | None = None) -> dict[str, object]:
    values = read_local_env()
    active_provider = (provider or values.get("CHINESE_API_PROVIDER") or "csu").strip().lower()
    if active_provider in {"aliyun", "dashscope", "alibaba"}:
        active_provider = "aliyun"
        api_key = (values.get("ALIYUN_API_KEY") or values.get("DASHSCOPE_API_KEY") or "").strip()
        base_url = (values.get("ALIYUN_API_BASE_URL") or DEFAULT_ALIYUN_BASE_URL).strip().rstrip("/")
        model = (values.get("ALIYUN_API_MODEL") or DEFAULT_ALIYUN_MODEL).strip()
        token_name = "DASHSCOPE_API_KEY"
        candidates = DEFAULT_ALIYUN_MODEL_CANDIDATES
    else:
        active_provider = "csu"
        api_key = values.get("CSU_API_KEY", "").strip()
        base_url = (values.get("CSU_API_BASE_URL") or DEFAULT_CSU_BASE_URL).strip().rstrip("/")
        model = (values.get("CSU_API_MODEL") or DEFAULT_CSU_MODEL).strip()
        token_name = (values.get("CSU_API_TOKEN_NAME") or DEFAULT_CSU_TOKEN_NAME).strip()
        candidates = DEFAULT_CSU_MODEL_CANDIDATES

    return {
        "provider": active_provider,
        "configured": bool(api_key),
        "apiKey": api_key,
        "baseUrl": base_url,
        "model": model,
        "tokenName": token_name,
        "candidates": candidates,
    }


def local_ocr_runtime_payload() -> list[dict[str, object]]:
    packages = [
        ("opencv", "cv2", "image preprocessing / contour features"),
        ("torch", "torch", "local classifier training/inference"),
        ("torchvision", "torchvision", "EMNIST/CASIA data tooling when installed"),
        ("onnxruntime", "onnxruntime", "exported ONNX recognizer inference"),
        ("paddleocr", "paddleocr", "ready-made Chinese OCR pipeline"),
        ("easyocr", "easyocr", "ready-made OCR baseline"),
        ("pytesseract", "pytesseract", "traditional OCR baseline"),
        ("transformers", "transformers", "TrOCR/SVTR-style model loading"),
    ]
    return [
        {
            "name": display,
            "module": module,
            "installed": importlib.util.find_spec(module) is not None,
            "role": role,
        }
        for display, module, role in packages
    ]


def chinese_status_payload(provider: str | None = None) -> dict[str, object]:
    config = chinese_api_config(provider)
    runtimes = local_ocr_runtime_payload()
    return {
        "ok": True,
        "mode": "pc",
        "endpoint": "/api/chinese/infer",
        "modelsEndpoint": "/api/chinese/models",
        "boardRole": "touchpad only: send stroke bitmap or trajectory to PC",
        "remote": {
            "provider": config["provider"],
            "configured": bool(config["configured"]),
            "tokenName": config["tokenName"],
            "baseUrl": config["baseUrl"],
            "model": config["model"],
            "candidateCount": len(config.get("candidates", [])),
        },
        "providers": {
            "csu": {
                "baseUrl": DEFAULT_CSU_BASE_URL,
                "tokenName": DEFAULT_CSU_TOKEN_NAME,
            },
            "aliyun": {
                "baseUrl": DEFAULT_ALIYUN_BASE_URL,
                "tokenName": "DASHSCOPE_API_KEY",
            },
        },
        "localRuntimes": runtimes,
        "localAvailable": any(bool(item["installed"]) for item in runtimes if item["name"] in {"opencv", "torch", "onnxruntime"}),
        "recommended": [
            {
                "name": "MobileNetV3 / EfficientNet classifier",
                "task": "single isolated 5000-class character",
                "note": "Best first implementation when the board writes one character at a time.",
            },
            {
                "name": "PP-OCRv5 recognizer",
                "task": "line text / mixed Chinese-English OCR",
                "note": "Use when moving from isolated characters to full text recognition.",
            },
            {
                "name": "CRNN/SVTR-style recognizer",
                "task": "sequence recognition",
                "note": "Use CTC/attention decoding when input may contain multiple characters.",
            },
        ],
    }


def normalize_image_data_url(data_url: str) -> str:
    if data_url.startswith("data:image/"):
        return data_url
    return f"data:image/png;base64,{data_url}"


def extract_json_object(text: str) -> dict[str, object] | None:
    content = text.strip()
    if content.startswith("```"):
        lines = [line for line in content.splitlines() if not line.strip().startswith("```")]
        content = "\n".join(lines).strip()
    try:
        loaded = json.loads(content)
        return loaded if isinstance(loaded, dict) else None
    except json.JSONDecodeError:
        pass
    start = content.find("{")
    end = content.rfind("}")
    if start >= 0 and end > start:
        try:
            loaded = json.loads(content[start : end + 1])
            return loaded if isinstance(loaded, dict) else None
        except json.JSONDecodeError:
            return None
    return None


def normalize_chinese_candidates(parsed: dict[str, object] | None, raw_text: str) -> tuple[str, list[dict[str, object]]]:
    if not parsed:
        text = raw_text.strip()
        return text, [{"text": text, "confidence": None, "note": "raw model output"}] if text else []

    result_text = str(parsed.get("text") or parsed.get("result") or parsed.get("label") or "").strip()
    raw_candidates = parsed.get("candidates", [])
    candidates: list[dict[str, object]] = []
    if isinstance(raw_candidates, list):
        for item in raw_candidates:
            if isinstance(item, dict):
                candidate_text = str(item.get("text") or item.get("label") or item.get("char") or "").strip()
                confidence = item.get("confidence", item.get("score"))
            else:
                candidate_text = str(item).strip()
                confidence = None
            if candidate_text:
                candidates.append({"text": candidate_text, "confidence": confidence})
    if result_text and not any(item["text"] == result_text for item in candidates):
        candidates.insert(0, {"text": result_text, "confidence": parsed.get("confidence", parsed.get("score"))})
    return result_text, candidates


def post_csu_chat_completion(
    request_body: dict[str, object],
    timeout: int = 60,
    provider: str | None = None,
) -> dict[str, object]:
    config = chinese_api_config(provider)
    if not config["configured"]:
        if config["provider"] == "aliyun":
            raise RuntimeError("ALIYUN_API_KEY is not configured. Run tools/save_aliyun_api_key.ps1 first.")
        raise RuntimeError("CSU_API_KEY is not configured. Run tools/save_csu_api_key.ps1 first.")
    raw = json.dumps(request_body, ensure_ascii=False).encode("utf-8")
    request = urlrequest.Request(
        f"{config['baseUrl']}/chat/completions",
        data=raw,
        method="POST",
        headers={
            "Authorization": f"Bearer {config['apiKey']}",
            "Content-Type": "application/json",
        },
    )
    try:
        with urlrequest.urlopen(request, timeout=timeout) as response:
            return json.loads(response.read().decode("utf-8", errors="replace"))
    except urlerror.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        if exc.code == 403 and ("openresty" in body.lower() or "Forbidden" in body):
            if config["provider"] == "csu":
                raise RuntimeError(
                    "CSU API HTTP 403: gateway rejected the request. "
                    "The CSU tutorial/API appears to require campus network/VPN access or enabled token permission."
                ) from exc
            raise RuntimeError(
                "Aliyun API HTTP 403: request was rejected. Check DashScope API key permissions, region, and BASE_URL."
            ) from exc
        raise RuntimeError(f"CSU API HTTP {exc.code}: {body[:500]}") from exc
    except urlerror.URLError as exc:
        raise RuntimeError(f"CSU API request failed: {exc.reason}") from exc


def call_csu_vision_api(data_url: str, model: str, prompt: str, provider: str | None = None) -> dict[str, object]:
    config = chinese_api_config(provider)
    request_body = {
        "model": model or config["model"],
        "temperature": 0,
        "max_tokens": 512,
        "messages": [
            {
                "role": "system",
                "content": (
                    "You are a Chinese handwriting recognizer. Return compact JSON only, "
                    "with fields text, candidates, and note. candidates should be an array "
                    "of objects with text and confidence if confidence is available."
                ),
            },
            {
                "role": "user",
                "content": [
                    {
                        "type": "text",
                        "text": prompt
                        or (
                            "Please recognize the Chinese handwritten character or short text in the image. "
                            "Return JSON only, no Markdown."
                        ),
                    },
                    {
                        "type": "image_url",
                        "image_url": {"url": normalize_image_data_url(data_url)},
                    },
                ],
            },
        ],
    }
    response_payload = post_csu_chat_completion(request_body, timeout=60, provider=str(config["provider"]))

    choices = response_payload.get("choices", []) if isinstance(response_payload, dict) else []
    message = choices[0].get("message", {}) if choices and isinstance(choices[0], dict) else {}
    content = message.get("content", "") if isinstance(message, dict) else ""
    if isinstance(content, list):
        raw_text = "\n".join(str(part.get("text", part)) if isinstance(part, dict) else str(part) for part in content)
    else:
        raw_text = str(content)
    parsed = extract_json_object(raw_text)
    text, candidates = normalize_chinese_candidates(parsed, raw_text)
    return {
        "provider": config["provider"],
        "model": request_body["model"],
        "rawText": raw_text,
        "parsed": parsed,
        "text": text,
        "candidates": candidates,
        "usage": response_payload.get("usage") if isinstance(response_payload, dict) else None,
    }


def chinese_models_payload() -> dict[str, object]:
    config = chinese_api_config()
    if not config["configured"]:
        return {
            "ok": True,
            "configured": False,
            "models": [],
            "message": "CSU_API_KEY is not configured yet.",
        }
    request = urlrequest.Request(
        f"{config['baseUrl']}/models",
        method="GET",
        headers={"Authorization": f"Bearer {config['apiKey']}"},
    )
    try:
        with urlrequest.urlopen(request, timeout=30) as response:
            payload = json.loads(response.read().decode("utf-8", errors="replace"))
    except urlerror.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        return {"ok": False, "configured": True, "provider": config["provider"], "models": [], "error": f"HTTP {exc.code}: {body[:500]}"}
    except urlerror.URLError as exc:
        return {"ok": False, "configured": True, "models": [], "error": str(exc.reason)}
    models = []
    for item in payload.get("data", []) if isinstance(payload, dict) else []:
        if isinstance(item, dict):
            models.append({"id": item.get("id", ""), "ownedBy": item.get("owned_by", "")})
    return {
        "ok": True,
        "configured": True,
        "provider": config["provider"],
        "baseUrl": config["baseUrl"],
        "models": models,
    }


def probe_image_data_url() -> str:
    image = Image.new("RGB", (64, 64), (8, 14, 26))
    # A tiny stroke image is enough to check whether the endpoint accepts image_url content.
    draw = ImageDraw.Draw(image)
    for offset in range(4):
        for xy in (
            [(16, 16 + offset), (46, 16 + offset)],
            [(46 - offset, 16), (20 - offset, 48)],
            [(18, 48 + offset), (50, 48 + offset)],
        ):
            draw.line(xy, fill=(255, 255, 255), width=1)
    buffer = BytesIO()
    image.save(buffer, format="PNG")
    return "data:image/png;base64," + base64.b64encode(buffer.getvalue()).decode("ascii")


def compact_api_text(payload: dict[str, object]) -> str:
    choices = payload.get("choices", []) if isinstance(payload, dict) else []
    if not choices or not isinstance(choices[0], dict):
        return ""
    message = choices[0].get("message", {})
    content = message.get("content", "") if isinstance(message, dict) else ""
    if isinstance(content, list):
        return " ".join(str(part.get("text", part)) if isinstance(part, dict) else str(part) for part in content)
    return str(content)


def probe_csu_models_payload(payload: dict[str, object]) -> dict[str, object]:
    provider = str(payload.get("provider") or "").strip() or None
    config = chinese_api_config(provider)
    if not config["configured"]:
        missing = "ALIYUN_API_KEY" if config["provider"] == "aliyun" else "CSU_API_KEY"
        return {"ok": True, "configured": False, "provider": config["provider"], "results": [], "message": f"{missing} is not configured yet."}

    requested = payload.get("models")
    candidates: list[str] = []
    if isinstance(requested, list):
        candidates.extend(str(item).strip() for item in requested if str(item).strip())
    current_model = str(config.get("model") or "").strip()
    if current_model:
        candidates.insert(0, current_model)
    candidates.extend(config.get("candidates", []))

    deduped: list[str] = []
    seen: set[str] = set()
    for model in candidates:
        key = model.lower()
        if key and key not in seen:
            deduped.append(model)
            seen.add(key)
    deduped = deduped[: int(payload.get("limit", 18) or 18)]

    image_url = probe_image_data_url()
    results: list[dict[str, object]] = []
    for model in deduped:
        request_body = {
            "model": model,
            "temperature": 0,
            "max_tokens": 24,
            "messages": [
                {
                    "role": "user",
                    "content": [
                        {"type": "text", "text": "Describe this image in 8 words or fewer."},
                        {"type": "image_url", "image_url": {"url": image_url}},
                    ],
                }
            ],
        }
        try:
            response_payload = post_csu_chat_completion(request_body, timeout=20, provider=str(config["provider"]))
            results.append(
                {
                    "model": model,
                    "visionOk": True,
                    "ok": True,
                    "reply": compact_api_text(response_payload)[:120],
                }
            )
        except Exception as exc:
            results.append(
                {
                    "model": model,
                    "visionOk": False,
                    "ok": False,
                    "error": str(exc)[:240],
                }
            )

    recommended = next((item["model"] for item in results if item.get("visionOk")), "")
    return {
        "ok": True,
        "configured": True,
        "provider": config["provider"],
        "baseUrl": config["baseUrl"],
        "recommended": recommended,
        "results": results,
    }


def chinese_infer_payload(payload: dict[str, object]) -> dict[str, object]:
    image_data = str(payload.get("image", ""))
    raw = image_from_data_url(image_data)
    preview_dir = ROOT_DIR / "host_app" / "captures" / "chinese"
    preview_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    preview_name = f"chinese_{stamp}.png"
    raw.save(preview_dir / preview_name)
    provider = str(payload.get("provider") or "").strip() or None
    status = chinese_status_payload(provider)
    remote = status.get("remote", {}) if isinstance(status.get("remote"), dict) else {}
    if not remote.get("configured"):
        missing = "ALIYUN_API_KEY" if remote.get("provider") == "aliyun" else "CSU_API_KEY"
        return {
            "ok": True,
            "available": False,
            "candidates": [],
            "savedPreview": str(preview_dir / preview_name),
            "message": f"{missing} is not configured yet. Run the matching save script, then refresh the dashboard.",
            "recommendation": status["recommended"],
            "remote": remote,
            "localRuntimes": status["localRuntimes"],
        }

    api_result = call_csu_vision_api(
        image_data,
        str(payload.get("model") or remote.get("model") or DEFAULT_CSU_MODEL),
        str(payload.get("prompt") or ""),
        str(remote.get("provider") or ""),
    )
    return {
        "ok": True,
        "available": True,
        "savedPreview": str(preview_dir / preview_name),
        "message": "Chinese recognition completed through the CSU OpenAI-compatible vision API.",
        **api_result,
        "recommendation": status["recommended"],
    }


def run_deploy(payload: dict[str, object]) -> dict[str, object]:
    action = str(payload.get("action", "build"))
    domain = str(payload.get("domain", "digit"))
    model = str(payload.get("model", "fnn"))
    epochs = int(payload.get("epochs", 3))
    batch_size = int(payload.get("batchSize", 512))
    uv4 = str(payload.get("uv4", "")).strip()
    augment = bool(payload.get("augment", True))

    if domain not in {"digit", "letter"}:
        raise ValueError("domain must be digit or letter")
    if domain == "digit" and model not in {"all", "perceptron", "fnn", "cnn"}:
        raise ValueError("digit model must be all, perceptron, fnn, or cnn")
    if domain == "letter" and model not in {"all", *LETTER_MODEL_CHOICES.keys()}:
        raise ValueError("letter model must be all, letter_perceptron, letter_fnn, or letter_ds_cnn")

    command = [
        sys.executable,
        str(TOOLS_DIR / "keil_flash.py"),
        "--action",
        action,
        "--domain",
        domain,
        "--model",
        model,
        "--epochs",
        str(epochs),
        "--batch-size",
        str(batch_size),
    ]
    if augment:
        command.append("--augment")
    if uv4:
        command.extend(["--uv4", uv4])

    completed = subprocess.run(
        command,
        cwd=ROOT_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if domain == "digit" and "export" in action and completed.returncode == 0:
        reload_models()
    output_lines = [line.strip() for line in completed.stdout.splitlines() if line.strip()]
    error = ""
    if completed.returncode != 0:
        priority = ("AXF not found", "L6047U", "Flash Download failed", "No Debug Unit", "Target not created", "Error:")
        for marker in priority:
            match = next((line for line in output_lines if marker in line), "")
            if match:
                error = match
                break
        if not error and output_lines:
            error = output_lines[-1]
    return {
        "ok": completed.returncode == 0,
        "returnCode": completed.returncode,
        "command": command,
        "output": completed.stdout,
        "error": error,
        "domain": domain,
    }


def run_batch_test(payload: dict[str, object]) -> dict[str, object]:
    domain = str(payload.get("domain", "digit")).strip().lower()
    requested_model = str(payload.get("model", "all"))
    if domain not in {"digit", "letter"}:
        raise ValueError("domain must be digit or letter")

    model_map = {
        "digit": {
            "p": "perceptron",
            "f": "fnn",
            "c": "cnn",
            "perceptron": "perceptron",
            "fnn": "fnn",
            "cnn": "cnn",
        },
        "letter": {
            "p": "letter_perceptron",
            "f": "letter_fnn",
            "c": "letter_ds_cnn",
            "letter_perceptron": "letter_perceptron",
            "letter_fnn": "letter_fnn",
            "letter_cnn": "letter_cnn",
            "letter_ds_cnn": "letter_ds_cnn",
        },
    }
    model_meta = {
        "perceptron": ("P", "Perceptron"),
        "fnn": ("F", "FNN"),
        "cnn": ("C", "Tiny-CNN"),
        "letter_perceptron": ("P", "Letter-Perceptron"),
        "letter_fnn": ("F", "Letter-FNN"),
        "letter_cnn": ("C", "Letter-Tiny-CNN"),
        "letter_ds_cnn": ("C", "Letter-DS-CNN"),
    }
    ordered_models = {
        "digit": ["perceptron", "fnn", "cnn"],
        "letter": ["letter_perceptron", "letter_fnn", "letter_ds_cnn"],
    }
    if requested_model == "all":
        model_names = ordered_models[domain]
    else:
        model_key = requested_model.lower()
        if model_key not in model_map[domain]:
            raise ValueError("model must be all, p, f, or c for the selected domain")
        model_names = [model_map[domain][model_key]]

    if domain == "letter":
        label_names = [chr(ord("A") + index) for index in range(26)]
        datasets = [
            ("standard", "EMNIST Letters", ROOT_DIR / "tf_card" / "emnist_letters"),
            ("personal", "Collected letters", ROOT_DIR / "tf_card" / "letters_collected"),
        ]
    else:
        label_names = [str(index) for index in range(10)]
        datasets = [
            ("standard", "MNIST standard", ROOT_DIR / "tf_card" / "mnist"),
            ("personal", "Personal handwriting", ROOT_DIR / "tf_card" / "personal"),
        ]
    results: list[dict[str, object]] = []
    for key, name, set_dir in datasets:
        if not (set_dir / "label.txt").exists():
            results.append(
                {
                    "dataset": key,
                    "datasetName": name,
                    "setDir": str(set_dir),
                    "error": "label.txt not found",
                    "results": [],
                }
            )
            continue

        model_results: list[dict[str, object]] = []
        for model in model_names:
            quant_file = default_quant_file(model)
            if not quant_file.exists():
                short_name, display_name = model_meta.get(model, ("?", model))
                model_results.append({
                    "model": model,
                    "modelShort": short_name,
                    "modelName": display_name,
                    "error": f"missing {quant_file.name}",
                })
                continue
            result = run_batch(set_dir, model, quant_file, verbose=False, label_names=label_names)
            short_name, display_name = model_meta.get(model, ("?", model))
            model_results.append(
                {
                    "model": result["model"],
                    "modelShort": short_name,
                    "modelName": display_name,
                    "total": result["total"],
                    "correct": result["correct"],
                    "accuracy": result["accuracy"],
                    "avgTimeUs": result["avg_time_us"],
                    "confusions": result.get("confusions", [])[:8],
                }
            )
        results.append({"dataset": key, "datasetName": name, "setDir": str(set_dir), "results": model_results})

    return {"ok": True, "domain": domain, "model": requested_model, "datasets": results}


class DashboardHandler(BaseHTTPRequestHandler):
    server_version = "DigitNNWeb/1.0"

    def log_message(self, format: str, *args: object) -> None:
        print(f"{self.address_string()} - {format % args}")

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        path = parsed.path
        if path == "/api/status":
            uv4 = find_uv4(None)
            json_response(
                self,
                {
                    "ok": True,
                    "labels": CLASS_LABELS,
                    "models": [
                        {"model": model, "short": short_name, "name": display_name, "available": model in load_models()}
                        for model, short_name, display_name in MODEL_SPECS
                    ],
                    "letterModels": [
                        {
                            "model": model,
                            "short": LETTER_MODEL_SHORT.get(model, "?"),
                            "name": name,
                            "available": (ROOT_DIR / "models" / f"{model}_quant.npz").exists(),
                        }
                        for model, name in LETTER_MODEL_CHOICES.items()
                    ],
                    "firmwareDomains": {
                        "digit": {
                            "models": ["perceptron", "fnn", "cnn"],
                            "includes": "digit P/F/C only",
                            "build": True,
                            "flash": True,
                        },
                        "letter": {
                            "models": list(LETTER_MODEL_CHOICES),
                            "includes": "letter P/F/C only",
                            "build": True,
                            "flash": True,
                        },
                    },
                    "activeFirmwareDomain": active_firmware_domain(),
                    "usage": usage_payload(),
                    "uv4": str(uv4) if uv4 else "",
                    "saveDir": str(ROOT_DIR / "tf_card" / "ui_collected"),
                    "letterSaveDir": str(ROOT_DIR / "tf_card" / "letters_collected"),
                },
            )
            return
        if path == "/api/usage":
            query = parse_qs(parsed.query)
            json_response(self, usage_payload(query.get("domain", [""])[0] or None))
            return
        if path == "/api/quantization-profile":
            json_response(self, quantization_profile_payload())
            return
        if path in {"/api/tf-card/status", "/api/sd-card/status"}:
            query = parse_qs(parsed.query)
            json_response(self, sd_card_status_payload(query.get("path", [""])[0] or None))
            return
        if path == "/api/chinese/status":
            query = parse_qs(parsed.query)
            provider = query.get("provider", [""])[0] or None
            json_response(self, chinese_status_payload(provider))
            return
        if path == "/api/chinese/models":
            payload = chinese_models_payload()
            json_response(self, payload)
            return
        if path == "/api/serial/ports":
            json_response(self, SERIAL_BRIDGE.list_ports())
            return
        if path == "/api/serial/read":
            query = parse_qs(parsed.query)
            since = int(query.get("since", ["0"])[0] or 0)
            json_response(self, SERIAL_BRIDGE.read_since(since))
            return
        self.serve_static(path)

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        try:
            payload = read_json(self)
            if parsed.path == "/api/infer":
                raw = image_from_data_url(str(payload.get("image", "")))
                json_response(self, infer_image(raw, bool(payload.get("thicken", False)), bool(payload.get("deskew", True))))
                return
            if parsed.path == "/api/infer-pixels":
                json_response(self, infer_pixels_payload(payload))
                return
            if parsed.path == "/api/save-sample":
                raw = image_from_data_url(str(payload.get("image", "")))
                output_dir = Path(str(payload.get("outputDir") or ROOT_DIR / "tf_card" / "ui_collected")).expanduser()
                json_response(
                    self,
                    save_sample(
                        raw,
                        str(payload.get("label", "0")),
                        bool(payload.get("thicken", False)),
                        bool(payload.get("deskew", True)),
                        output_dir,
                    ),
                )
                return
            if parsed.path == "/api/save-letter-sample":
                raw = image_from_data_url(str(payload.get("image", "")))
                output_dir = Path(str(payload.get("outputDir") or ROOT_DIR / "tf_card" / "letters_collected")).expanduser()
                json_response(self, save_letter_sample(raw, str(payload.get("label", "A")), output_dir))
                return
            if parsed.path == "/api/save-board-sample":
                output_dir = Path(str(payload.get("outputDir") or ROOT_DIR / "tf_card" / "ui_collected")).expanduser()
                json_response(
                    self,
                    save_board_sample(
                        payload.get("pixels", []),
                        int(payload.get("width", 28)),
                        int(payload.get("height", 28)),
                        str(payload.get("label", "0")),
                        output_dir,
                        payload.get("results"),
                    ),
                )
                return
            if parsed.path == "/api/deploy":
                result = run_deploy(payload)
                json_response(self, result, HTTPStatus.OK if result["ok"] else HTTPStatus.INTERNAL_SERVER_ERROR)
                return
            if parsed.path == "/api/batch-test":
                json_response(self, run_batch_test(payload))
                return
            if parsed.path == "/api/chinese/infer":
                json_response(self, chinese_infer_payload(payload))
                return
            if parsed.path == "/api/chinese/probe-models":
                json_response(self, probe_csu_models_payload(payload))
                return
            if parsed.path == "/api/serial/connect":
                port = str(payload.get("port", ""))
                baudrate = int(payload.get("baudrate", 115200))
                json_response(self, SERIAL_BRIDGE.connect(port, baudrate))
                return
            if parsed.path == "/api/serial/disconnect":
                json_response(self, SERIAL_BRIDGE.disconnect())
                return
        except Exception as exc:
            error_response(self, str(exc), HTTPStatus.INTERNAL_SERVER_ERROR)
            return
        error_response(self, f"unknown endpoint: {parsed.path}", HTTPStatus.NOT_FOUND)

    def serve_static(self, request_path: str) -> None:
        if request_path in {"/", ""}:
            file_path = WEB_DIR / "index.html"
        else:
            relative = unquote(request_path.lstrip("/"))
            file_path = (WEB_DIR / relative).resolve()
            if WEB_DIR.resolve() not in file_path.parents and file_path != WEB_DIR.resolve():
                error_response(self, "invalid path", HTTPStatus.FORBIDDEN)
                return

        if not file_path.exists() or not file_path.is_file():
            error_response(self, "not found", HTTPStatus.NOT_FOUND)
            return

        content_type, _encoding = mimetypes.guess_type(str(file_path))
        body = file_path.read_bytes()
        if content_type and (content_type.startswith("text/") or content_type in {"application/javascript", "application/json"}):
            content_type = f"{content_type}; charset=utf-8"
        self.send_response(HTTPStatus.OK)
        self.send_header("Content-Type", content_type or "application/octet-stream")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8765)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    os.chdir(ROOT_DIR)
    load_models()
    server = ThreadingHTTPServer((args.host, args.port), DashboardHandler)
    print(f"DigitNN web dashboard: http://{args.host}:{args.port}/")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("stopped")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
