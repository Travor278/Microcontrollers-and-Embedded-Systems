"""Rebuild tf_card/manifest.csv from formal TF-card test sets."""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tf-card-dir", type=Path, default=ROOT_DIR / "tf_card")
    parser.add_argument(
        "--include-cache",
        action="store_true",
        help="Also include ui_collected/ capture cache entries in the manifest.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    rows = ["set,filename,label"]

    skip_sets = set() if args.include_cache else {"ui_collected"}
    for label_file in sorted(args.tf_card_dir.glob("*/label.txt")):
        set_name = label_file.parent.name
        if set_name in skip_sets:
            continue
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
