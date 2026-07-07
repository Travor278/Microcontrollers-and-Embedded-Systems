"""Train EMNIST Letters prototype models for the letter-recognition workspace."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path
from typing import Callable, Iterable

import numpy as np
import torch
from PIL import Image, ImageOps
from torch import nn
from torch.utils.data import DataLoader, Dataset, Subset
from torchvision import datasets, transforms


ROOT_DIR = Path(__file__).resolve().parents[1]
MODEL_DIR = ROOT_DIR / "models"
GENERATED_DIR = ROOT_DIR / "firmware" / "generated"
KEIL_GENERATED_DIR = ROOT_DIR / "keil_touch_digit_nn" / "User" / "digit_nn" / "generated"
CLASS_NAMES = [chr(ord("A") + index) for index in range(26)]


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


def write_domain_header(output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "RecognitionDomain.h").write_text(
        """/**
 * @file RecognitionDomain.h
 * @brief Active recognition domain for the shared STM32 firmware shell.
 */
#ifndef RECOGNITION_DOMAIN_H
#define RECOGNITION_DOMAIN_H

#define RECOGNITION_DOMAIN_DIGIT   1U
#define RECOGNITION_DOMAIN_LETTER  2U

#define RECOGNITION_DOMAIN         RECOGNITION_DOMAIN_LETTER
#define RECOGNIZER_CLASS_COUNT     26U
#define RECOGNIZER_LABEL_BASE      'A'
#define RECOGNIZER_DOMAIN_NAME     "LetterNN"
#define RECOGNIZER_READY_TEXT      "Ready: draw letter"

