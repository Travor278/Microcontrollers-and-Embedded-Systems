"""Train MNIST models and export quantized C arrays for STM32."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path
from typing import Iterable

import numpy as np
import torch
from torch import nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms


ROOT_DIR = Path(__file__).resolve().parents[1]
MODEL_DIR = ROOT_DIR / "models"
GENERATED_DIR = ROOT_DIR / "firmware" / "generated"
KEIL_GENERATED_DIR = ROOT_DIR / "keil_touch_digit_nn" / "User" / "digit_nn" / "generated"


class Perceptron(nn.Module):
    """Single-layer neural network for the basic task."""

    def __init__(self) -> None:
        super().__init__()
        self.linear = nn.Linear(28 * 28, 10)

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.linear(images.view(images.size(0), -1))


class FNN(nn.Module):
    """One-hidden-layer fully connected network for the advanced task."""

    def __init__(self, hidden_size: int = 64) -> None:
        super().__init__()
        self.layers = nn.Sequential(
            nn.Flatten(),
            nn.Linear(28 * 28, hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, 10),
        )

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.layers(images)


class CNN(nn.Module):
    """Tiny CNN small enough for STM32F103 integer inference."""

    def __init__(self) -> None:
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(1, 4, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
            nn.Conv2d(4, 8, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
        )
        self.classifier = nn.Sequential(
            nn.Flatten(),
            nn.Linear(8 * 7 * 7, 10),
        )

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.classifier(self.features(images))


def build_model(model_name: str) -> nn.Module:
    if model_name == "perceptron":
        return Perceptron()
    if model_name == "fnn":
        return FNN()
    if model_name == "cnn":
        return CNN()
    raise ValueError(f"unsupported model: {model_name}")


def build_loaders(data_dir: Path, batch_size: int, augment: bool) -> tuple[DataLoader, DataLoader]:
    test_transform = transforms.Compose([transforms.ToTensor()])
    if augment:
        train_transform = transforms.Compose([
            transforms.RandomAffine(degrees=12, translate=(0.10, 0.10), scale=(0.85, 1.15), shear=5, fill=0),
            transforms.ToTensor(),
        ])
    else:
        train_transform = test_transform

    train_set = datasets.MNIST(root=data_dir, train=True, download=True, transform=train_transform)
    test_set = datasets.MNIST(root=data_dir, train=False, download=True, transform=test_transform)
    train_loader = DataLoader(train_set, batch_size=batch_size, shuffle=True, num_workers=0)
    test_loader = DataLoader(test_set, batch_size=batch_size, shuffle=False, num_workers=0)
    return train_loader, test_loader


def train_one_epoch(model: nn.Module, loader: DataLoader, optimizer: torch.optim.Optimizer, device: torch.device) -> float:
    criterion = nn.CrossEntropyLoss()
    total_loss = 0.0
    model.train()

    for images, labels in loader:
        images = images.to(device)
        labels = labels.to(device)

        optimizer.zero_grad()
        logits = model(images)
        loss = criterion(logits, labels)
        loss.backward()
        optimizer.step()
        total_loss += float(loss.item()) * images.size(0)

    return total_loss / len(loader.dataset)


@torch.no_grad()
def evaluate(model: nn.Module, loader: DataLoader, device: torch.device) -> float:
    correct = 0
    total = 0
    model.eval()

    for images, labels in loader:
        images = images.to(device)
        labels = labels.to(device)
        prediction = model(images).argmax(dim=1)
        correct += int((prediction == labels).sum().item())
        total += int(labels.numel())

    return correct / total


def quantize_weight(weight: np.ndarray) -> tuple[np.ndarray, float]:
    max_abs = float(np.max(np.abs(weight)))
    scale = max_abs / 127.0 if max_abs > 0.0 else 1.0
    quantized = np.clip(np.round(weight / scale), -127, 127).astype(np.int8)
    return quantized, scale


def format_c_array(values: np.ndarray, indent: str = "    ") -> str:
    flat_values = values.reshape(-1)
    lines: list[str] = []

    for start in range(0, len(flat_values), 16):
        chunk = flat_values[start:start + 16]
        lines.append(indent + ", ".join(str(int(value)) for value in chunk))

    return ",\n".join(lines)


def format_c_matrix(values: np.ndarray, indent: str = "    ") -> str:
    if values.ndim != 2:
        raise ValueError("format_c_matrix expects a 2D array")

    rows: list[str] = []
    for row in values:
        rows.append(f"{indent}{{\n{format_c_array(row, indent + '    ')}\n{indent}}}")
    return ",\n".join(rows)


def format_c_nested(values: np.ndarray, indent: str = "    ") -> str:
    if values.ndim == 1:
        return format_c_array(values, indent)

    rows: list[str] = []
    for row in values:
        rows.append(f"{indent}{{\n{format_c_nested(row, indent + '    ')}\n{indent}}}")
    return ",\n".join(rows)


def write_perceptron_c(model: Perceptron, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    weight = model.linear.weight.detach().cpu().numpy()
    bias = model.linear.bias.detach().cpu().numpy()
    weight_q, scale = quantize_weight(weight)
    bias_q = np.round(bias * 255.0 / scale).astype(np.int32)

    (output_dir / "PerceptronData.h").write_text(
        """/**
 * @file PerceptronData.h
 * @brief Quantized perceptron weights exported from tools/train_mnist.py.
 * @author generated
 */
