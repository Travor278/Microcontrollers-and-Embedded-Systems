# Models

训练输出建议放在本目录：

```text
models/
├── perceptron.pt
├── perceptron_quant.npz
├── perceptron_metrics.json
├── fnn.pt
├── fnn_quant.npz
├── fnn_metrics.json
├── cnn.pt
├── cnn_quant.npz
├── cnn_metrics.json
└── tf_card_eval.json
```

生成方式示例：

```powershell
python tools\train_mnist.py --model perceptron --epochs 2 --batch-size 512 --export-c --export-keil
python tools\train_mnist.py --model fnn --epochs 8 --batch-size 512 --augment --export-c --export-keil
python tools\train_mnist.py --model cnn --epochs 5 --batch-size 512 --augment --export-c --export-keil
python tools\evaluate_tf_card.py
```

`--export-c` 会覆盖 `firmware/generated` 中对应的 C 模型参数文件；`--export-keil` 会同步到 `keil_touch_digit_nn/User/digit_nn/generated`。
