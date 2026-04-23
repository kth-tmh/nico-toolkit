# PyTorch Toy Demos

This directory contains a narrow translator for the bundled NICO toy examples.
It is intentionally limited to the command subset used by:

- `toy-examples/XOR`
- `toy-examples/Figure8`
- `toy-examples/Sinusoids`

The generator parses the original shell construction scripts and emits small
PyTorch demo programs under `toy-examples/pytorch_demo/generated/`.

## Scope

Supported features in this first pass:

- `CreateNet`
- `AddGroup`
- `AddStream`
- `AddUnit` with the toy-example options in use
- `LinkGroup`
- `Rename`
- `Connect` with single delays and delay windows
- implicit bias terms from `AddUnit`

Deliberately out of scope here:

- the full NICO command set
- exact reproduction of historical NICO training behavior
- the speech example outside `toy-examples`

The generated runtimes use a recursive memoized evaluator over `(unit, time)`.
That keeps the implementation small while still covering the delayed-connection
patterns used by the toy examples.

## Usage

Generate all demo scripts:

```bash
python3 toy-examples/pytorch_demo/generate.py
```

This writes:

- `toy-examples/pytorch_demo/generated/xor_demo.py`
- `toy-examples/pytorch_demo/generated/figure8_demo.py`
- `toy-examples/pytorch_demo/generated/sinusoids_demo.py`

Each generated demo expects `torch` to be available and can emit an SVG
comparison of prediction versus target without requiring `matplotlib`.
