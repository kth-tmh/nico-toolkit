#!/usr/bin/env python3
"""Generate narrow PyTorch demos from the toy NICO shell scripts.

This is intentionally incomplete. It only supports the command subset used by:
  - toy-examples/XOR
  - toy-examples/Figure8
  - toy-examples/Sinusoids
"""

from __future__ import annotations

import argparse
import json
import os
import pathlib
import re
import shlex
from dataclasses import dataclass, field


ROOT = pathlib.Path(__file__).resolve().parents[2]
TOY_ROOT = ROOT / "toy-examples"
OUT_DIR = TOY_ROOT / "pytorch_demo" / "generated"


VAR_RE = re.compile(r"\$([A-Za-z_][A-Za-z0-9_]*)")


@dataclass
class StreamSpec:
    name: str
    size: int
    mode: str
    ext: str = ""
    directory: str = "."


@dataclass
class UnitSpec:
    name: str
    role: str
    activation: str
    bias_bound: float = 0.1
    stream_name: str | None = None
    stream_pos: int | None = None


@dataclass
class ExampleSpec:
    example_name: str
    script_path: str
    net_name: str | None = None
    groups: dict[str, list[str]] = field(default_factory=dict)
    units: list[UnitSpec] = field(default_factory=list)
    streams: dict[str, StreamSpec] = field(default_factory=dict)
    connections: list[dict[str, object]] = field(default_factory=list)
    notes: list[str] = field(default_factory=list)
    default_basenames: list[str] = field(default_factory=list)

    def add_group(self, name: str) -> None:
        self.groups.setdefault(name, [])

    def add_unit(self, group: str, unit: UnitSpec) -> None:
        self.groups.setdefault(group, []).append(unit.name)
        self.units.append(unit)

    def rename_group_member(self, selector: str, new_name: str) -> None:
        group_name, _, index_text = selector.partition("#")
        if not group_name or not index_text:
            raise ValueError(f"Unsupported rename selector: {selector}")
        index = int(index_text) - 1
        group_members = self.groups[group_name]
        old_name = group_members[index]
        group_members[index] = new_name
        for unit in self.units:
            if unit.name == old_name:
                unit.name = new_name
                return
        raise ValueError(f"Unit {old_name} not found for rename")

    def get_group_units(self, name: str) -> list[str]:
        return list(self.groups[name])


def expand_vars(text: str, variables: dict[str, str]) -> str:
    def replace(match: re.Match[str]) -> str:
        return variables.get(match.group(1), match.group(0))

    return VAR_RE.sub(replace, text)


def parse_add_stream(tokens: list[str], spec: ExampleSpec) -> None:
    ext = ""
    directory = "."
    pos = 1
    while pos < len(tokens) - 4:
        flag = tokens[pos]
        if flag == "-x":
            ext = tokens[pos + 1]
            pos += 2
        elif flag == "-d":
            directory = tokens[pos + 1]
            pos += 2
        elif flag.startswith("-d") and flag != "-d":
            directory = flag[2:]
            pos += 1
        elif flag == "-F":
            pos += 2
        elif flag.startswith("-F") and flag != "-F":
            pos += 1
        elif flag == "-S":
            pos += 2
        else:
            raise ValueError(f"Unsupported AddStream option: {flag}")

    size = int(tokens[pos])
    mode = tokens[pos + 1]
    name = tokens[pos + 2]
    spec.streams[name] = StreamSpec(name=name, size=size, mode=mode, ext=ext, directory=directory)


def parse_add_unit(tokens: list[str], spec: ExampleSpec) -> None:
    role = "hidden"
    activation = "tanh"
    count = 1
    bias_bound = 0.1
    pos = 1
    while pos < len(tokens) - 2:
        flag = tokens[pos]
        if flag == "-i":
            role = "input"
            pos += 1
        elif flag == "-o":
            role = "output"
            pos += 1
        elif flag == "-u":
            count = int(tokens[pos + 1])
            pos += 2
        elif flag == "-l":
            activation = "linear"
            pos += 1
        elif flag == "-t":
            activation = "arctan"
            pos += 1
        elif flag == "-s":
            activation = "sigmoid"
            pos += 1
        elif flag == "-r":
            bias_bound = float(tokens[pos + 1])
            pos += 2
        else:
            raise ValueError(f"Unsupported AddUnit option: {flag}")

    group_name = tokens[-2]
    spec.add_group(group_name)
    for index in range(count):
        unit_name = f"{group_name}_{index + 1}"
        spec.add_unit(
            group_name,
            UnitSpec(
                name=unit_name,
                role=role,
                activation=activation,
                bias_bound=bias_bound,
            ),
        )


