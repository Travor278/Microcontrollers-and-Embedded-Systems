"""Segment a personal handwritten digit photo into MNIST-style 28x28 BMP files."""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


ROOT_DIR = Path(__file__).resolve().parents[1]


def otsu_threshold(gray: np.ndarray) -> int:
    hist = np.bincount(gray.reshape(-1), minlength=256).astype(np.float64)
    total = gray.size
    sum_total = float(np.dot(np.arange(256), hist))
    sum_background = 0.0
    weight_background = 0.0
    max_variance = -1.0
    threshold = 127

    for value in range(256):
        weight_background += hist[value]
        if weight_background == 0:
            continue

        weight_foreground = total - weight_background
        if weight_foreground == 0:
            break

        sum_background += value * hist[value]
        mean_background = sum_background / weight_background
        mean_foreground = (sum_total - sum_background) / weight_foreground
        variance = weight_background * weight_foreground * (mean_background - mean_foreground) ** 2

        if variance > max_variance:
            max_variance = variance
            threshold = value

    return threshold


def fill_short_gaps(active: np.ndarray, max_gap: int) -> np.ndarray:
    active = active.copy()
    start = None

    for index, value in enumerate(active):
        if value and start is not None:
            if index - start <= max_gap:
                active[start:index] = True
            start = None
        elif (not value) and (index == 0 or active[index - 1]):
            start = index

    return active


def find_segments(mask: np.ndarray, expected_count: int | None) -> list[tuple[int, int]]:
    column_sum = mask.sum(axis=0)
    min_column_pixels = max(8, int(mask.shape[0] * 0.025))
    active = column_sum > min_column_pixels
    active = fill_short_gaps(active, max_gap=max(12, mask.shape[1] // 80))
    segments: list[tuple[int, int]] = []
    start = None

    for index, value in enumerate(active):
        if value and start is None:
            start = index
        elif (not value) and start is not None:
            if index - start >= 12:
                segments.append((start, index - 1))
            start = None

    if start is not None and len(active) - start >= 12:
        segments.append((start, len(active) - 1))

    if expected_count is not None and len(segments) != expected_count:
        raise SystemExit(f"expected {expected_count} digits, found {len(segments)} segments: {segments}")

    return segments


def crop_digit(gray: np.ndarray, mask: np.ndarray, segment: tuple[int, int], dilate_size: int) -> Image.Image:
    x0, x1 = segment
    submask = mask[:, x0:x1 + 1]
    ys, xs = np.where(submask)

    if len(xs) == 0:
        raise ValueError("empty digit segment")

    pad = 18
    left = max(x0 + int(xs.min()) - pad, 0)
    right = min(x0 + int(xs.max()) + pad, gray.shape[1] - 1)
    top = max(int(ys.min()) - pad, 0)
    bottom = min(int(ys.max()) + pad, gray.shape[0] - 1)
    digit_gray = gray[top:bottom + 1, left:right + 1]
    digit_mask = mask[top:bottom + 1, left:right + 1]
    threshold = otsu_threshold(digit_gray)
    foreground = np.zeros_like(digit_gray, dtype=np.uint8)
    dark_values = digit_gray[digit_mask]
    dark_min = int(dark_values.min()) if dark_values.size else 0
    scale = max(threshold - dark_min, 1)
    dark_pixels = digit_gray[digit_mask].astype(np.int16)
    foreground[digit_mask] = np.clip((threshold - dark_pixels) * 255 // scale, 0, 255).astype(np.uint8)

    digit = Image.fromarray(foreground, mode="L")
    digit.thumbnail((20, 20), Image.Resampling.LANCZOS)
    canvas = Image.new("L", (28, 28), 0)
    x_offset = (28 - digit.width) // 2
    y_offset = (28 - digit.height) // 2
    canvas.paste(digit, (x_offset, y_offset))

    if dilate_size > 1:
        if dilate_size % 2 == 0:
            dilate_size += 1
        canvas = canvas.filter(ImageFilter.MaxFilter(dilate_size))

    return canvas


def write_preview(images: list[Image.Image], labels: str, output_path: Path) -> None:
    scale = 8
    cell_w = 28 * scale
    cell_h = 28 * scale + 24
    preview = Image.new("RGB", (cell_w * len(images), cell_h), "white")
    draw = ImageDraw.Draw(preview)

    for index, image in enumerate(images):
        large = image.resize((28 * scale, 28 * scale), Image.Resampling.NEAREST).convert("RGB")
        preview.paste(large, (index * cell_w, 0))
        label = labels[index] if index < len(labels) else "?"
        draw.text((index * cell_w + 4, 28 * scale + 5), f"{index}:{label}", fill="black")

    preview.save(output_path)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--labels", required=True, help="Digit labels from left to right, for example 7132564908.")
    parser.add_argument("--output-dir", type=Path, default=ROOT_DIR / "testsets" / "personal" / "processed")
    parser.add_argument("--prefix", default="number")
    parser.add_argument("--dilate-size", type=int, default=1, help="Odd MaxFilter size used to thicken strokes.")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    source = Image.open(args.input).convert("L")
    gray = np.asarray(source, dtype=np.uint8)
    threshold = otsu_threshold(gray)
    mask = gray < threshold
    segments = find_segments(mask, expected_count=len(args.labels))
    images: list[Image.Image] = []
    label_lines: list[str] = []

    for index, (segment, label) in enumerate(zip(segments, args.labels)):
        digit = crop_digit(gray, mask, segment, args.dilate_size)
        filename = f"{args.prefix}_{index:02d}_{label}.bmp"
        digit.save(args.output_dir / filename)
        images.append(digit)
        label_lines.append(f"{filename},{label}")

    (args.output_dir / "label.txt").write_text("\n".join(label_lines) + "\n", encoding="utf-8")
    write_preview(images, args.labels, args.output_dir / f"{args.prefix}_preview.png")
    print(f"threshold={threshold} segments={segments}")
    print(f"wrote {len(images)} digits to {args.output_dir}")


if __name__ == "__main__":
    main()
