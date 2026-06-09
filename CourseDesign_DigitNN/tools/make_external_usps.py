"""Create a TF-card BMP test set from the public USPS handwritten digit dataset."""

from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path

import numpy as np
from PIL import Image
from sklearn.datasets import fetch_openml


ROOT_DIR = Path(__file__).resolve().parents[1]


def select_balanced(data: np.ndarray, labels: np.ndarray, per_class: int) -> list[tuple[np.ndarray, int]]:
    selected: list[tuple[np.ndarray, int]] = []
    counts: dict[int, int] = defaultdict(int)

    for row, raw_label in zip(data, labels):
        label = int(raw_label) - 1
        if counts[label] >= per_class:
            continue
        selected.append((row.reshape(16, 16), label))
        counts[label] += 1
        if all(counts[digit] >= per_class for digit in range(10)):
            break

    missing = [digit for digit in range(10) if counts[digit] < per_class]
    if missing:
        raise SystemExit(f"not enough USPS samples for classes: {missing}")

    return selected


def to_mnist_style_image(pixels_16x16: np.ndarray) -> Image.Image:
    pixels = ((pixels_16x16.astype(np.float32) + 1.0) * 127.5).clip(0, 255).astype(np.uint8)
    image = Image.fromarray(pixels, mode="L")
    canvas = Image.new("L", (28, 28), 0)
    image = image.resize((22, 22), Image.Resampling.BICUBIC)
    canvas.paste(image, ((28 - image.width) // 2, (28 - image.height) // 2))
    return canvas


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=ROOT_DIR / "tf_card" / "external_usps")
    parser.add_argument("--per-class", type=int, default=10)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    dataset = fetch_openml("usps", version=2, as_frame=False, parser="auto")
    samples = select_balanced(dataset.data, dataset.target, args.per_class)
    args.output_dir.mkdir(parents=True, exist_ok=True)

    labels: list[str] = []
    for index, (pixels, label) in enumerate(samples):
        filename = f"usps_{index:04d}_{label}.bmp"
        to_mnist_style_image(pixels).save(args.output_dir / filename)
        labels.append(f"{filename},{label}")

    (args.output_dir / "label.txt").write_text("\n".join(labels) + "\n", encoding="utf-8")
    print(f"exported {len(labels)} USPS BMP files to {args.output_dir}")


if __name__ == "__main__":
    main()
