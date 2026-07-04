"""Local web dashboard for DigitNN drawing, inference, capture, and flashing."""

from __future__ import annotations

import argparse
import base64
import csv
import json
import mimetypes
import os
import subprocess
import sys
import threading
from datetime import datetime
from collections import deque
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from io import BytesIO
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlparse

import numpy as np
from PIL import Image

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

from host_batch_test import default_quant_file, predict_scores  # noqa: E402
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


def usage_payload() -> dict[str, object]:
    usage = parse_build_usage()
    if usage is None:
        return {"ok": True, "available": False}

    return {
        "ok": True,
        "available": True,
        "flash": usage["flash"],
        "sram": usage["sram"],
        "flashPercent": usage["flash"] * 100.0 / FLASH_BYTES,
        "sramPercent": usage["sram"] * 100.0 / SRAM_BYTES,
        "detail": usage,
    }


def run_deploy(payload: dict[str, object]) -> dict[str, object]:
    action = str(payload.get("action", "build"))
    model = str(payload.get("model", "fnn"))
    epochs = int(payload.get("epochs", 3))
    batch_size = int(payload.get("batchSize", 512))
    uv4 = str(payload.get("uv4", "")).strip()
    augment = bool(payload.get("augment", True))

    command = [
        sys.executable,
        str(TOOLS_DIR / "keil_flash.py"),
        "--action",
        action,
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
    if "export" in action and completed.returncode == 0:
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
    }


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
                    "usage": usage_payload(),
                    "uv4": str(uv4) if uv4 else "",
                    "saveDir": str(ROOT_DIR / "tf_card" / "ui_collected"),
                },
            )
            return
        if path == "/api/usage":
            json_response(self, usage_payload())
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