def connect_range(low: int, high: int) -> list[int]:
    step = 1 if high >= low else -1
    return list(range(low, high + step, step))


def parse_connect(tokens: list[str], spec: ExampleSpec) -> None:
    low = high = 0
    weight_bound = 0.1
    const_weight: float | None = None
    pos = 1
    while pos < len(tokens) - 3:
        flag = tokens[pos]
        if flag == "-D":
            low = int(tokens[pos + 1])
            high = int(tokens[pos + 2])
            pos += 3
        elif flag == "-d":
            low = high = int(tokens[pos + 1])
            pos += 2
        elif flag == "-r":
            weight_bound = float(tokens[pos + 1])
            pos += 2
        elif flag == "-w":
            const_weight = float(tokens[pos + 1])
            pos += 2
        else:
            raise ValueError(f"Unsupported Connect option: {flag}")

    from_name = tokens[-3]
    to_name = tokens[-2]
    from_units = spec.get_group_units(from_name)
    to_units = spec.get_group_units(to_name)
    for delay in connect_range(low, high):
        for src in from_units:
            for dst in to_units:
                spec.connections.append(
                    {
                        "from": src,
                        "to": dst,
                        "delay": delay,
                        "bound": weight_bound,
                        "const_weight": const_weight,
                    }
                )


def parse_link_group(tokens: list[str], spec: ExampleSpec) -> None:
    stream_name = tokens[1]
    group_name = tokens[2]
    stream = spec.streams[stream_name]
    group_units = spec.get_group_units(group_name)
    if len(group_units) != stream.size:
        raise ValueError(f"LinkGroup size mismatch: {group_name} -> {stream_name}")
    for position, unit_name in enumerate(group_units):
        for unit in spec.units:
            if unit.name == unit_name:
                unit.stream_name = stream_name
                unit.stream_pos = position
                break


def parse_script(script_path: pathlib.Path) -> ExampleSpec:
    example_name = script_path.parent.name.lower()
    spec = ExampleSpec(example_name=example_name, script_path=str(script_path.relative_to(ROOT)))
    variables: dict[str, str] = {}

    for raw_line in script_path.read_text().splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" in line and not line.startswith(("CreateNet", "Add", "Connect", "Link", "Rename", "Norm", "BackProp", "Excite")):
            name, value = line.split("=", 1)
            variables[name.strip()] = value.strip()
            continue

        expanded = expand_vars(line, variables)
        tokens = shlex.split(expanded)
        if not tokens:
            continue
        command = tokens[0]

        if command == "CreateNet":
            spec.net_name = tokens[1]
        elif command == "AddGroup":
            spec.add_group(tokens[1])
        elif command == "AddUnit":
            parse_add_unit(tokens, spec)
        elif command == "AddStream":
            parse_add_stream(tokens, spec)
        elif command == "LinkGroup":
            parse_link_group(tokens, spec)
        elif command == "Connect":
            parse_connect(tokens, spec)
        elif command == "Rename":
            spec.rename_group_member(tokens[1], tokens[2])
        elif command == "NormStream":
            spec.notes.append(expanded)
        elif command in {"Display", "BackProp", "Excite", "cat"}:
            spec.notes.append(expanded)
        else:
            raise ValueError(f"Unsupported command in {script_path}: {command}")

    if example_name == "xor":
        spec.default_basenames = ["xor"]
    elif example_name == "figure8":
        spec.default_basenames = ["fig8"]
    elif example_name == "sinusoids":
        trainfiles = script_path.parent / "trainfiles"
        if trainfiles.exists():
            spec.default_basenames = [line.strip().removesuffix(".data") for line in trainfiles.read_text().splitlines() if line.strip()]
    return spec