#endif
""",
        encoding="utf-8",
    )


class LetterDataset(Dataset):
    def __init__(self, base: Dataset) -> None:
        self.base = base

    def __len__(self) -> int:
        return len(self.base)

    def __getitem__(self, index: int) -> tuple[torch.Tensor, int]:
        image, label = self.base[index]
        return image, int(label) - 1


class LetterPerceptron(nn.Module):
    def __init__(self, class_count: int = 26) -> None:
        super().__init__()
        self.linear = nn.Linear(28 * 28, class_count)

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.linear(images.view(images.size(0), -1))


class LetterFNN(nn.Module):
    def __init__(self, hidden_size: int = 96, class_count: int = 26) -> None:
        super().__init__()
        self.layers = nn.Sequential(
            nn.Flatten(),
            nn.Linear(28 * 28, hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, class_count),
        )

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.layers(images)


class LetterCNN(nn.Module):
    """Tiny-CNN with the same layer pattern as the STM32 integer kernel."""

    def __init__(self, class_count: int = 26) -> None:
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(1, 8, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
            nn.Conv2d(8, 16, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
        )
        self.classifier = nn.Sequential(
            nn.Flatten(),
            nn.Linear(16 * 7 * 7, class_count),
        )

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.classifier(self.features(images))


class DepthwiseBlock(nn.Module):
    def __init__(self, in_channels: int, out_channels: int) -> None:
        super().__init__()
        self.layers = nn.Sequential(
            nn.Conv2d(in_channels, in_channels, kernel_size=3, padding=1, groups=in_channels),
            nn.ReLU(),
            nn.Conv2d(in_channels, out_channels, kernel_size=1),
            nn.ReLU(),
        )

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.layers(images)


class LetterDSCNN(nn.Module):
    """Depthwise-separable CNN: a stronger third option that still has MCU-friendly structure."""

    def __init__(self, class_count: int = 26) -> None:
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(1, 12, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.MaxPool2d(2),
            DepthwiseBlock(12, 24),
            nn.MaxPool2d(2),
            DepthwiseBlock(24, 32),
        )
        self.classifier = nn.Sequential(
            nn.Flatten(),
            nn.Linear(32 * 7 * 7, class_count),
        )

    def forward(self, images: torch.Tensor) -> torch.Tensor:
        return self.classifier(self.features(images))


def emnist_orientation(image: Image.Image) -> Image.Image:
    return ImageOps.mirror(image.rotate(-90))


def make_transform(augment: bool) -> transforms.Compose:
    steps: list[Callable] = [transforms.Lambda(emnist_orientation)]
    if augment:
        steps.append(transforms.RandomAffine(degrees=10, translate=(0.08, 0.08), scale=(0.88, 1.12), shear=4, fill=0))
    steps.append(transforms.ToTensor())
    return transforms.Compose(steps)


def remapped_targets(raw_targets) -> list[int]:
    return [int(label) - 1 for label in raw_targets]


def balanced_subset(dataset: Dataset, labels: list[int], max_per_class: int | None, seed: int) -> Dataset:
    if max_per_class is None:
        return dataset

    rng = np.random.default_rng(seed)
    selected: list[int] = []
    for class_id in range(len(CLASS_NAMES)):
        indices = [index for index, label in enumerate(labels) if label == class_id]
        rng.shuffle(indices)
        selected.extend(indices[:max_per_class])
    selected.sort()
    return Subset(dataset, selected)


def build_model(model_name: str) -> nn.Module:
    if model_name == "letter_perceptron":
        return LetterPerceptron()
    if model_name == "letter_fnn":
        return LetterFNN()
    if model_name == "letter_cnn":
        return LetterCNN()
    if model_name == "letter_ds_cnn":
        return LetterDSCNN()
    raise ValueError(f"unsupported model: {model_name}")


def build_loaders(args: argparse.Namespace) -> tuple[DataLoader, DataLoader]:
    train_base = datasets.EMNIST(root=args.data_dir, split="letters", train=True, download=True, transform=make_transform(args.augment))
    test_base = datasets.EMNIST(root=args.data_dir, split="letters", train=False, download=True, transform=make_transform(False))
    train_set = balanced_subset(LetterDataset(train_base), remapped_targets(train_base.targets), args.max_train_per_class, args.seed)
    test_set = balanced_subset(LetterDataset(test_base), remapped_targets(test_base.targets), args.max_test_per_class, args.seed + 1)
    return (
        DataLoader(train_set, batch_size=args.batch_size, shuffle=True, num_workers=0),
        DataLoader(test_set, batch_size=args.batch_size, shuffle=False, num_workers=0),
    )


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
    return total_loss / max(len(loader.dataset), 1)


@torch.no_grad()
def evaluate(model: nn.Module, loader: DataLoader, device: torch.device) -> dict[str, object]:
    correct = 0
    total = 0
    confusion = np.zeros((len(CLASS_NAMES), len(CLASS_NAMES)), dtype=np.int64)
    model.eval()
    for images, labels in loader:
        images = images.to(device)
        labels = labels.to(device)
        prediction = model(images).argmax(dim=1)
        correct += int((prediction == labels).sum().item())
        total += int(labels.numel())
        for truth, pred in zip(labels.cpu().numpy(), prediction.cpu().numpy()):
            confusion[int(truth), int(pred)] += 1

    pairs = []
    for truth in range(len(CLASS_NAMES)):
        for pred in range(len(CLASS_NAMES)):
            if truth != pred and confusion[truth, pred]:
                pairs.append(
                    {
                        "truth": CLASS_NAMES[truth],
                        "prediction": CLASS_NAMES[pred],
                        "count": int(confusion[truth, pred]),
                    }
                )
    pairs.sort(key=lambda item: (-int(item["count"]), str(item["truth"]), str(item["prediction"])))
    return {
        "accuracy": correct / total if total else 0.0,
        "total": total,
        "correct": correct,
        "confusions": pairs[:12],
    }


def quantize_weight(weight: np.ndarray) -> tuple[np.ndarray, float]:
    max_abs = float(np.max(np.abs(weight)))
    scale = max_abs / 127.0 if max_abs > 0.0 else 1.0
    return np.clip(np.round(weight / scale), -127, 127).astype(np.int8), scale


def write_perceptron_c(model: LetterPerceptron, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    weight = model.linear.weight.detach().cpu().numpy()
    bias = model.linear.bias.detach().cpu().numpy()
    weight_q, scale = quantize_weight(weight)
    bias_q = np.round(bias * 255.0 / scale).astype(np.int32)

    (output_dir / "PerceptronData.h").write_text(
        """/**
 * @file PerceptronData.h
 * @brief Quantized letter perceptron weights exported from tools/train_letters.py.
 */
