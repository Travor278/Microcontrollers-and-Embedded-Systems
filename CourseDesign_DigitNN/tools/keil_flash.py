"""Build, flash, and refresh model weights for the Keil DigitNN project."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
PROJECT_PATH = ROOT_DIR / "keil_touch_digit_nn" / "Project" / "RVMDK（uv5）" / "BH-F103.uvprojx"
LOG_DIR = ROOT_DIR / "keil_touch_digit_nn" / "Output"
OUTPUT_AXF = LOG_DIR / "DigitNN_Touch.axf"


def find_uv4(explicit_path: str | None) -> Path | None:
    if explicit_path:
        path = Path(explicit_path).expanduser()
        return path if path.exists() else None

    env_path = os.environ.get("KEIL_UV4")
    if env_path:
        path = Path(env_path).expanduser()
        if path.exists():
            return path

    path_from_env = shutil.which("UV4.exe") or shutil.which("UV4")
    if path_from_env:
        return Path(path_from_env)

    running_uv4 = find_running_uv4()
    if running_uv4 is not None:
        return running_uv4

    candidates = [
        Path("D:/UV4/UV4.exe"),
        Path("C:/Keil_v5/UV4/UV4.exe"),
        Path("C:/Keil/UV4/UV4.exe"),
        Path("D:/Keil_v5/UV4/UV4.exe"),
        Path("D:/Keil/UV4/UV4.exe"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def find_running_uv4() -> Path | None:
    if os.name != "nt":
        return None
    try:
        completed = subprocess.run(
            [
                "powershell",
                "-NoProfile",
                "-Command",
                (
                    "Get-CimInstance Win32_Process -Filter \"Name='UV4.exe'\" | "
                    "Select-Object -First 1 -ExpandProperty ExecutablePath"
                ),
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=2,
        )
    except Exception:
        return None
    path_text = completed.stdout.strip().splitlines()[0] if completed.stdout.strip() else ""
    if not path_text:
        return None
    path = Path(path_text)
    return path if path.exists() else None


def run_command(command: list[str], dry_run: bool) -> int:
    print(" ".join(f'"{item}"' if " " in item else item for item in command))
    if dry_run:
        return 0

    completed = subprocess.run(command, cwd=ROOT_DIR)
    return int(completed.returncode)


def run_keil(uv4: Path, action: str, project: Path, target: str | None, dry_run: bool) -> int:
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_path = LOG_DIR / f"keil_{action}_{timestamp}.log"
    command = [str(uv4), f"-{action}", str(project), "-j0", "-o", str(log_path)]
    if target:
        command.extend(["-t", target])

    exit_code = run_command(command, dry_run)
    if log_path.exists():
        print(log_path)
        print(log_path.read_text(encoding="utf-8", errors="replace"))
    return exit_code


def export_model(model: str, epochs: int, batch_size: int, augment: bool, dry_run: bool) -> int:
    command = [
        sys.executable,
        str(ROOT_DIR / "tools" / "train_mnist.py"),
        "--model",
        model,
        "--epochs",
        str(epochs),
        "--batch-size",
        str(batch_size),
        "--export-c",
        "--export-keil",
    ]
    if augment:
        command.append("--augment")
    return run_command(command, dry_run)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--action",
        choices=["build", "rebuild", "flash", "build-flash", "export-model", "export-build-flash"],
        default="build",
    )
    parser.add_argument("--model", choices=["perceptron", "fnn", "cnn"], default="fnn")
    parser.add_argument("--epochs", type=int, default=3)
    parser.add_argument("--batch-size", type=int, default=512)
    parser.add_argument("--augment", action="store_true")
    parser.add_argument("--uv4", default=None, help="Path to UV4.exe. Also supports KEIL_UV4 env var.")
    parser.add_argument("--project", type=Path, default=PROJECT_PATH)
    parser.add_argument("--target", default=None, help="Optional Keil target name.")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if not args.project.exists():
        raise SystemExit(f"Keil project not found: {args.project}")

    if args.action in {"export-model", "export-build-flash"}:
        exit_code = export_model(args.model, args.epochs, args.batch_size, args.augment, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)

    if args.action == "export-model":
        return

    uv4 = find_uv4(args.uv4)
    if uv4 is None:
        if args.dry_run:
            uv4 = Path("UV4.exe")
        else:
            raise SystemExit("UV4.exe not found. Set KEIL_UV4 or pass --uv4 C:\\Keil_v5\\UV4\\UV4.exe")

    if args.action == "flash" and not args.dry_run and not OUTPUT_AXF.exists():
        raise SystemExit(
            f"AXF not found: {OUTPUT_AXF}. Run a successful build first. "
            "If Keil reports L6047U, the current MDK license cannot link this image size."
        )

    if args.action in {"build", "build-flash", "export-build-flash"}:
        exit_code = run_keil(uv4, "b", args.project, args.target, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)
    elif args.action == "rebuild":
        exit_code = run_keil(uv4, "r", args.project, args.target, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)

    if args.action in {"flash", "build-flash", "export-build-flash"}:
        exit_code = run_keil(uv4, "f", args.project, args.target, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)


if __name__ == "__main__":
    main()
