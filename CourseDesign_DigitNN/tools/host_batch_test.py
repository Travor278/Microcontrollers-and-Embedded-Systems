"""Run PC-side batch inference on BMP test sets using exported quantized weights."""

from __future__ import annotations

import argparse
import time
from pathlib import Path

import numpy as np
from PIL import Image


ROOT_DIR = Path(__file__).resolve().parents[1]


def label_to_index(label: str) -> int:
    text = label.strip()
    if len(text) == 1 and text.isalpha():
        return ord(text.upper()) - ord("A")
    return int(text)


def format_label(index: int, label_names: list[str] | None = None) -> str | int:
    if label_names and 0 <= index < len(label_names):
        return label_names[index]
    return index


def load_labels(label_file: Path) -> dict[str, int]:
    labels: dict[str, int] = {}
    for line in label_file.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        filename, label = line.split(",", maxsplit=1)
        labels[filename.strip()] = label_to_index(label)
    return labels


def load_image(path: Path) -> np.ndarray:
    image = Image.open(path).convert("L").resize((28, 28))
    return np.asarray(image, dtype=np.uint8).reshape(-1)


def scores_perceptron(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> np.ndarray:
    scores = data["weight"].astype(np.int32) @ pixels.astype(np.int32) + data["bias"].astype(np.int32)
    return scores


def predict_perceptron(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> int:
    return int(np.argmax(scores_perceptron(pixels, data)))


def scores_fnn(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> np.ndarray:
    hidden = data["weight_1"].astype(np.int32) @ pixels.astype(np.int32) + data["bias_1"].astype(np.int32)
    hidden = np.maximum(hidden, 0) >> 8
    scores = data["weight_2"].astype(np.int32) @ hidden + data["bias_2"].astype(np.int32)
    return scores


def predict_fnn(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> int:
    return int(np.argmax(scores_fnn(pixels, data)))


def conv_relu_pool2d(inputs: np.ndarray, weights: np.ndarray, bias: np.ndarray, shift: int) -> np.ndarray:
    input_channels, height, width = inputs.shape
    output_channels = weights.shape[0]
    pooled = np.zeros((output_channels, height // 2, width // 2), dtype=np.int32)

    for output_channel in range(output_channels):
        for pool_y in range(height // 2):
            for pool_x in range(width // 2):
                max_value = 0
                for dy in range(2):
                    for dx in range(2):
                        y = pool_y * 2 + dy
                        x = pool_x * 2 + dx
                        value = int(bias[output_channel])
                        for input_channel in range(input_channels):
                            for kernel_y in range(3):
                                source_y = y + kernel_y - 1
                                if source_y < 0 or source_y >= height:
                                    continue
                                for kernel_x in range(3):
                                    source_x = x + kernel_x - 1
                                    if source_x < 0 or source_x >= width:
                                        continue
                                    value += int(inputs[input_channel, source_y, source_x]) * int(weights[output_channel, input_channel, kernel_y, kernel_x])
                        if value > max_value:
                            max_value = value
                pooled[output_channel, pool_y, pool_x] = max_value >> shift
    return pooled


def depthwise_relu2d(inputs: np.ndarray, weights: np.ndarray, bias: np.ndarray, shift: int) -> np.ndarray:
    channels, height, width = inputs.shape
    output = np.zeros((channels, height, width), dtype=np.int32)

    for channel in range(channels):
        for y in range(height):
            for x in range(width):
                value = int(bias[channel])
                for kernel_y in range(3):
                    source_y = y + kernel_y - 1
                    if source_y < 0 or source_y >= height:
                        continue
                    for kernel_x in range(3):
                        source_x = x + kernel_x - 1
                        if source_x < 0 or source_x >= width:
                            continue
                        value += int(inputs[channel, source_y, source_x]) * int(weights[channel, kernel_y, kernel_x])
                output[channel, y, x] = max(value, 0) >> shift
    return output


def pointwise_relu2d(inputs: np.ndarray, weights: np.ndarray, bias: np.ndarray, shift: int) -> np.ndarray:
    input_channels, height, width = inputs.shape
    output_channels = weights.shape[0]
    output = np.zeros((output_channels, height, width), dtype=np.int32)

    for output_channel in range(output_channels):
        for y in range(height):
            for x in range(width):
                value = int(bias[output_channel])
                for input_channel in range(input_channels):
                    value += int(inputs[input_channel, y, x]) * int(weights[output_channel, input_channel])
                output[output_channel, y, x] = max(value, 0) >> shift
    return output


def pointwise_relu_pool2d(inputs: np.ndarray, weights: np.ndarray, bias: np.ndarray, shift: int) -> np.ndarray:
    input_channels, height, width = inputs.shape
    output_channels = weights.shape[0]
    pooled = np.zeros((output_channels, height // 2, width // 2), dtype=np.int32)

    for output_channel in range(output_channels):
        for pool_y in range(height // 2):
            for pool_x in range(width // 2):
                max_value = 0
                for dy in range(2):
                    for dx in range(2):
                        y = pool_y * 2 + dy
                        x = pool_x * 2 + dx
                        value = int(bias[output_channel])
                        for input_channel in range(input_channels):
                            value += int(inputs[input_channel, y, x]) * int(weights[output_channel, input_channel])
                        if value > max_value:
                            max_value = value
                pooled[output_channel, pool_y, pool_x] = max_value >> shift
    return pooled


def scores_cnn(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> np.ndarray:
    image = pixels.astype(np.int32).reshape(1, 28, 28)
    conv1_weight = data["conv1_weight"].astype(np.int32)[:, np.newaxis, :, :]
    conv1 = conv_relu_pool2d(image, conv1_weight, data["conv1_bias"].astype(np.int32), int(data["conv1_shift"][0]))
    conv2 = conv_relu_pool2d(conv1, data["conv2_weight"].astype(np.int32), data["conv2_bias"].astype(np.int32), int(data["conv2_shift"][0]))
    features = conv2.reshape(-1)
    scores = data["fc_weight"].astype(np.int32) @ features + data["fc_bias"].astype(np.int32)
    return scores


def predict_cnn(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> int:
    return int(np.argmax(scores_cnn(pixels, data)))


def scores_ds_cnn(pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> np.ndarray:
    image = pixels.astype(np.int32).reshape(1, 28, 28)
    conv1_weight = data["conv1_weight"].astype(np.int32)[:, np.newaxis, :, :]
    conv1 = conv_relu_pool2d(image, conv1_weight, data["conv1_bias"].astype(np.int32), int(data["conv1_shift"][0]))
    ds1_depth = depthwise_relu2d(
        conv1,
        data["ds1_dw_weight"].astype(np.int32),
        data["ds1_dw_bias"].astype(np.int32),
        int(data["ds1_dw_shift"][0]),
    )
    ds1_pool = pointwise_relu_pool2d(
        ds1_depth,
        data["ds1_pw_weight"].astype(np.int32),
        data["ds1_pw_bias"].astype(np.int32),
        int(data["ds1_pw_shift"][0]),
    )
    ds2_depth = depthwise_relu2d(
        ds1_pool,
        data["ds2_dw_weight"].astype(np.int32),
        data["ds2_dw_bias"].astype(np.int32),
        int(data["ds2_dw_shift"][0]),
    )
    ds2_features = pointwise_relu2d(
        ds2_depth,
        data["ds2_pw_weight"].astype(np.int32),
        data["ds2_pw_bias"].astype(np.int32),
        int(data["ds2_pw_shift"][0]),
    )
    features = ds2_features.reshape(-1)
    return data["fc_weight"].astype(np.int32) @ features + data["fc_bias"].astype(np.int32)


def default_quant_file(model: str) -> Path:
    if model in {"perceptron", "letter_perceptron"}:
        if model.startswith("letter_"):
            return ROOT_DIR / "models" / "letter_perceptron_quant.npz"
        return ROOT_DIR / "models" / "perceptron_quant.npz"
    if model in {"fnn", "letter_fnn"}:
        if model.startswith("letter_"):
            return ROOT_DIR / "models" / "letter_fnn_quant.npz"
        return ROOT_DIR / "models" / "fnn_quant.npz"
    if model == "letter_ds_cnn":
        return ROOT_DIR / "models" / "letter_ds_cnn_quant.npz"
    if model == "letter_cnn":
        return ROOT_DIR / "models" / "letter_cnn_quant.npz"
    return ROOT_DIR / "models" / "cnn_quant.npz"


def predict(model: str, pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> int:
    return int(np.argmax(predict_scores(model, pixels, data)))


def predict_scores(model: str, pixels: np.ndarray, data: np.lib.npyio.NpzFile) -> np.ndarray:
    if model in {"perceptron", "letter_perceptron"}:
        return scores_perceptron(pixels, data)
    if model in {"fnn", "letter_fnn"}:
        return scores_fnn(pixels, data)
    if model == "letter_ds_cnn":
        return scores_ds_cnn(pixels, data)
    return scores_cnn(pixels, data)


def run_batch(
    set_dir: Path,
    model: str,
    quant_file: Path,
    verbose: bool = True,
    label_names: list[str] | None = None,
) -> dict[str, object]:
    labels = load_labels(set_dir / "label.txt")
    correct = 0
    total = 0
    elapsed_ns = 0
    confusion_counts: dict[tuple[int, int], int] = {}
    confusion_examples: dict[tuple[int, int], list[str]] = {}

    with np.load(quant_file) as data:
        for filename, label in labels.items():
            pixels = load_image(set_dir / filename)
            start_ns = time.perf_counter_ns()
            prediction = predict(model, pixels, data)
            elapsed_ns += time.perf_counter_ns() - start_ns
            correct += int(prediction == label)
            total += 1
            if prediction != label:
                pair = (label, prediction)
                confusion_counts[pair] = confusion_counts.get(pair, 0) + 1
                examples = confusion_examples.setdefault(pair, [])
                if len(examples) < 3:
                    examples.append(filename)
            if verbose:
                print(f"{filename}: label={label} prediction={prediction}")

    accuracy = (correct / total) if total else 0.0
    average_us = (elapsed_ns / total / 1000.0) if total else 0.0
    confusions = [
        {
            "truth": format_label(truth, label_names),
            "prediction": format_label(prediction, label_names),
            "truthIndex": truth,
            "predictionIndex": prediction,
            "count": count,
            "rate": (count / total) if total else 0.0,
            "examples": confusion_examples.get((truth, prediction), []),
        }
        for (truth, prediction), count in sorted(confusion_counts.items(), key=lambda item: (-item[1], item[0][0], item[0][1]))
    ]
    return {
        "model": model,
        "set_dir": str(set_dir),
        "total": total,
        "correct": correct,
        "accuracy": accuracy,
        "avg_time_us": average_us,
        "confusions": confusions,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--set-dir", type=Path, default=ROOT_DIR / "testsets" / "mnist")
    parser.add_argument(
        "--model",
        choices=["perceptron", "fnn", "cnn", "letter_perceptron", "letter_fnn", "letter_cnn", "letter_ds_cnn"],
        default="perceptron",
    )
    parser.add_argument("--quant-file", type=Path, default=None)
    parser.add_argument("--quiet", action="store_true")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    quant_file = args.quant_file
    if quant_file is None:
        quant_file = default_quant_file(args.model)

    result = run_batch(args.set_dir, args.model, quant_file, verbose=not args.quiet)
    print(
        f"model={result['model']} total={result['total']} correct={result['correct']} "
        f"accuracy={result['accuracy']:.2%} avg_time_us={result['avg_time_us']:.2f}"
    )


if __name__ == "__main__":
    main()