#ifndef PERCEPTRON_DATA_H
#define PERCEPTRON_DATA_H

#include <stdint.h>

#define PERCEPTRON_INPUT_SIZE    784U
#define PERCEPTRON_CLASS_COUNT   26U

extern const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE];
extern const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )
    (output_dir / "PerceptronData.c").write_text(
        f"""/**
 * @file PerceptronData.c
 * @brief Quantized letter perceptron weights exported from tools/train_letters.py.
 */
#include "PerceptronData.h"

const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE] = {{
{format_c_matrix(weight_q)}
}};

const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT] = {{
{format_c_array(bias_q)}
}};
""",
        encoding="utf-8",
    )
    np.savez(
        MODEL_DIR / "letter_perceptron_quant.npz",
        weight=weight_q,
        bias=bias_q,
        scale=np.array([scale], dtype=np.float32),
        class_names=np.array(CLASS_NAMES),
    )


def write_fnn_c(model: LetterFNN, output_dir: Path) -> None:
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
 * @brief Quantized letter FNN weights exported from tools/train_letters.py.
 */
#ifndef FNN_DATA_H
#define FNN_DATA_H

#include <stdint.h>

#define FNN_INPUT_SIZE     784U
#define FNN_HIDDEN_SIZE    {hidden_size}U
#define FNN_CLASS_COUNT    26U
#define FNN_HIDDEN_SHIFT   8U

extern const int8_t g_fnn_weight_1[FNN_HIDDEN_SIZE][FNN_INPUT_SIZE];
extern const int32_t g_fnn_bias_1[FNN_HIDDEN_SIZE];
extern const int8_t g_fnn_weight_2[FNN_CLASS_COUNT][FNN_HIDDEN_SIZE];
extern const int32_t g_fnn_bias_2[FNN_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )
    (output_dir / "FNN_Data.c").write_text(
        f"""/**
 * @file FNN_Data.c
 * @brief Quantized letter FNN weights exported from tools/train_letters.py.
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
""",
        encoding="utf-8",
    )
    np.savez(
        MODEL_DIR / "letter_fnn_quant.npz",
        weight_1=weight_1_q,
        bias_1=bias_1_q,
        weight_2=weight_2_q,
        bias_2=bias_2_q,
        scale_1=np.array([scale_1], dtype=np.float32),
        scale_2=np.array([scale_2], dtype=np.float32),
        class_names=np.array(CLASS_NAMES),
    )


def write_cnn_c(model: LetterCNN, output_dir: Path) -> None:
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
 * @brief Quantized letter Tiny-CNN weights exported from tools/train_letters.py.
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
#define CNN_CLASS_COUNT            26U
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
    (output_dir / "CNN_Data.c").write_text(
        f"""/**
 * @file CNN_Data.c
 * @brief Quantized letter Tiny-CNN weights exported from tools/train_letters.py.
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
""",
        encoding="utf-8",
    )
    np.savez(
        MODEL_DIR / "letter_cnn_quant.npz",
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
        class_names=np.array(CLASS_NAMES),
    )


