"""Rebuild tf_card/manifest.csv from all label.txt files."""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tf-card-dir", type=Path, default=ROOT_DIR / "tf_card")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    rows = ["set,filename,label"]

    for label_file in sorted(args.tf_card_dir.glob("*/label.txt")):
        set_name = label_file.parent.name
        for line in label_file.read_text(encoding="utf-8").splitlines():
            if not line.strip():
                continue
            filename, label = line.split(",", maxsplit=1)
            rows.append(f"{set_name},{filename.strip()},{label.strip()}")

    manifest = args.tf_card_dir / "manifest.csv"
    manifest.write_text("\n".join(rows) + "\n", encoding="utf-8")
    print(f"wrote {len(rows) - 1} entries to {manifest}")


if __name__ == "__main__":
    main()
