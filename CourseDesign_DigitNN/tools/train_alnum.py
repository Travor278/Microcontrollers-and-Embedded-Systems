"""Train a PC-side 0-9A-Z prototype model from MNIST and EMNIST Letters."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Callable

import numpy as np
import torch
from PIL import ImageOps
from torch import nn
from torch.utils.data import ConcatDataset, DataLoader, Dataset, Subset
from torchvision import datasets, transforms


ROOT_DIR = Path(__file__).resolve().parents[1]
MODEL_DIR = ROOT_DIR / "models"
CLASS_NAMES = [str(item) for item in range(10)] + [chr(ord("A") + item) for item in range(26)]


class RemappedDataset(Dataset):
    def __init__(self, base: Dataset, labels: list[int]) -> None:
        self.base = base
        self.labels = labels

    def __len__(self) -> int:
        return len(self.base)

    def __getitem__(self, index: int) -> tuple[torch.Tensor, int]:
        image, _label = self.base[index]
        return image, self.labels[index]


class AlnumCNN(nn.Module):
    def __init__(self, class_count: int = 36) -> None:
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


def emnist_orientation(image):
    return ImageOps.mirror(image.rotate(-90))


def make_transform(augment: bool, emnist: bool) -> transforms.Compose:
    steps: list[Callable] = []
    if emnist:
        steps.append(transforms.Lambda(emnist_orientation))
    if augment:
        steps.append(transforms.RandomAffine(degrees=12, translate=(0.10, 0.10), scale=(0.85, 1.15), shear=5, fill=0))
    steps.append(transforms.ToTensor())
    return transforms.Compose(steps)


def mapped_targets(raw_targets, mapper: Callable[[int], int]) -> list[int]:
    return [mapper(int(target)) for target in raw_targets]


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


def build_dataset(data_dir: Path, train: bool, augment: bool, max_per_class: int | None, seed: int) -> Dataset:
    mnist = datasets.MNIST(root=data_dir, train=train, download=True, transform=make_transform(augment and train, emnist=False))
    emnist = datasets.EMNIST(root=data_dir, split="letters", train=train, download=True, transform=make_transform(augment and train, emnist=True))

    mnist_labels = mapped_targets(mnist.targets, lambda label: label)
    emnist_labels = mapped_targets(emnist.targets, lambda label: label + 9)

    digit_set = RemappedDataset(mnist, mnist_labels)
    letter_set = RemappedDataset(emnist, emnist_labels)
    merged = ConcatDataset([digit_set, letter_set])
    merged_labels = mnist_labels + emnist_labels
    return balanced_subset(merged, merged_labels, max_per_class, seed)


def build_loaders(args: argparse.Namespace) -> tuple[DataLoader, DataLoader]:
    train_set = build_dataset(args.data_dir, train=True, augment=args.augment, max_per_class=args.max_train_per_class, seed=args.seed)
    test_set = build_dataset(args.data_dir, train=False, augment=False, max_per_class=args.max_test_per_class, seed=args.seed + 1)
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
    by_class_total = np.zeros(len(CLASS_NAMES), dtype=np.int64)
    by_class_correct = np.zeros(len(CLASS_NAMES), dtype=np.int64)
    model.eval()
    for images, labels in loader:
        images = images.to(device)
        labels = labels.to(device)
        prediction = model(images).argmax(dim=1)
        correct_mask = prediction == labels
        correct += int(correct_mask.sum().item())
        total += int(labels.numel())
        for class_id in range(len(CLASS_NAMES)):
            class_mask = labels == class_id
            by_class_total[class_id] += int(class_mask.sum().item())
            by_class_correct[class_id] += int((correct_mask & class_mask).sum().item())

    return {
        "accuracy": correct / total if total else 0.0,
        "total": total,
        "correct": correct,
        "by_class": {
            CLASS_NAMES[index]: {
                "total": int(by_class_total[index]),
                "correct": int(by_class_correct[index]),
                "accuracy": float(by_class_correct[index] / by_class_total[index]) if by_class_total[index] else 0.0,
            }
            for index in range(len(CLASS_NAMES))
        },
    }


def quantize_weight(weight: np.ndarray) -> tuple[np.ndarray, float]:
    max_abs = float(np.max(np.abs(weight)))
    scale = max_abs / 127.0 if max_abs > 0.0 else 1.0
    return np.clip(np.round(weight / scale), -127, 127).astype(np.int8), scale


def export_quantized(model: AlnumCNN, output_path: Path) -> None:
    conv1 = model.features[0]
    conv2 = model.features[3]
    fc = model.classifier[1]
    assert isinstance(conv1, nn.Conv2d)
    assert isinstance(conv2, nn.Conv2d)
    assert isinstance(fc, nn.Linear)

    conv1_weight_q, conv1_scale = quantize_weight(conv1.weight.detach().cpu().numpy()[:, 0, :, :])
    conv2_weight_q, conv2_scale = quantize_weight(conv2.weight.detach().cpu().numpy())
    fc_weight_q, fc_scale = quantize_weight(fc.weight.detach().cpu().numpy())

    np.savez(
        output_path,
        class_names=np.array(CLASS_NAMES),
        conv1_weight=conv1_weight_q,
        conv1_bias=conv1.bias.detach().cpu().numpy().astype(np.float32),
        conv2_weight=conv2_weight_q,
        conv2_bias=conv2.bias.detach().cpu().numpy().astype(np.float32),
        fc_weight=fc_weight_q,
        fc_bias=fc.bias.detach().cpu().numpy().astype(np.float32),
        conv1_scale=np.array([conv1_scale], dtype=np.float32),
        conv2_scale=np.array([conv2_scale], dtype=np.float32),
        fc_scale=np.array([fc_scale], dtype=np.float32),
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--epochs", type=int, default=5)
    parser.add_argument("--batch-size", type=int, default=512)
    parser.add_argument("--lr", type=float, default=1e-3)
    parser.add_argument("--data-dir", type=Path, default=ROOT_DIR / "data")
    parser.add_argument("--out-dir", type=Path, default=MODEL_DIR)
    parser.add_argument("--augment", action="store_true")
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
    model = AlnumCNN(class_count=len(CLASS_NAMES)).to(device)
    optimizer = torch.optim.Adam(model.parameters(), lr=args.lr)
    records: list[dict[str, object]] = []

    for epoch in range(1, args.epochs + 1):
        loss = train_one_epoch(model, train_loader, optimizer, device)
        metrics = evaluate(model, test_loader, device)
        record = {"epoch": epoch, "loss": loss, "accuracy": metrics["accuracy"], "total": metrics["total"], "correct": metrics["correct"]}
        records.append(record)
        print(f"epoch={epoch} loss={loss:.4f} accuracy={metrics['accuracy']:.4%}")

    final_metrics = evaluate(model, test_loader, device)
    torch.save(model.state_dict(), args.out_dir / "alnum_cnn.pt")
    export_quantized(model.cpu(), args.out_dir / "alnum_cnn_quant.npz")
    (args.out_dir / "alnum_classes.json").write_text(json.dumps(CLASS_NAMES, ensure_ascii=False, indent=2), encoding="utf-8")
    (args.out_dir / "alnum_metrics.json").write_text(
        json.dumps({"records": records, "final": final_metrics}, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    print(f"saved: {args.out_dir / 'alnum_cnn.pt'}")
    print(f"saved: {args.out_dir / 'alnum_cnn_quant.npz'}")


if __name__ == "__main__":
    main()