def ds_cnn_quant_arrays(model: LetterDSCNN) -> dict[str, np.ndarray]:
    conv1 = model.features[0]
    block1 = model.features[3]
    block2 = model.features[5]
    fc = model.classifier[1]
    assert isinstance(conv1, nn.Conv2d)
    assert isinstance(block1, DepthwiseBlock)
    assert isinstance(block2, DepthwiseBlock)
    assert isinstance(fc, nn.Linear)

    ds1_dw = block1.layers[0]
    ds1_pw = block1.layers[2]
    ds2_dw = block2.layers[0]
    ds2_pw = block2.layers[2]
    assert isinstance(ds1_dw, nn.Conv2d)
    assert isinstance(ds1_pw, nn.Conv2d)
    assert isinstance(ds2_dw, nn.Conv2d)
    assert isinstance(ds2_pw, nn.Conv2d)

    shift = 8

    conv1_weight_q, conv1_scale = quantize_weight(conv1.weight.detach().cpu().numpy()[:, 0, :, :])
    ds1_dw_weight_q, ds1_dw_scale = quantize_weight(ds1_dw.weight.detach().cpu().numpy()[:, 0, :, :])
    ds1_pw_weight_q, ds1_pw_scale = quantize_weight(ds1_pw.weight.detach().cpu().numpy()[:, :, 0, 0])
    ds2_dw_weight_q, ds2_dw_scale = quantize_weight(ds2_dw.weight.detach().cpu().numpy()[:, 0, :, :])
    ds2_pw_weight_q, ds2_pw_scale = quantize_weight(ds2_pw.weight.detach().cpu().numpy()[:, :, 0, 0])
    fc_weight_q, fc_scale = quantize_weight(fc.weight.detach().cpu().numpy())

    conv1_feature_scale = 255.0 / (conv1_scale * float(1 << shift))
    ds1_dw_feature_scale = conv1_feature_scale / (ds1_dw_scale * float(1 << shift))
    ds1_pw_feature_scale = ds1_dw_feature_scale / (ds1_pw_scale * float(1 << shift))
    ds2_dw_feature_scale = ds1_pw_feature_scale / (ds2_dw_scale * float(1 << shift))
    ds2_pw_feature_scale = ds2_dw_feature_scale / (ds2_pw_scale * float(1 << shift))

    return {
        "model_name": np.array(["letter_ds_cnn"]),
        "class_names": np.array(CLASS_NAMES),
        "conv1_weight": conv1_weight_q,
        "conv1_bias": np.round(conv1.bias.detach().cpu().numpy() * 255.0 / conv1_scale).astype(np.int32),
        "conv1_shift": np.array([shift], dtype=np.int32),
        "ds1_dw_weight": ds1_dw_weight_q,
        "ds1_dw_bias": np.round(ds1_dw.bias.detach().cpu().numpy() * conv1_feature_scale / ds1_dw_scale).astype(np.int32),
        "ds1_dw_shift": np.array([shift], dtype=np.int32),
        "ds1_pw_weight": ds1_pw_weight_q,
        "ds1_pw_bias": np.round(ds1_pw.bias.detach().cpu().numpy() * ds1_dw_feature_scale / ds1_pw_scale).astype(np.int32),
        "ds1_pw_shift": np.array([shift], dtype=np.int32),
        "ds2_dw_weight": ds2_dw_weight_q,
        "ds2_dw_bias": np.round(ds2_dw.bias.detach().cpu().numpy() * ds1_pw_feature_scale / ds2_dw_scale).astype(np.int32),
        "ds2_dw_shift": np.array([shift], dtype=np.int32),
        "ds2_pw_weight": ds2_pw_weight_q,
        "ds2_pw_bias": np.round(ds2_pw.bias.detach().cpu().numpy() * ds2_dw_feature_scale / ds2_pw_scale).astype(np.int32),
        "ds2_pw_shift": np.array([shift], dtype=np.int32),
        "fc_weight": fc_weight_q,
        "fc_bias": np.round(fc.bias.detach().cpu().numpy() * ds2_pw_feature_scale / fc_scale).astype(np.int32),
        "conv1_scale": np.array([conv1_scale], dtype=np.float32),
        "ds1_dw_scale": np.array([ds1_dw_scale], dtype=np.float32),
        "ds1_pw_scale": np.array([ds1_pw_scale], dtype=np.float32),
        "ds2_dw_scale": np.array([ds2_dw_scale], dtype=np.float32),
        "ds2_pw_scale": np.array([ds2_pw_scale], dtype=np.float32),
        "fc_scale": np.array([fc_scale], dtype=np.float32),
    }


