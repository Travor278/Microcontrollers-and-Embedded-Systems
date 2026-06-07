"""Train MNIST models and export quantized C arrays for STM32."""

from __future__ import annotations

import argparse
import json
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
    """Compact CNN for PC/Linux-side edge-collaboration experiments."""

    def __init__(self) -> None:
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(1, 16, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
            nn.Conv2d(16, 32, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
        )
        self.classifier = nn.Sequential(
            nn.Flatten(),
            nn.Linear(32 * 7 * 7, 64),
            nn.ReLU(),
            nn.Linear(64, 10),
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


def build_loaders(data_dir: Path, batch_size: int) -> tuple[DataLoader, DataLoader]:
    transform = transforms.Compose([transforms.ToTensor()])
    train_set = datasets.MNIST(root=data_dir, train=True, download=True, transform=transform)
    test_set = datasets.MNIST(root=data_dir, train=False, download=True, transform=transform)
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
    parser.add_argument("--export-c", action="store_true", help="Export quantized C arrays for MCU deployment.")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    train_loader, test_loader = build_loaders(args.data_dir, args.batch_size)
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
        elif args.model == "fnn":
            write_fnn_c(model.cpu(), GENERATED_DIR)  # type: ignore[arg-type]
        else:
            print("CNN C export is intentionally skipped; use it for Linux-side edge collaboration.")


if __name__ == "__main__":
    main()
