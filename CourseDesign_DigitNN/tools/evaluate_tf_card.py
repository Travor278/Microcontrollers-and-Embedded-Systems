"""Evaluate all exported models on the prepared TF-card test sets."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from host_batch_test import ROOT_DIR, default_quant_file, run_batch


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tf-card-dir", type=Path, default=ROOT_DIR / "tf_card")
    parser.add_argument("--sets", nargs="+", default=["mnist", "personal", "external_usps"])
    parser.add_argument("--models", nargs="+", choices=["perceptron", "fnn", "cnn"], default=["perceptron", "fnn", "cnn"])
    parser.add_argument("--output-json", type=Path, default=ROOT_DIR / "models" / "tf_card_eval.json")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    rows: list[dict[str, object]] = []

    for set_name in args.sets:
        set_dir = args.tf_card_dir / set_name
        for model in args.models:
            result = run_batch(set_dir, model, default_quant_file(model), verbose=False)
            result["set"] = set_name
            try:
                result["set_dir"] = str(set_dir.relative_to(ROOT_DIR))
            except ValueError:
                result["set_dir"] = str(set_dir)
            rows.append(result)

    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(rows, ensure_ascii=False, indent=2), encoding="utf-8")

    print("| Set | Model | Total | Correct | Accuracy | Avg time (us) |")
    print("| --- | --- | ---: | ---: | ---: | ---: |")
    for row in rows:
        print(
            f"| {row['set']} | {row['model']} | {row['total']} | {row['correct']} | "
            f"{row['accuracy']:.2%} | {row['avg_time_us']:.2f} |"
        )
    print(f"saved: {args.output_json}")


if __name__ == "__main__":
    main()
