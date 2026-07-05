"""Build, flash, and refresh model weights for the Keil DigitNN project."""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
PROJECT_PATH = ROOT_DIR / "keil_touch_digit_nn" / "Project" / "RVMDK（uv5）" / "BH-F103.uvprojx"
LOG_DIR = ROOT_DIR / "keil_touch_digit_nn" / "Output"
OUTPUT_AXF = LOG_DIR / "DigitNN_Touch.axf"
BUILD_LOG = LOG_DIR / "DigitNN_Touch.build_log.htm"
USAGE_CACHE_FILE = LOG_DIR / "firmware_usage.json"
GENERATED_DOMAIN_HEADER = ROOT_DIR / "keil_touch_digit_nn" / "User" / "digit_nn" / "generated" / "RecognitionDomain.h"
KEIL_GENERATED_DIR = ROOT_DIR / "keil_touch_digit_nn" / "User" / "digit_nn" / "generated"
FIRMWARE_GENERATED_DIR = ROOT_DIR / "firmware" / "generated"
GENERATED_CACHE_ROOT = ROOT_DIR / "firmware" / "generated_cache"
GENERATED_FILES = (
    "RecognitionDomain.h",
    "PerceptronData.h",
    "PerceptronData.c",
    "FNN_Data.h",
    "FNN_Data.c",
    "CNN_Data.h",
    "CNN_Data.c",
)
DIGIT_MODELS = ("perceptron", "fnn", "cnn")
LETTER_MODELS = ("letter_perceptron", "letter_fnn", "letter_ds_cnn")
PROGRAM_SIZE_RE = re.compile(
    r"Program Size:\s*Code=(\d+)\s*RO-data=(\d+)\s*RW-data=(\d+)\s*ZI-data=(\d+)"
)