def export_ds_cnn_quantized(model: LetterDSCNN, output_path: Path) -> None:
    np.savez(output_path, **ds_cnn_quant_arrays(model))


def write_ds_cnn_c_from_npz(npz_path: Path, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    with np.load(npz_path) as data:
        conv1_weight = data["conv1_weight"]
        conv1_bias = data["conv1_bias"]
        ds1_dw_weight = data["ds1_dw_weight"]
        ds1_dw_bias = data["ds1_dw_bias"]
        ds1_pw_weight = data["ds1_pw_weight"]
        ds1_pw_bias = data["ds1_pw_bias"]
        ds2_dw_weight = data["ds2_dw_weight"]
        ds2_dw_bias = data["ds2_dw_bias"]
        ds2_pw_weight = data["ds2_pw_weight"]
        ds2_pw_bias = data["ds2_pw_bias"]
        fc_weight = data["fc_weight"]
        fc_bias = data["fc_bias"]
        conv1_shift = int(data["conv1_shift"][0])
        ds1_dw_shift = int(data["ds1_dw_shift"][0])
        ds1_pw_shift = int(data["ds1_pw_shift"][0])
        ds2_dw_shift = int(data["ds2_dw_shift"][0])
        ds2_pw_shift = int(data["ds2_pw_shift"][0])

    (output_dir / "CNN_Data.h").write_text(
        f"""/**
 * @file CNN_Data.h
 * @brief Quantized letter DS-CNN weights exported from tools/train_letters.py.
 */
#ifndef CNN_DATA_H
#define CNN_DATA_H

#include <stdint.h>

#define CNN_MODEL_KIND_STANDARD       0U
#define CNN_MODEL_KIND_DS_CNN         1U
#define CNN_MODEL_KIND                CNN_MODEL_KIND_DS_CNN
#define CNN_INPUT_WIDTH               28U
#define CNN_INPUT_HEIGHT              28U
#define CNN_KERNEL_SIZE               3U
#define CNN_POOL1_WIDTH               14U
#define CNN_POOL1_HEIGHT              14U
#define CNN_POOL2_WIDTH               7U
#define CNN_POOL2_HEIGHT              7U
#define CNN_CONV1_OUT_CHANNELS        {int(conv1_weight.shape[0])}U
#define CNN_DS1_DW_CHANNELS           {int(ds1_dw_weight.shape[0])}U
#define CNN_DS1_PW_OUT_CHANNELS       {int(ds1_pw_weight.shape[0])}U
#define CNN_DS2_DW_CHANNELS           {int(ds2_dw_weight.shape[0])}U
#define CNN_DS2_PW_OUT_CHANNELS       {int(ds2_pw_weight.shape[0])}U
#define CNN_CONV2_IN_CHANNELS         CNN_DS1_PW_OUT_CHANNELS
#define CNN_CONV2_OUT_CHANNELS        CNN_DS2_PW_OUT_CHANNELS
#define CNN_FEATURE_SIZE              (CNN_DS2_PW_OUT_CHANNELS * CNN_POOL2_WIDTH * CNN_POOL2_HEIGHT)
#define CNN_CLASS_COUNT               26U
#define CNN_CONV1_SHIFT               {conv1_shift}U
#define CNN_DS1_DW_SHIFT              {ds1_dw_shift}U
#define CNN_DS1_PW_SHIFT              {ds1_pw_shift}U
#define CNN_DS2_DW_SHIFT              {ds2_dw_shift}U
#define CNN_DS2_PW_SHIFT              {ds2_pw_shift}U
#define CNN_CONV2_SHIFT               CNN_DS2_PW_SHIFT

extern const int8_t g_cnn_conv1_weight[CNN_CONV1_OUT_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_conv1_bias[CNN_CONV1_OUT_CHANNELS];
extern const int8_t g_cnn_ds1_depthwise_weight[CNN_DS1_DW_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_ds1_depthwise_bias[CNN_DS1_DW_CHANNELS];
extern const int8_t g_cnn_ds1_pointwise_weight[CNN_DS1_PW_OUT_CHANNELS][CNN_DS1_DW_CHANNELS];
extern const int32_t g_cnn_ds1_pointwise_bias[CNN_DS1_PW_OUT_CHANNELS];
extern const int8_t g_cnn_ds2_depthwise_weight[CNN_DS2_DW_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE];
extern const int32_t g_cnn_ds2_depthwise_bias[CNN_DS2_DW_CHANNELS];
extern const int8_t g_cnn_ds2_pointwise_weight[CNN_DS2_PW_OUT_CHANNELS][CNN_DS2_DW_CHANNELS];
extern const int32_t g_cnn_ds2_pointwise_bias[CNN_DS2_PW_OUT_CHANNELS];
extern const int8_t g_cnn_fc_weight[CNN_CLASS_COUNT][CNN_FEATURE_SIZE];
extern const int32_t g_cnn_fc_bias[CNN_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )
    (output_dir / "CNN_Data.c").write_text(
        f"""/**
 * @file CNN_Data.c
 * @brief Quantized letter DS-CNN weights exported from tools/train_letters.py.
 */
#include "CNN_Data.h"

const int8_t g_cnn_conv1_weight[CNN_CONV1_OUT_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(conv1_weight)}
}};