#ifndef PERCEPTRON_DATA_H
#define PERCEPTRON_DATA_H

#include <stdint.h>

#define PERCEPTRON_INPUT_SIZE    784U
#define PERCEPTRON_CLASS_COUNT   10U

extern const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE];
extern const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )

    c_text = f"""/**
 * @file PerceptronData.c
 * @brief Quantized perceptron weights exported from tools/train_mnist.py.
 * @author generated
 */
#include "PerceptronData.h"

const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE] = {{
{format_c_matrix(weight_q)}
}};

const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT] = {{
{format_c_array(bias_q)}
}};
"""
    (output_dir / "PerceptronData.c").write_text(c_text, encoding="utf-8")
    np.savez(MODEL_DIR / "perceptron_quant.npz", weight=weight_q, bias=bias_q, scale=np.array([scale], dtype=np.float32))


def write_fnn_c(model: FNN, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    first_linear = model.layers[1]
    second_linear = model.layers[3]
    assert isinstance(first_linear, nn.Linear)
    assert isinstance(second_linear, nn.Linear)

    weight_1 = first_linear.weight.detach().cpu().numpy()
    bias_1 = first_linear.bias.detach().cpu().numpy()
    weight_2 = second_linear.weight.detach().cpu().numpy()
    bias_2 = second_linear.bias.detach().cpu().numpy()

    weight_1_q, scale_1 = quantize_weight(weight_1)
    weight_2_q, scale_2 = quantize_weight(weight_2)
    bias_1_q = np.round(bias_1 * 255.0 / scale_1).astype(np.int32)
    bias_2_q = np.round(bias_2 * (1 << 8) / scale_2).astype(np.int32)
    hidden_size = weight_1_q.shape[0]

    (output_dir / "FNN_Data.h").write_text(
        f"""/**
 * @file FNN_Data.h
 * @brief Quantized FNN weights exported from tools/train_mnist.py.
 * @author generated
 */
#ifndef FNN_DATA_H
#define FNN_DATA_H

#include <stdint.h>

#define FNN_INPUT_SIZE     784U
#define FNN_HIDDEN_SIZE    {hidden_size}U
#define FNN_CLASS_COUNT    10U
#define FNN_HIDDEN_SHIFT   8U

extern const int8_t g_fnn_weight_1[FNN_HIDDEN_SIZE][FNN_INPUT_SIZE];
extern const int32_t g_fnn_bias_1[FNN_HIDDEN_SIZE];
extern const int8_t g_fnn_weight_2[FNN_CLASS_COUNT][FNN_HIDDEN_SIZE];
extern const int32_t g_fnn_bias_2[FNN_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )

    c_text = f"""/**
 * @file FNN_Data.c
 * @brief Quantized FNN weights exported from tools/train_mnist.py.
 * @author generated
 */
#include "FNN_Data.h"

const int8_t g_fnn_weight_1[FNN_HIDDEN_SIZE][FNN_INPUT_SIZE] = {{
{format_c_matrix(weight_1_q)}
}};

const int32_t g_fnn_bias_1[FNN_HIDDEN_SIZE] = {{
{format_c_array(bias_1_q)}
}};

const int8_t g_fnn_weight_2[FNN_CLASS_COUNT][FNN_HIDDEN_SIZE] = {{
{format_c_matrix(weight_2_q)}
}};

const int32_t g_fnn_bias_2[FNN_CLASS_COUNT] = {{
{format_c_array(bias_2_q)}
}};
"""
    (output_dir / "FNN_Data.c").write_text(c_text, encoding="utf-8")
    np.savez(
        MODEL_DIR / "fnn_quant.npz",
        weight_1=weight_1_q,
        bias_1=bias_1_q,
        weight_2=weight_2_q,
        bias_2=bias_2_q,
        scale_1=np.array([scale_1], dtype=np.float32),
        scale_2=np.array([scale_2], dtype=np.float32),
    )


def write_cnn_c(model: CNN, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    conv1 = model.features[0]
    conv2 = model.features[3]
    fc = model.classifier[1]
    assert isinstance(conv1, nn.Conv2d)
    assert isinstance(conv2, nn.Conv2d)
    assert isinstance(fc, nn.Linear)

    conv1_weight = conv1.weight.detach().cpu().numpy()[:, 0, :, :]
    conv1_bias = conv1.bias.detach().cpu().numpy()
    conv2_weight = conv2.weight.detach().cpu().numpy()
    conv2_bias = conv2.bias.detach().cpu().numpy()
    fc_weight = fc.weight.detach().cpu().numpy()
    fc_bias = fc.bias.detach().cpu().numpy()

    conv1_weight_q, conv1_scale = quantize_weight(conv1_weight)
    conv2_weight_q, conv2_scale = quantize_weight(conv2_weight)
    fc_weight_q, fc_scale = quantize_weight(fc_weight)

    conv1_shift = 8
    conv2_shift = 8
    conv1_feature_scale = 255.0 / (conv1_scale * float(1 << conv1_shift))
    conv2_feature_scale = conv1_feature_scale / (conv2_scale * float(1 << conv2_shift))

    conv1_bias_q = np.round(conv1_bias * 255.0 / conv1_scale).astype(np.int32)
    conv2_bias_q = np.round(conv2_bias * conv1_feature_scale / conv2_scale).astype(np.int32)
    fc_bias_q = np.round(fc_bias * conv2_feature_scale / fc_scale).astype(np.int32)

    (output_dir / "CNN_Data.h").write_text(
        f"""/**
 * @file CNN_Data.h
 * @brief Quantized Tiny-CNN weights exported from tools/train_mnist.py.
 * @author generated
 */
#ifndef CNN_DATA_H
#define CNN_DATA_H

#include <stdint.h>

#define CNN_INPUT_WIDTH            28U
#define CNN_INPUT_HEIGHT           28U
#define CNN_CONV1_OUT_CHANNELS     {conv1_weight_q.shape[0]}U
#define CNN_CONV2_IN_CHANNELS      {conv2_weight_q.shape[1]}U
#define CNN_CONV2_OUT_CHANNELS     {conv2_weight_q.shape[0]}U
#define CNN_KERNEL_SIZE            3U
#define CNN_POOL1_WIDTH            14U
#define CNN_POOL1_HEIGHT           14U
#define CNN_POOL2_WIDTH            7U
#define CNN_POOL2_HEIGHT           7U
#define CNN_FEATURE_SIZE           (CNN_CONV2_OUT_CHANNELS * CNN_POOL2_WIDTH * CNN_POOL2_HEIGHT)
#define CNN_CLASS_COUNT            10U
#define CNN_CONV1_SHIFT            {conv1_shift}U
#define CNN_CONV2_SHIFT            {conv2_shift}U

extern const int8_t g_cnn_conv1_weight[CNN_CONV1_OUT_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_conv1_bias[CNN_CONV1_OUT_CHANNELS];
extern const int8_t g_cnn_conv2_weight[CNN_CONV2_OUT_CHANNELS][CNN_CONV2_IN_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_conv2_bias[CNN_CONV2_OUT_CHANNELS];
extern const int8_t g_cnn_fc_weight[CNN_CLASS_COUNT][CNN_FEATURE_SIZE];
extern const int32_t g_cnn_fc_bias[CNN_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )

    c_text = f"""/**
 * @file CNN_Data.c
 * @brief Quantized Tiny-CNN weights exported from tools/train_mnist.py.
 * @author generated
 */
#include "CNN_Data.h"

const int8_t g_cnn_conv1_weight[CNN_CONV1_OUT_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(conv1_weight_q)}
}};

const int32_t g_cnn_conv1_bias[CNN_CONV1_OUT_CHANNELS] = {{
{format_c_array(conv1_bias_q)}
}};

const int8_t g_cnn_conv2_weight[CNN_CONV2_OUT_CHANNELS][CNN_CONV2_IN_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(conv2_weight_q)}
}};

const int32_t g_cnn_conv2_bias[CNN_CONV2_OUT_CHANNELS] = {{
{format_c_array(conv2_bias_q)}
}};

const int8_t g_cnn_fc_weight[CNN_CLASS_COUNT][CNN_FEATURE_SIZE] = {{
{format_c_matrix(fc_weight_q)}
}};

const int32_t g_cnn_fc_bias[CNN_CLASS_COUNT] = {{
{format_c_array(fc_bias_q)}
}};
"""
    (output_dir / "CNN_Data.c").write_text(c_text, encoding="utf-8")
    np.savez(
        MODEL_DIR / "cnn_quant.npz",
        conv1_weight=conv1_weight_q,
        conv1_bias=conv1_bias_q,
        conv2_weight=conv2_weight_q,
        conv2_bias=conv2_bias_q,
        fc_weight=fc_weight_q,
        fc_bias=fc_bias_q,
        conv1_shift=np.array([conv1_shift], dtype=np.int32),
        conv2_shift=np.array([conv2_shift], dtype=np.int32),
        conv1_scale=np.array([conv1_scale], dtype=np.float32),
        conv2_scale=np.array([conv2_scale], dtype=np.float32),
        fc_scale=np.array([fc_scale], dtype=np.float32),
    )


def sync_generated_for_keil(filenames: Iterable[str]) -> None:
    KEIL_GENERATED_DIR.mkdir(parents=True, exist_ok=True)

    for filename in filenames:
        source = GENERATED_DIR / filename
        target = KEIL_GENERATED_DIR / filename
        if filename.endswith(".c"):
            text = source.read_text(encoding="utf-8")
            text = text.replace('#include "PerceptronData.h"', '#include "digit_nn/generated/PerceptronData.h"')
            text = text.replace('#include "FNN_Data.h"', '#include "digit_nn/generated/FNN_Data.h"')
            text = text.replace('#include "CNN_Data.h"', '#include "digit_nn/generated/CNN_Data.h"')
            target.write_text(text, encoding="utf-8")
        else:
            shutil.copyfile(source, target)


def save_metrics(metrics_path: Path, records: Iterable[dict[str, object]]) -> None:
    metrics_path.write_text(json.dumps(list(records), ensure_ascii=False, indent=2), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model", choices=["perceptron", "fnn", "cnn"], default="perceptron")
    parser.add_argument("--epochs", type=int, default=5)
    parser.add_argument("--batch-size", type=int, default=128)
    parser.add_argument("--lr", type=float, default=1e-3)
    parser.add_argument("--data-dir", type=Path, default=ROOT_DIR / "data")
    parser.add_argument("--out-dir", type=Path, default=MODEL_DIR)
    parser.add_argument("--augment", action="store_true", help="Use light affine augmentation to improve handwritten robustness.")
    parser.add_argument("--export-c", action="store_true", help="Export quantized C arrays for MCU deployment.")
    parser.add_argument("--export-keil", action="store_true", help="Also sync generated C arrays into the Keil touch project.")
    parser.add_argument("--seed", type=int, default=2026)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    torch.manual_seed(args.seed)
    np.random.seed(args.seed)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    train_loader, test_loader = build_loaders(args.data_dir, args.batch_size, args.augment)
    model = build_model(args.model).to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=args.lr)
    records: list[dict[str, object]] = []

    for epoch in range(1, args.epochs + 1):
        loss = train_one_epoch(model, train_loader, optimizer, device)
        accuracy = evaluate(model, test_loader, device)
        record = {"model": args.model, "epoch": epoch, "loss": loss, "accuracy": accuracy}
        records.append(record)
        print(f"epoch={epoch} loss={loss:.4f} accuracy={accuracy:.4%}")

    model_path = args.out_dir / f"{args.model}.pt"
    torch.save(model.state_dict(), model_path)
    save_metrics(args.out_dir / f"{args.model}_metrics.json", records)
    print(f"saved model: {model_path}")

    if args.export_c:
        if args.model == "perceptron":
            write_perceptron_c(model.cpu(), GENERATED_DIR)  # type: ignore[arg-type]
            exported_files = ["PerceptronData.c", "PerceptronData.h"]
        elif args.model == "fnn":
            write_fnn_c(model.cpu(), GENERATED_DIR)  # type: ignore[arg-type]
            exported_files = ["FNN_Data.c", "FNN_Data.h"]
        else:
            write_cnn_c(model.cpu(), GENERATED_DIR)  # type: ignore[arg-type]
            exported_files = ["CNN_Data.c", "CNN_Data.h"]
        if args.export_keil:
            sync_generated_for_keil(exported_files)
            print(f"synced generated files to Keil: {KEIL_GENERATED_DIR}")


if __name__ == "__main__":
    main()
