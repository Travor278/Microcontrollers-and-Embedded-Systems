"""Serial dashboard for the STM32 handwritten digit recognition firmware."""

from __future__ import annotations

import argparse
import csv
from datetime import datetime
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Serial port, for example COM3.")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--log", type=Path, default=ROOT_DIR / "models" / "serial_log.csv")
    return parser.parse_args()


def parse_frame(line: str) -> dict[str, str]:
    parts = line.strip().split(",")
    frame: dict[str, str] = {"type": parts[0] if parts else ""}
    for part in parts[1:]:
        if "=" not in part:
            continue
        key, value = part.split("=", maxsplit=1)
        frame[key.strip()] = value.strip()
    return frame


def main() -> None:
    args = parse_args()
    args.log.parent.mkdir(parents=True, exist_ok=True)

    try:
        import serial
    except ModuleNotFoundError as exc:
        raise SystemExit("pyserial is required: python -m pip install pyserial") from exc

    with serial.Serial(args.port, args.baud, timeout=1) as ser, args.log.open("a", newline="", encoding="utf-8") as log_file:
        writer = csv.DictWriter(
            log_file,
            fieldnames=["time", "type", "model", "label", "confidence", "time_us", "set", "total", "correct", "accuracy", "avg_time_us", "state", "message"],
        )
        if log_file.tell() == 0:
            writer.writeheader()

        print(f"listening on {args.port} at {args.baud} baud")
        print("type commands such as CMD,INFO or CMD,MODEL,F in another serial tool, or press Ctrl+C to stop")

        while True:
            raw_line = ser.readline().decode("utf-8", errors="replace").strip()
            if not raw_line:
                continue
            frame = parse_frame(raw_line)
            frame["time"] = datetime.now().isoformat(timespec="seconds")
            writer.writerow({key: frame.get(key, "") for key in writer.fieldnames or []})
            log_file.flush()
            print(raw_line)


if __name__ == "__main__":
    main()