def find_uv4(explicit_path: str | None) -> Path | None:
    if explicit_path:
        path = Path(explicit_path).expanduser()
        return path if path.exists() else None

    env_path = os.environ.get("KEIL_UV4")
    if env_path:
        path = Path(env_path).expanduser()
        if path.exists():
            return path

    path_from_env = shutil.which("UV4.exe") or shutil.which("UV4")
    if path_from_env:
        return Path(path_from_env)

    running_uv4 = find_running_uv4()
    if running_uv4 is not None:
        return running_uv4

    candidates = [
        Path("D:/UV4/UV4.exe"),
        Path("C:/Keil_v5/UV4/UV4.exe"),
        Path("C:/Keil/UV4/UV4.exe"),
        Path("D:/Keil_v5/UV4/UV4.exe"),
        Path("D:/Keil/UV4/UV4.exe"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def find_running_uv4() -> Path | None:
    if os.name != "nt":
        return None
    try:
        completed = subprocess.run(
            [
                "powershell",
                "-NoProfile",
                "-Command",
                (
                    "Get-CimInstance Win32_Process -Filter \"Name='UV4.exe'\" | "
                    "Select-Object -First 1 -ExpandProperty ExecutablePath"
                ),
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=2,
        )
    except Exception:
        return None
    path_text = completed.stdout.strip().splitlines()[0] if completed.stdout.strip() else ""
    if not path_text:
        return None
    path = Path(path_text)
    return path if path.exists() else None


def run_command(command: list[str], dry_run: bool) -> int:
    print(" ".join(f'"{item}"' if " " in item else item for item in command))
    if dry_run:
        return 0

    completed = subprocess.run(command, cwd=ROOT_DIR)
    return int(completed.returncode)


def run_keil(uv4: Path, action: str, project: Path, target: str | None, dry_run: bool) -> int:
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_path = LOG_DIR / f"keil_{action}_{timestamp}.log"
    command = [str(uv4), f"-{action}", str(project), "-j0", "-o", str(log_path)]
    if target:
        command.extend(["-t", target])

    exit_code = run_command(command, dry_run)
    if log_path.exists():
        print(log_path)
        print(log_path.read_text(encoding="utf-8", errors="replace"))
    return exit_code


def parse_usage_from_text(text: str) -> dict[str, int] | None:
    match = PROGRAM_SIZE_RE.search(text)
    if not match:
        return None
    code, ro_data, rw_data, zi_data = (int(item) for item in match.groups())
    return {
        "code": code,
        "ro_data": ro_data,
        "rw_data": rw_data,
        "zi_data": zi_data,
        "flash": code + ro_data + rw_data,
        "sram": rw_data + zi_data,
    }


def read_usage_cache() -> dict[str, object]:
    if not USAGE_CACHE_FILE.exists():
        return {"domains": {}}
    try:
        payload = json.loads(USAGE_CACHE_FILE.read_text(encoding="utf-8", errors="replace"))
    except (OSError, json.JSONDecodeError):
        return {"domains": {}}
    if not isinstance(payload, dict):
        return {"domains": {}}
    domains = payload.get("domains")
    if not isinstance(domains, dict):
        payload["domains"] = {}
    return payload


def update_usage_cache(domain: str) -> None:
    if domain not in {"digit", "letter"} or not BUILD_LOG.exists():
        return
    usage = parse_usage_from_text(BUILD_LOG.read_text(encoding="utf-8", errors="ignore"))
    if usage is None:
        return

    LOG_DIR.mkdir(parents=True, exist_ok=True)
    cache = read_usage_cache()
    domains = cache.setdefault("domains", {})
    entry = {
        "domain": domain,
        "updatedAt": datetime.now().isoformat(timespec="seconds"),
        "buildLog": str(BUILD_LOG),
        "mtime": BUILD_LOG.stat().st_mtime,
        **usage,
    }
    domains[domain] = entry
    cache["latest"] = entry
    USAGE_CACHE_FILE.write_text(json.dumps(cache, ensure_ascii=False, indent=2), encoding="utf-8")


def active_generated_domain() -> str | None:
    if not GENERATED_DOMAIN_HEADER.exists():
        return None
    text = GENERATED_DOMAIN_HEADER.read_text(encoding="utf-8", errors="replace")
    match = re.search(r"#define\s+RECOGNITION_DOMAIN\s+RECOGNITION_DOMAIN_(DIGIT|LETTER)", text)
    return match.group(1).lower() if match else None


def format_c_array(values, indent: str = "    ") -> str:
    flat_values = values.reshape(-1)
    lines: list[str] = []
    for start in range(0, len(flat_values), 16):
        chunk = flat_values[start:start + 16]
        lines.append(indent + ", ".join(str(int(value)) for value in chunk))
    return ",\n".join(lines)


def format_c_nested(values, indent: str = "    ") -> str:
    if values.ndim == 1:
        return format_c_array(values, indent)
    rows: list[str] = []
    for row in values:
        rows.append(f"{indent}{{\n{format_c_nested(row, indent + '    ')}\n{indent}}}")
    return ",\n".join(rows)


def write_domain_header(output_dir: Path, domain: str, class_count: int) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    is_letter = domain == "letter"
    output_dir.joinpath("RecognitionDomain.h").write_text(
        f"""/**
 * @file RecognitionDomain.h
 * @brief Active recognition domain for the shared STM32 firmware shell.
 */
#ifndef RECOGNITION_DOMAIN_H
#define RECOGNITION_DOMAIN_H

#define RECOGNITION_DOMAIN_DIGIT   1U
#define RECOGNITION_DOMAIN_LETTER  2U

#define RECOGNITION_DOMAIN         RECOGNITION_DOMAIN_{"LETTER" if is_letter else "DIGIT"}
#define RECOGNIZER_CLASS_COUNT     {class_count}U
#define RECOGNIZER_LABEL_BASE      '{"A" if is_letter else "0"}'
#define RECOGNIZER_DOMAIN_NAME     "{"LetterNN" if is_letter else "DigitNN"}"
#define RECOGNIZER_READY_TEXT      "Ready: draw {"letter" if is_letter else "digit"}"

#endif
""",
        encoding="utf-8",
    )


def write_perceptron_from_npz(npz_path: Path, output_dir: Path, domain: str, class_count: int) -> None:
    import numpy as np

    with np.load(npz_path) as data:
        weight = data["weight"]
        bias = data["bias"]
    output_dir.joinpath("PerceptronData.h").write_text(
        f"""/**
 * @file PerceptronData.h
 * @brief Cached quantized {domain} perceptron weights.
 */
#ifndef PERCEPTRON_DATA_H
#define PERCEPTRON_DATA_H

#include <stdint.h>

#define PERCEPTRON_INPUT_SIZE    784U
#define PERCEPTRON_CLASS_COUNT   {class_count}U

extern const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE];
extern const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )
    output_dir.joinpath("PerceptronData.c").write_text(
        f"""/**
 * @file PerceptronData.c
 * @brief Cached quantized {domain} perceptron weights.
 */
#include "digit_nn/generated/PerceptronData.h"

const int8_t g_perceptron_weights[PERCEPTRON_CLASS_COUNT][PERCEPTRON_INPUT_SIZE] = {{
{format_c_nested(weight)}
}};

const int32_t g_perceptron_bias[PERCEPTRON_CLASS_COUNT] = {{
{format_c_array(bias)}
}};
""",
        encoding="utf-8",
    )


def write_fnn_from_npz(npz_path: Path, output_dir: Path, domain: str, class_count: int) -> None:
    import numpy as np

    with np.load(npz_path) as data:
        weight_1 = data["weight_1"]
        bias_1 = data["bias_1"]
        weight_2 = data["weight_2"]
        bias_2 = data["bias_2"]
    hidden_size = int(weight_1.shape[0])
    output_dir.joinpath("FNN_Data.h").write_text(
        f"""/**
 * @file FNN_Data.h
 * @brief Cached quantized {domain} FNN weights.
 */
#ifndef FNN_DATA_H
#define FNN_DATA_H

#include <stdint.h>

#define FNN_INPUT_SIZE     784U
#define FNN_HIDDEN_SIZE    {hidden_size}U
#define FNN_CLASS_COUNT    {class_count}U
#define FNN_HIDDEN_SHIFT   8U

extern const int8_t g_fnn_weight_1[FNN_HIDDEN_SIZE][FNN_INPUT_SIZE];
extern const int32_t g_fnn_bias_1[FNN_HIDDEN_SIZE];
extern const int8_t g_fnn_weight_2[FNN_CLASS_COUNT][FNN_HIDDEN_SIZE];
extern const int32_t g_fnn_bias_2[FNN_CLASS_COUNT];

#endif
""",
        encoding="utf-8",
    )
    output_dir.joinpath("FNN_Data.c").write_text(
        f"""/**
 * @file FNN_Data.c
 * @brief Cached quantized {domain} FNN weights.
 */
#include "digit_nn/generated/FNN_Data.h"

const int8_t g_fnn_weight_1[FNN_HIDDEN_SIZE][FNN_INPUT_SIZE] = {{
{format_c_nested(weight_1)}
}};

const int32_t g_fnn_bias_1[FNN_HIDDEN_SIZE] = {{
{format_c_array(bias_1)}
}};

const int8_t g_fnn_weight_2[FNN_CLASS_COUNT][FNN_HIDDEN_SIZE] = {{
{format_c_nested(weight_2)}
}};

const int32_t g_fnn_bias_2[FNN_CLASS_COUNT] = {{
{format_c_array(bias_2)}
}};
""",
        encoding="utf-8",
    )


def write_cnn_from_npz(npz_path: Path, output_dir: Path, domain: str, class_count: int) -> None:
    import numpy as np

    with np.load(npz_path) as data:
        conv1_weight = data["conv1_weight"]
        conv1_bias = data["conv1_bias"]
        conv2_weight = data["conv2_weight"]
        conv2_bias = data["conv2_bias"]
        fc_weight = data["fc_weight"]
        fc_bias = data["fc_bias"]
        conv1_shift = int(data["conv1_shift"][0])
        conv2_shift = int(data["conv2_shift"][0])
    output_dir.joinpath("CNN_Data.h").write_text(
        f"""/**
 * @file CNN_Data.h
 * @brief Cached quantized {domain} Tiny-CNN weights.
 */
#ifndef CNN_DATA_H
#define CNN_DATA_H

#include <stdint.h>

#define CNN_MODEL_KIND_STANDARD       0U
#define CNN_MODEL_KIND_DS_CNN         1U
#define CNN_MODEL_KIND                CNN_MODEL_KIND_STANDARD
#define CNN_INPUT_WIDTH            28U
#define CNN_INPUT_HEIGHT           28U
#define CNN_CONV1_OUT_CHANNELS     {int(conv1_weight.shape[0])}U
#define CNN_CONV2_IN_CHANNELS      {int(conv2_weight.shape[1])}U
#define CNN_CONV2_OUT_CHANNELS     {int(conv2_weight.shape[0])}U
#define CNN_KERNEL_SIZE            3U
#define CNN_POOL1_WIDTH            14U
#define CNN_POOL1_HEIGHT           14U
#define CNN_POOL2_WIDTH            7U
#define CNN_POOL2_HEIGHT           7U
#define CNN_FEATURE_SIZE           (CNN_CONV2_OUT_CHANNELS * CNN_POOL2_WIDTH * CNN_POOL2_HEIGHT)
#define CNN_CLASS_COUNT            {class_count}U
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
    output_dir.joinpath("CNN_Data.c").write_text(
        f"""/**
 * @file CNN_Data.c
 * @brief Cached quantized {domain} Tiny-CNN weights.
 */
#include "digit_nn/generated/CNN_Data.h"

const int8_t g_cnn_conv1_weight[CNN_CONV1_OUT_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(conv1_weight)}
}};

const int32_t g_cnn_conv1_bias[CNN_CONV1_OUT_CHANNELS] = {{
{format_c_array(conv1_bias)}
}};

const int8_t g_cnn_conv2_weight[CNN_CONV2_OUT_CHANNELS][CNN_CONV2_IN_CHANNELS][CNN_KERNEL_SIZE][CNN_KERNEL_SIZE] = {{
{format_c_nested(conv2_weight)}
}};

const int32_t g_cnn_conv2_bias[CNN_CONV2_OUT_CHANNELS] = {{
{format_c_array(conv2_bias)}
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


def write_ds_cnn_from_npz(npz_path: Path, output_dir: Path, domain: str, class_count: int) -> None:
    import numpy as np

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

    output_dir.joinpath("CNN_Data.h").write_text(
        f"""/**
 * @file CNN_Data.h
 * @brief Cached quantized {domain} DS-CNN weights.
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
#define CNN_CLASS_COUNT               {class_count}U
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
    output_dir.joinpath("CNN_Data.c").write_text(
        f"""/**
 * @file CNN_Data.c
 * @brief Cached quantized {domain} DS-CNN weights.
 */
#include "digit_nn/generated/CNN_Data.h"

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


def copy_generated(source_dir: Path, target_dir: Path) -> None:
    target_dir.mkdir(parents=True, exist_ok=True)
    for filename in GENERATED_FILES:
        shutil.copyfile(source_dir / filename, target_dir / filename)


def build_cache_from_quant(domain: str) -> None:
    cache_dir = GENERATED_CACHE_ROOT / domain
    cache_dir.mkdir(parents=True, exist_ok=True)
    if domain == "letter":
        class_count = 26
        files = {
            "perceptron": ROOT_DIR / "models" / "letter_perceptron_quant.npz",
            "fnn": ROOT_DIR / "models" / "letter_fnn_quant.npz",
            "cnn": ROOT_DIR / "models" / "letter_ds_cnn_quant.npz",
        }
    else:
        class_count = 10
        files = {
            "perceptron": ROOT_DIR / "models" / "perceptron_quant.npz",
            "fnn": ROOT_DIR / "models" / "fnn_quant.npz",
            "cnn": ROOT_DIR / "models" / "cnn_quant.npz",
        }
    missing = [str(path) for path in files.values() if not path.exists()]
    if missing:
        raise SystemExit("No cached generated files and missing quantized models:\n" + "\n".join(missing))
    write_domain_header(cache_dir, domain, class_count)
    write_perceptron_from_npz(files["perceptron"], cache_dir, domain, class_count)
    write_fnn_from_npz(files["fnn"], cache_dir, domain, class_count)
    if domain == "letter":
        write_ds_cnn_from_npz(files["cnn"], cache_dir, domain, class_count)
    else:
        write_cnn_from_npz(files["cnn"], cache_dir, domain, class_count)


def cache_current_generated(domain: str) -> None:
    cache_dir = GENERATED_CACHE_ROOT / domain
    cache_dir.mkdir(parents=True, exist_ok=True)
    for filename in GENERATED_FILES:
        source = KEIL_GENERATED_DIR / filename
        if source.exists():
            shutil.copyfile(source, cache_dir / filename)


def restore_cached_generated(domain: str) -> None:
    cache_dir = GENERATED_CACHE_ROOT / domain
    if not all((cache_dir / filename).exists() for filename in GENERATED_FILES):
        build_cache_from_quant(domain)
    copy_generated(cache_dir, KEIL_GENERATED_DIR)
    copy_generated(cache_dir, FIRMWARE_GENERATED_DIR)


def prepare_generated_domain(domain: str, dry_run: bool) -> bool:
    if dry_run:
        return False
    active = active_generated_domain()
    if active == domain:
        cache_current_generated(domain)
        return False
    print(f"switching generated firmware domain: {active or 'unknown'} -> {domain}")
    restore_cached_generated(domain)
    return True


def generated_newer_than_axf() -> bool:
    if not OUTPUT_AXF.exists():
        return True
    axf_mtime = OUTPUT_AXF.stat().st_mtime
    for filename in GENERATED_FILES:
        path = KEIL_GENERATED_DIR / filename
        if path.exists() and path.stat().st_mtime > axf_mtime:
            return True
    return False


def ordered_models(selected: str, choices: tuple[str, ...]) -> list[str]:
    if selected in {"", "all"}:
        return list(choices)
    if selected not in choices:
        raise SystemExit(f"model must be one of: all, {', '.join(choices)}")
    return [selected, *[model for model in choices if model != selected]]


def export_one_digit_model(model: str, epochs: int, batch_size: int, augment: bool, dry_run: bool) -> int:
    command = [
        sys.executable,
        str(ROOT_DIR / "tools" / "train_mnist.py"),
        "--model",
        model,
        "--epochs",
        str(epochs),
        "--batch-size",
        str(batch_size),
        "--export-c",
        "--export-keil",
    ]
    if augment:
        command.append("--augment")
    return run_command(command, dry_run)


def export_one_letter_model(model: str, epochs: int, batch_size: int, augment: bool, dry_run: bool) -> int:
    command = [
        sys.executable,
        str(ROOT_DIR / "tools" / "train_letters.py"),
        "--model",
        model,
        "--epochs",
        str(epochs),
        "--batch-size",
        str(batch_size),
        "--export-c",
        "--export-keil",
    ]
    if augment:
        command.append("--augment")
    return run_command(command, dry_run)


def export_model(domain: str, model: str, epochs: int, batch_size: int, augment: bool, dry_run: bool) -> int:
    if domain == "digit":
        models = ordered_models(model, DIGIT_MODELS)
        print(f"exporting digit firmware domain models: {', '.join(models)}")
        for item in models:
            exit_code = export_one_digit_model(item, epochs, batch_size, augment, dry_run)
            if exit_code != 0:
                return exit_code
        return 0
    if domain == "letter":
        models = ordered_models(model, LETTER_MODELS)
        print(f"exporting letter firmware domain models: {', '.join(models)}")
        for item in models:
            exit_code = export_one_letter_model(item, epochs, batch_size, augment, dry_run)
            if exit_code != 0:
                return exit_code
        return 0
    raise SystemExit("domain must be digit or letter")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--action",
        choices=["build", "rebuild", "flash", "build-flash", "export-model", "export-build-flash"],
        default="build",
    )
    parser.add_argument("--domain", choices=["digit", "letter"], default="digit")
    parser.add_argument("--model", default="all")
    parser.add_argument("--epochs", type=int, default=3)
    parser.add_argument("--batch-size", type=int, default=512)
    parser.add_argument("--augment", action="store_true")
    parser.add_argument("--uv4", default=None, help="Path to UV4.exe. Also supports KEIL_UV4 env var.")
    parser.add_argument("--project", type=Path, default=PROJECT_PATH)
    parser.add_argument("--target", default=None, help="Optional Keil target name.")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if not args.project.exists():
        raise SystemExit(f"Keil project not found: {args.project}")

    exported_models = False
    if args.action in {"export-model", "export-build-flash"}:
        exit_code = export_model(args.domain, args.model, args.epochs, args.batch_size, args.augment, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)
        exported_models = True
        if not args.dry_run:
            cache_current_generated(args.domain)

    if args.action == "export-model":
        return

    domain_switched = prepare_generated_domain(args.domain, args.dry_run)
    if exported_models:
        domain_switched = False

    uv4 = find_uv4(args.uv4)
    if uv4 is None:
        if args.dry_run:
            uv4 = Path("UV4.exe")
        else:
            raise SystemExit("UV4.exe not found. Set KEIL_UV4 or pass --uv4 C:\\Keil_v5\\UV4\\UV4.exe")

    needs_flash_build = (
        args.action == "flash"
        and not args.dry_run
        and (domain_switched or generated_newer_than_axf())
    )

    if args.action in {"build", "build-flash", "export-build-flash"} or needs_flash_build:
        exit_code = run_keil(uv4, "b", args.project, args.target, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)
        if not args.dry_run:
            update_usage_cache(args.domain)
    elif args.action == "rebuild":
        exit_code = run_keil(uv4, "r", args.project, args.target, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)
        if not args.dry_run:
            update_usage_cache(args.domain)

    if args.action in {"flash", "build-flash", "export-build-flash"}:
        exit_code = run_keil(uv4, "f", args.project, args.target, args.dry_run)
        if exit_code != 0:
            raise SystemExit(exit_code)


if __name__ == "__main__":
    main()
