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
└── cnn_metrics.json
```

生成方式示例：

```powershell
python tools\train_mnist.py --model perceptron --epochs 5 --export-c
python tools\train_mnist.py --model fnn --epochs 8 --export-c
python tools\train_mnist.py --model cnn --epochs 5
```

`--export-c` 会覆盖 `firmware/generated` 中对应的 C 模型参数文件。