const int32_t g_cnn_conv1_bias[CNN_CONV1_OUT_CHANNELS] = {{
{format_c_array(conv1_bias)}
}};

const int8_t g_cnn_ds1_depthwise_weight[CNN_DS1_DW_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(ds1_dw_weight)}
}};

const int32_t g_cnn_ds1_depthwise_bias[CNN_DS1_DW_CHANNELS] = {{
{format_c_array(ds1_dw_bias)}
}};

const int8_t g_cnn_ds1_pointwise_weight[CNN_DS1_PW_OUT_CHANNELS][CNN_DS1_DW_CHANNELS] = {{
{format_c_nested(ds1_pw_weight)}
}};

const int32_t g_cnn_ds1_pointwise_bias[CNN_DS1_PW_OUT_CHANNELS] = {{
{format_c_array(ds1_pw_bias)}
}};

const int8_t g_cnn_ds2_depthwise_weight[CNN_DS2_DW_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(ds2_dw_weight)}
}};

const int32_t g_cnn_ds2_depthwise_bias[CNN_DS2_DW_CHANNELS] = {{
{format_c_array(ds2_dw_bias)}
}};

const int8_t g_cnn_ds2_pointwise_weight[CNN_DS2_PW_OUT_CHANNELS][CNN_DS2_DW_CHANNELS] = {{
{format_c_nested(ds2_pw_weight)}
}};

const int32_t g_cnn_ds2_pointwise_bias[CNN_DS2_PW_OUT_CHANNELS] = {{
{format_c_array(ds2_pw_bias)}
}};

const int8_t g_cnn_fc_weight[CNN_CLASS_COUNT][CNN_FEATURE_SIZE] = {{
{format_c_nested(fc_weight)}
}};