PYTORCH_TEMPLATE = """#!/usr/bin/env python3
from __future__ import annotations

import argparse
import math
import pathlib
from typing import Dict, List

import torch


SPEC = {spec_json}
ROOT = pathlib.Path(__file__).resolve().parents[3]
EXAMPLE_DIR = ROOT / "toy-examples" / SPEC["example_dir"]


def read_ascii_matrix(path: pathlib.Path, width: int) -> torch.Tensor:
    rows: List[List[float]] = []
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if not line:
            continue
        values = [float(part) for part in line.split()]
        if len(values) != width:
            raise ValueError(f"Expected {{width}} columns in {{path}}, got {{len(values)}}")
        rows.append(values)
    return torch.tensor(rows, dtype=torch.float32)


def stream_file(base_name: str, stream_name: str) -> pathlib.Path:
    stream = SPEC["streams"][stream_name]
    directory = EXAMPLE_DIR / stream["directory"]
    return directory / f"{{base_name}}.{{stream['ext']}}"


def load_example(base_name: str) -> tuple[Dict[str, torch.Tensor], Dict[str, torch.Tensor]]:
    inputs: Dict[str, torch.Tensor] = {{}}
    targets: Dict[str, torch.Tensor] = {{}}
    for name, stream in SPEC["streams"].items():
        tensor = read_ascii_matrix(stream_file(base_name, name), stream["size"])
        if stream["mode"] == "r":
            inputs[name] = tensor
        else:
            targets[name] = tensor
    return inputs, targets


def activation(kind: str, value: torch.Tensor) -> torch.Tensor:
    if kind == "linear":
        return value
    if kind == "tanh":
        return torch.tanh(value)
    if kind == "sigmoid":
        return torch.sigmoid(value)
    if kind == "arctan":
        return torch.atan(value)
    raise ValueError(f"Unsupported activation: {{kind}}")


class ToyNICOModel(torch.nn.Module):
    def __init__(self) -> None:
        super().__init__()
        self.num_units = len(SPEC["units"])
        self.unit_names = [unit["name"] for unit in SPEC["units"]]
        self.activations = [unit["activation"] for unit in SPEC["units"]]
        self.roles = [unit["role"] for unit in SPEC["units"]]
        self.unit_index = {{name: index for index, name in enumerate(self.unit_names)}}
        self.edge_from = [self.unit_index[edge["from"]] for edge in SPEC["connections"]]
        self.edge_to = [self.unit_index[edge["to"]] for edge in SPEC["connections"]]
        self.edge_delay = [int(edge["delay"]) for edge in SPEC["connections"]]
        self.incoming = {{index: [] for index in range(self.num_units)}}
        for edge_index, unit_index in enumerate(self.edge_to):
            self.incoming[unit_index].append(edge_index)
        self.output_links = {{
            name: [self.unit_index[unit_name] for unit_name in stream["linked_units"]]
            for name, stream in SPEC["streams"].items()
            if stream["mode"] != "r"
        }}
        self.input_links = {{
            name: [self.unit_index[unit_name] for unit_name in stream["linked_units"]]
            for name, stream in SPEC["streams"].items()
            if stream["mode"] == "r"
        }}
        self.edge_weight = torch.nn.Parameter(torch.empty(len(self.edge_from), dtype=torch.float32))
        self.bias = torch.nn.Parameter(torch.zeros(self.num_units, dtype=torch.float32))
        self.reset_parameters()

    def reset_parameters(self) -> None:
        with torch.no_grad():
            for index, edge in enumerate(SPEC["connections"]):
                const_weight = edge["const_weight"]
                if const_weight is not None:
                    self.edge_weight[index] = float(const_weight)
                else:
                    bound = float(edge["bound"])
                    self.edge_weight[index].uniform_(-bound, bound)
            self.bias.zero_()
            for index, unit in enumerate(SPEC["units"]):
                if unit["role"] == "input":
                    continue
                bound = float(unit["bias_bound"])
                if bound > 0.0:
                    self.bias[index].uniform_(-bound, bound)

    def forward(self, inputs: Dict[str, torch.Tensor]) -> Dict[str, torch.Tensor]:
        first_stream = next(iter(inputs.values()))
        steps = int(first_stream.shape[0])
        cache: Dict[tuple[int, int], torch.Tensor] = {{}}
        input_positions = {{}}
        for stream_name, unit_indexes in self.input_links.items():
            for offset, unit_index in enumerate(unit_indexes):
                input_positions[unit_index] = (stream_name, offset)

        def unit_value(unit_index: int, step: int) -> torch.Tensor:
            if step < 0 or step >= steps:
                return self.edge_weight.new_zeros(())
            key = (unit_index, step)
            if key in cache:
                return cache[key]
            if self.roles[unit_index] == "input":
                stream_name, offset = input_positions[unit_index]
                value = inputs[stream_name][step][offset]
                cache[key] = value
                return value
            total = self.bias[unit_index]
            for edge_index in self.incoming[unit_index]:
                source_index = self.edge_from[edge_index]
                source_step = step - self.edge_delay[edge_index]
                total = total + self.edge_weight[edge_index] * unit_value(source_index, source_step)
            value = activation(self.activations[unit_index], total)
            cache[key] = value
            return value

        return {{
            stream_name: torch.stack(
                [
                    torch.stack([unit_value(unit_index, step) for unit_index in unit_indexes], dim=0)
                    for step in range(steps)
                ],
                dim=0,
            )
            for stream_name, unit_indexes in self.output_links.items()
        }}


def loss_for_outputs(outputs: Dict[str, torch.Tensor], targets: Dict[str, torch.Tensor]) -> torch.Tensor:
    total = torch.tensor(0.0, dtype=torch.float32)
    for stream_name, target in targets.items():
        total = total + torch.nn.functional.mse_loss(outputs[stream_name], target)
    return total


def train_model(model: ToyNICOModel, basenames: List[str], epochs: int, lr: float) -> None:
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    for epoch in range(epochs):
        epoch_loss = 0.0
        for base_name in basenames:
            inputs, targets = load_example(base_name)
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = loss_for_outputs(outputs, targets)
            loss.backward()
            optimizer.step()
            epoch_loss += float(loss.detach())
        if epoch % max(1, epochs // 10) == 0 or epoch == epochs - 1:
            print(f"epoch={{epoch:4d}} loss={{epoch_loss:.6f}}")


def polyline(points: List[tuple[float, float]], color: str, width: float) -> str:
    joined = " ".join(f"{{x:.2f}},{{y:.2f}}" for x, y in points)
    return f'<polyline fill="none" stroke="{{color}}" stroke-width="{{width}}" points="{{joined}}" />'


def scale_xy(values: torch.Tensor, width: int, height: int, padding: int) -> List[tuple[float, float]]:
    xs = values[:, 0]
    ys = values[:, 1]
    min_x = float(xs.min())
    max_x = float(xs.max())
    min_y = float(ys.min())
    max_y = float(ys.max())
    span_x = max(max_x - min_x, 1e-6)
    span_y = max(max_y - min_y, 1e-6)
    points: List[tuple[float, float]] = []
    for x_value, y_value in zip(xs.tolist(), ys.tolist()):
        x = padding + (x_value - min_x) / span_x * (width - 2 * padding)
        y = height - padding - (y_value - min_y) / span_y * (height - 2 * padding)
        points.append((x, y))
    return points


def scale_series(values: torch.Tensor, width: int, height: int, padding: int, offset: float) -> List[tuple[float, float]]:
    min_y = float(values.min())
    max_y = float(values.max())
    span_y = max(max_y - min_y, 1e-6)
    steps = max(1, values.shape[0] - 1)
    points: List[tuple[float, float]] = []
    for index, value in enumerate(values.tolist()):
        x = padding + index / steps * (width - 2 * padding)
        y = height - padding - (value - min_y) / span_y * (height - 2 * padding) + offset
        points.append((x, y))
    return points


def save_svg(outputs: Dict[str, torch.Tensor], targets: Dict[str, torch.Tensor], path: pathlib.Path) -> None:
    stream_name = next(iter(targets.keys()))
    prediction = outputs[stream_name].detach().cpu()
    target = targets[stream_name].detach().cpu()
    width = 900
    height = 420
    padding = 40
    lines: List[str] = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{{width}}" height="{{height}}" viewBox="0 0 {{width}} {{height}}">',
        '<rect width="100%" height="100%" fill="#fffaf2" />',
        '<text x="40" y="28" font-family="Helvetica, Arial, sans-serif" font-size="20">NICO toy-example prediction vs target</text>',
        '<text x="40" y="50" font-family="Helvetica, Arial, sans-serif" font-size="12" fill="#555">orange = target, blue = prediction</text>',
    ]
    if target.shape[1] == 2:
        lines.append(polyline(scale_xy(target, width, height, padding), "#d97706", 3.0))
        lines.append(polyline(scale_xy(prediction, width, height, padding), "#2563eb", 2.0))
    else:
        colors = ["#d97706", "#059669", "#7c3aed", "#dc2626"]
        for index in range(target.shape[1]):
            lines.append(polyline(scale_series(target[:, index], width, height, padding, 0.0), colors[index % len(colors)], 3.0))
            lines.append(polyline(scale_series(prediction[:, index], width, height, padding, 0.0), "#2563eb", 1.5))
    lines.append("</svg>")
    path.write_text("\\n".join(lines))


def report_outputs(model: ToyNICOModel, basenames: List[str], svg_path: str | None) -> None:
    for base_name in basenames:
        inputs, targets = load_example(base_name)
        outputs = model(inputs)
        print(f"example={{base_name}}")
        for stream_name, values in outputs.items():
            print(stream_name, values.detach())
        if svg_path:
            output_path = pathlib.Path(svg_path)
            if len(basenames) > 1:
                output_path = output_path.with_name(f"{{output_path.stem}}_{{base_name}}{{output_path.suffix}}")
            save_svg(outputs, targets, output_path)
            print(f"wrote {{output_path}}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Run a generated PyTorch demo for a NICO toy example.")
    parser.add_argument("--epochs", type=int, default={default_epochs}, help="training epochs")
    parser.add_argument("--lr", type=float, default={default_lr}, help="optimizer learning rate")
    parser.add_argument("--seed", type=int, default=0, help="random seed")
    parser.add_argument("--skip-train", action="store_true", help="skip training and only run inference")
    parser.add_argument("--basename", action="append", default=[], help="example basename to load")
    parser.add_argument("--svg", default="{default_svg}", help="SVG output path")
    args = parser.parse_args()

    torch.manual_seed(args.seed)
    basenames = args.basename or SPEC["default_basenames"]
    model = ToyNICOModel()
    if not args.skip_train:
        train_model(model, basenames, epochs=args.epochs, lr=args.lr)
    report_outputs(model, basenames, svg_path=args.svg)


if __name__ == "__main__":
    main()
"""


