"""Export a small MNIST BMP test set for TF-card batch testing."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image
from torchvision import datasets, transforms


ROOT_DIR = Path(__file__).resolve().parents[1]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT_DIR / "testsets" / "mnist")
    parser.add_argument("--data-dir", type=Path, default=ROOT_DIR / "data")
    parser.add_argument("--count", type=int, default=20)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    dataset = datasets.MNIST(root=args.data_dir, train=False, download=True, transform=transforms.ToTensor())
    labels: list[str] = []

    for index in range(min(args.count, len(dataset))):
        tensor, label = dataset[index]
        pixels = (tensor.squeeze(0).numpy() * 255.0).astype("uint8")
        image = Image.fromarray(pixels, mode="L")
        filename = f"img_{index:04d}_{int(label)}.bmp"
        image.save(args.output / filename)
        labels.append(f"{filename},{int(label)}")

    (args.output / "label.txt").write_text("\n".join(labels) + "\n", encoding="utf-8")
    print(f"exported {len(labels)} BMP files to {args.output}")


if __name__ == "__main__":
    main()