const int32_t g_cnn_fc_bias[CNN_CLASS_COUNT] = {{
{format_c_array(fc_bias)}
}};
""",
        encoding="utf-8",
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


def export_quantized(model: nn.Module, model_name: str, output_path: Path) -> None:
    arrays: dict[str, np.ndarray] = {"class_names": np.array(CLASS_NAMES)}
    state = model.state_dict()
    for name, tensor in state.items():
        values = tensor.detach().cpu().numpy()
        key = name.replace(".", "_")
        if values.ndim >= 2:
            quantized, scale = quantize_weight(values)
            arrays[f"{key}_q"] = quantized
            arrays[f"{key}_scale"] = np.array([scale], dtype=np.float32)
        else:
            arrays[key] = values.astype(np.float32)
    arrays["model_name"] = np.array([model_name])
    np.savez(output_path, **arrays)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--model",
        choices=["letter_perceptron", "letter_fnn", "letter_cnn", "letter_ds_cnn"],
        default="letter_fnn",
    )
    parser.add_argument("--epochs", type=int, default=5)
    parser.add_argument("--batch-size", type=int, default=512)
    parser.add_argument("--lr", type=float, default=1e-3)
    parser.add_argument("--data-dir", type=Path, default=ROOT_DIR / "data")
    parser.add_argument("--out-dir", type=Path, default=MODEL_DIR)
    parser.add_argument("--augment", action="store_true")
    parser.add_argument("--export-c", action="store_true", help="Export quantized C arrays for MCU deployment.")
    parser.add_argument("--export-keil", action="store_true", help="Also sync generated C arrays into the Keil touch project.")
    parser.add_argument("--max-train-per-class", type=int, default=None, help="Optional quick prototype limit.")
    parser.add_argument("--max-test-per-class", type=int, default=None, help="Optional quick evaluation limit.")
    parser.add_argument("--seed", type=int, default=2026)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    torch.manual_seed(args.seed)
    np.random.seed(args.seed)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    train_loader, test_loader = build_loaders(args)
    model = build_model(args.model).to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=args.lr)
    records: list[dict[str, object]] = []

    for epoch in range(1, args.epochs + 1):
        loss = train_one_epoch(model, train_loader, optimizer, device)
        metrics = evaluate(model, test_loader, device)
        record = {"model": args.model, "epoch": epoch, "loss": loss, **metrics}
        records.append(record)
        print(f"epoch={epoch} loss={loss:.4f} accuracy={metrics['accuracy']:.4%}")

    model_cpu = model.cpu()
    model_path = args.out_dir / f"{args.model}.pt"
    quant_path = args.out_dir / f"{args.model}_quant.npz"
    metrics_path = args.out_dir / f"{args.model}_metrics.json"
    torch.save(model.state_dict(), model_path)
    if args.model == "letter_ds_cnn":
        export_ds_cnn_quantized(model_cpu, quant_path)  # type: ignore[arg-type]
    else:
        export_quantized(model_cpu, args.model, quant_path)
    metrics_path.write_text(json.dumps(records, ensure_ascii=False, indent=2), encoding="utf-8")
    (args.out_dir / "letter_classes.json").write_text(json.dumps(CLASS_NAMES, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"saved: {model_path}")
    print(f"saved: {quant_path}")
    print(f"saved: {metrics_path}")

    if args.export_c:
        write_domain_header(GENERATED_DIR)
        if args.model == "letter_perceptron":
            write_perceptron_c(model.cpu(), GENERATED_DIR)  # type: ignore[arg-type]
            exported_files = ["RecognitionDomain.h", "PerceptronData.c", "PerceptronData.h"]
        elif args.model == "letter_fnn":
            write_fnn_c(model.cpu(), GENERATED_DIR)  # type: ignore[arg-type]
            exported_files = ["RecognitionDomain.h", "FNN_Data.c", "FNN_Data.h"]
        elif args.model == "letter_cnn":
            write_cnn_c(model_cpu, GENERATED_DIR)  # type: ignore[arg-type]
            exported_files = ["RecognitionDomain.h", "CNN_Data.c", "CNN_Data.h"]
        elif args.model == "letter_ds_cnn":
            write_ds_cnn_c_from_npz(quant_path, GENERATED_DIR)
            exported_files = ["RecognitionDomain.h", "CNN_Data.c", "CNN_Data.h"]
        else:
            raise SystemExit("unsupported letter model export")
        if args.export_keil:
            sync_generated_for_keil(exported_files)
            print(f"synced generated files to Keil: {KEIL_GENERATED_DIR}")


if __name__ == "__main__":
    main()
