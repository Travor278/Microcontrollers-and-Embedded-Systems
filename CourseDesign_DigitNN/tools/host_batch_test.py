"""Run PC-side batch inference on BMP test sets using exported quantized weights."""

from __future__ import annotations

import argparse
import time
from pathlib import Path

import numpy as np
from PIL import Image


ROOT_DIR = Path(__file__).resolve().parents[1]


def load_labels(label_file: Path) -> dict[str, int]:
    labels: dict[str, int] = {}
    for line in label_file.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        filename, label = line.split(",", maxsplit=1)
        labels[filename.strip()] = int(label.strip())
    return labels


def load_image(path: Path) -> np.ndarray:
    image = Image.open(path).convert("L").resize((28, 28))
    return np.asarray(image, dtype=np.uint8).reshape(-1)


def predict_perceptron(pixels: np.ndarray, quant_file: Path) -> int:
    data = np.load(quant_file)
    scores = data["weight"].astype(np.int32) @ pixels.astype(np.int32) + data["bias"].astype(np.int32)
    return int(np.argmax(scores))


def predict_fnn(pixels: np.ndarray, quant_file: Path) -> int:
    data = np.load(quant_file)
    hidden = data["weight_1"].astype(np.int32) @ pixels.astype(np.int32) + data["bias_1"].astype(np.int32)
    hidden = np.maximum(hidden, 0) >> 8
    scores = data["weight_2"].astype(np.int32) @ hidden + data["bias_2"].astype(np.int32)
    return int(np.argmax(scores))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--set-dir", type=Path, default=ROOT_DIR / "testsets" / "mnist")
    parser.add_argument("--model", choices=["perceptron", "fnn"], default="perceptron")
    parser.add_argument("--quant-file", type=Path, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    quant_file = args.quant_file
    if quant_file is None:
        quant_file = ROOT_DIR / "models" / ("perceptron_quant.npz" if args.model == "perceptron" else "fnn_quant.npz")

    labels = load_labels(args.set_dir / "label.txt")
    correct = 0
    total = 0
    elapsed_ns = 0

    for filename, label in labels.items():
        pixels = load_image(args.set_dir / filename)
        start_ns = time.perf_counter_ns()
        if args.model == "perceptron":
            prediction = predict_perceptron(pixels, quant_file)
        else:
            prediction = predict_fnn(pixels, quant_file)
        elapsed_ns += time.perf_counter_ns() - start_ns
        correct += int(prediction == label)
        total += 1
        print(f"{filename}: label={label} prediction={prediction}")

    accuracy = (correct / total) if total else 0.0
    average_us = (elapsed_ns / total / 1000.0) if total else 0.0
    print(f"model={args.model} total={total} correct={correct} accuracy={accuracy:.2%} avg_time_us={average_us:.2f}")


if __name__ == "__main__":
    main()