def to_emit_dict(spec: ExampleSpec) -> dict[str, object]:
    unit_lookup = {unit.name: unit for unit in spec.units}
    streams: dict[str, dict[str, object]] = {}
    for stream_name, stream in spec.streams.items():
        linked_units = [unit.name for unit in spec.units if unit.stream_name == stream_name]
        streams[stream_name] = {
            "name": stream.name,
            "size": stream.size,
            "mode": stream.mode,
            "ext": stream.ext,
            "directory": stream.directory,
            "linked_units": linked_units,
        }

    units = [
        {
            "name": unit.name,
            "role": unit.role,
            "activation": unit.activation,
            "bias_bound": unit.bias_bound,
        }
        for unit in spec.units
    ]

    return {
        "example_name": spec.example_name,
        "example_dir": pathlib.Path(spec.script_path).parent.name,
        "script_path": spec.script_path,
        "default_basenames": spec.default_basenames,
        "streams": streams,
        "units": units,
        "connections": spec.connections,
        "notes": spec.notes,
    }


def generate_script(spec: ExampleSpec) -> str:
    defaults = {
        "xor": {"epochs": 1500, "lr": 0.05, "svg": "xor.svg"},
        "figure8": {"epochs": 600, "lr": 0.01, "svg": "figure8.svg"},
        "sinusoids": {"epochs": 800, "lr": 0.01, "svg": "sinusoids.svg"},
    }[spec.example_name]
    spec_json = json.dumps(to_emit_dict(spec), indent=2, sort_keys=True)
    return PYTORCH_TEMPLATE.format(
        spec_json=spec_json,
        default_epochs=defaults["epochs"],
        default_lr=defaults["lr"],
        default_svg=defaults["svg"],
    )


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate narrow PyTorch demos from the toy NICO examples.")
    parser.add_argument(
        "--example",
        action="append",
        choices=["xor", "figure8", "sinusoids"],
        help="only generate a specific example",
    )
    args = parser.parse_args()

    examples = {
        "xor": TOY_ROOT / "XOR" / "mk_xor_net",
        "figure8": TOY_ROOT / "Figure8" / "mknet",
        "sinusoids": TOY_ROOT / "Sinusoids" / "mknet",
    }

    requested = args.example or ["xor", "figure8", "sinusoids"]
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for name in requested:
        spec = parse_script(examples[name])
        output_path = OUT_DIR / f"{name}_demo.py"
        output_path.write_text(generate_script(spec))
        os.chmod(output_path, 0o755)
        print(f"wrote {output_path.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
