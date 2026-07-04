"""Export an EMNIST Letters BMP test set for the letter-recognition workspace."""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Callable

import numpy as np
from PIL import Image, ImageOps
from torchvision import datasets, transforms


ROOT_DIR = Path(__file__).resolve().parents[1]
CLASS_NAMES = [chr(ord("A") + index) for index in range(26)]


def emnist_orientation(image: Image.Image) -> Image.Image:
    return ImageOps.mirror(image.rotate(-90))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT_DIR / "tf_card" / "emnist_letters")
    parser.add_argument("--data-dir", type=Path, default=ROOT_DIR / "data")
    parser.add_argument("--per-class", type=int, default=20)
    parser.add_argument("--seed", type=int, default=2026)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    for old_file in args.output.glob("*.bmp"):
        old_file.unlink()

    transform: Callable[[Image.Image], Image.Image] = emnist_orientation
    dataset = datasets.EMNIST(root=args.data_dir, split="letters", train=False, download=True, transform=transform)
    rng = np.random.default_rng(args.seed)
    by_class: dict[int, list[int]] = {index: [] for index in range(26)}

    for index, label in enumerate(dataset.targets):
        class_id = int(label) - 1
        if 0 <= class_id < 26:
            by_class[class_id].append(index)

    labels: list[str] = []
    exported = 0
    for class_id, indices in by_class.items():
        rng.shuffle(indices)
        class_name = CLASS_NAMES[class_id]
        for source_index in indices[: args.per_class]:
            image, _label = dataset[source_index]
            pixels = np.asarray(image.convert("L").resize((28, 28)), dtype=np.uint8)
            filename = f"emnist_{exported:04d}_{class_name}.bmp"
            Image.fromarray(pixels, mode="L").save(args.output / filename)
            labels.append(f"{filename},{class_name}")
            exported += 1

    (args.output / "label.txt").write_text("\n".join(labels) + "\n", encoding="utf-8")
    print(f"exported {exported} BMP files to {args.output}")


if __name__ == "__main__":
    main()
