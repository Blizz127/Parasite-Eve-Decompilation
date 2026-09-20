#!/usr/bin/env python3
"""Map a coverage-build hit set onto the retail guest-function boundaries.

The native port's coverage build (CMake option `PE_EXEC_COVERAGE=ON`) compiles
the field runtime with GCC's `-finstrument-functions`, so every entered host
function is recorded by `__cyg_profile_func_enter` and dumped, one host entry
address per line, by `PE_ExecCoverage_Dump`.

This tool is the offline half.  It:

  1. builds the retail guest-function boundary map from the splat plan:
     `configs/USA/disc1.yaml` (which spans are decompiled `c` leaves, and the
     image-offset -> VRAM geometry) plus `asm/disc1/*.s` (`nonmatching <name>,
     <size>` / `glabel`/`endlabel` pairs give every remaining function's VRAM
     range);
  2. resolves each recorded host address to a symbol with `nm` on the exact
     coverage binary, recovers the guest VMA the symbol stands for (port
     functions are named `func_XXXXXXXX`, optionally with a `.cold`/`.constprop`
     suffix), and lands it in the boundary map;
  3. classifies the executed guest functions into decompiled-C leaves (a `c`
     span in the yaml) vs pc_port-only transcriptions (an `asm` function with
     no C leaf), and reports the unresolved loud boundaries the run hit from
     `build/exec_coverage_boundaries.txt`.

Only the standard library is used.  `nm` (binutils) and an asm/disc1 split are
required; the split is produced locally by `bash scripts/split_us.sh`.
"""

from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
LOAD_FILE_START = 0x800
LOAD_VRAM = 0x80010000
DEFAULT_CONFIG = Path("configs/USA/disc1.yaml")
DEFAULT_ASM_DIR = Path("asm/disc1")

YAML_ROW_RE = re.compile(
    r"^\s*-\s*\[(0x[0-9A-Fa-f]+)\s*(?:,\s*([A-Za-z_]+)\s*(?:,\s*([^\]]+))?)?\]"
)
# `nonmatching func_80020F18, 0x13C` and `glabel func_...` lines in the split.
NONMATCHING_RE = re.compile(
    r"^\s*nonmatching\s+([A-Za-z_][A-Za-z0-9_]*)\s*,\s*0x([0-9A-Fa-f]+)\s*$",
    re.MULTILINE,
)
GLABEL_RE = re.compile(
    r"^\s*glabel\s+([A-Za-z_][A-Za-z0-9_]*)\s*$", re.MULTILINE
)
ENDLABEL_RE = re.compile(
    r"^\s*endlabel\s+([A-Za-z_][A-Za-z0-9_]*)\s*$", re.MULTILINE
)
# 8018F018 8018F330 is the VRAM in a disassembly comment, used to size a
# glabel/endlabel pair that has no `nonmatching` size.
VMA_COMMENT_RE = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s")

FUNC_SYMBOL_RE = re.compile(r"^(func_[0-9A-Fa-f]{8})(?:\..*)?$")


class CoverageError(RuntimeError):
    """A coverage input cannot be interpreted."""


@dataclass(frozen=True)
class GuestBoundary:
    vma: int
    name: str
    kind: str  # "c" or "asm"
    size: int

    @property
    def end(self) -> int:
        return self.vma + self.size


@dataclass
class GuestMap:
    by_vma: dict[int, GuestBoundary] = field(default_factory=dict)
    c_count: int = 0
    asm_count: int = 0

    @property
    def total(self) -> int:
        return len(self.by_vma)

    def find(self, vma: int) -> GuestBoundary | None:
        return self.by_vma.get(vma)


def load_guest_map(root: Path, config: Path, asm_dir: Path) -> GuestMap:
    """Build the VRAM -> guest function map from the yaml plan and the split."""
    config_path = config if config.is_absolute() else root / config
    if not config_path.is_file():
        raise CoverageError(f"missing splat config {config_path}")
    text = config_path.read_text(encoding="utf-8")
    rows: list[tuple[int, str, str | None]] = []
    for line in text.splitlines():
        m = YAML_ROW_RE.match(line)
        if not m:
            continue
        offset = int(m.group(1), 16)
        kind = (m.group(2) or "").strip() or "asm"
        name = (m.group(3) or "").strip() or None
        rows.append((offset, kind, name))
    if not rows:
        raise CoverageError(f"no subsegment rows parsed from {config_path}")

    game_map = GuestMap()
    # c spans: one function each; size runs to the next yaml subsegment row.
    for i, (offset, kind, name) in enumerate(rows):
        if kind != "c":
            continue
        if not name:
            raise CoverageError(f"c span at 0x{offset:X} has no function name")
        end_offset = rows[i + 1][0] if i + 1 < len(rows) else offset
        size = max(0, end_offset - offset)
        vma = LOAD_VRAM + (offset - LOAD_FILE_START)
        game_map.by_vma[vma] = GuestBoundary(vma, name, "c", size)
        game_map.c_count += 1

    # asm functions: from the split.  `nonmatching <name>, <size>` is exact;
    # otherwise size a glabel/endlabel pair from the disassembly VMA comments.
    asm_root = asm_dir if asm_dir.is_absolute() else root / asm_dir
    if not asm_root.is_dir():
        raise CoverageError(
            f"missing split directory {asm_root}; run scripts/split_us.sh first"
        )
    asm_count = 0
    for path in sorted(asm_root.glob("*.s")):
        body = path.read_text(encoding="utf-8", errors="replace")
        for m in NONMATCHING_RE.finditer(body):
            name = m.group(1)
            size = int(m.group(2), 16)
            vma = vma_from_name(name)
            if vma is None:
                continue
            if vma in game_map.by_vma:
                continue  # a named c span already owns this address
            game_map.by_vma[vma] = GuestBoundary(vma, name, "asm", size)
            asm_count += 1
        # Fall back to glabel/endlabel pairs for functions without a
        # `nonmatching` size (also covers any future split format).
        for m in GLABEL_RE.finditer(body):
            name = m.group(1)
            vma = vma_from_name(name)
            if vma is None or vma in game_map.by_vma:
                continue
            # find the matching endlabel position and VMA
            end = ENDLABEL_RE.search(body, m.end())
            if not end or end.group(1) != name:
                continue
            # The last instruction VMA before the endlabel, plus one word, is
            # the function end (the disassembly comment carries the VRAM).
            vm = VMA_COMMENT_RE.findall(body[m.end():end.start()])
            if not vm:
                continue
            end_vma = int(vm[-1], 16) + 4
            if end_vma <= vma:
                continue
            game_map.by_vma[vma] = GuestBoundary(vma, name, "asm", end_vma - vma)
            asm_count += 1
    game_map.asm_count = asm_count
    return game_map


def vma_from_name(name: str, prefix: str = "func_") -> int | None:
    if not name.startswith(prefix):
        return None
    hexpart = name[len(prefix):]
    if len(hexpart) < 8 or not re.fullmatch(r"[0-9A-Fa-f]{8}", hexpart[:8]):
        return None
    return int(hexpart[:8], 16)


@dataclass
class SymbolTable:
    addresses: list[int]
    names: list[str]

    def resolve(self, address: int) -> str | None:
        idx = bisect.bisect_right(self.addresses, address) - 1
        if idx < 0:
            return None
        return self.names[idx]


def load_symbols(binary: Path) -> SymbolTable:
    if not binary.is_file():
        raise CoverageError(f"coverage binary not found: {binary}")
    proc = subprocess.run(
        ["nm", "-n", "--defined-only", str(binary)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        raise CoverageError(
            f"nm failed on {binary}: {proc.stderr.decode(errors='replace').strip()}"
        )
    addresses: list[int] = []
    names: list[str] = []
    for line in proc.stdout.decode(errors="replace").splitlines():
        parts = line.split()
        if len(parts) < 3:
            continue
        try:
            address = int(parts[0], 16)
        except ValueError:
            continue
        addresses.append(address)
        names.append(parts[2])
    if not addresses:
        raise CoverageError(f"no symbols found in {binary}")
    return SymbolTable(addresses, names)


def read_hits(path: Path) -> list[int]:
    if not path.is_file():
        raise CoverageError(f"hit file not found: {path}")
    hits: list[int] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        hits.append(int(line, 16))
    return hits


def read_boundary_events(path: Path) -> list[list[int]]:
    if not path.is_file():
        return []
    events: list[list[int]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        events.append([int(tok, 16) for tok in line.split()])
    return events


@dataclass
class RunCoverage:
    label: str
    hits_file: str
    boundaries_file: str | None
    executed_host_functions: int = 0
    executed_guest_functions: int = 0
    decompiled_c: int = 0
    pc_port_only: int = 0
    unmapped_guest_functions: int = 0
    executed_host_helpers: int = 0
    boundary_events: int = 0
    unresolved_boundary_guest_functions: list[str] = field(default_factory=list)
    unresolved_boundary_unattributed_events: int = 0

    @property
    def decompiled_c_share_percent(self) -> float:
        if not self.executed_guest_functions:
            return 0.0
        return round(100.0 * self.decompiled_c / self.executed_guest_functions, 2)

    def to_json(self) -> dict[str, Any]:
        return {
            "label": self.label,
            "hits_file": self.hits_file,
            "boundaries_file": self.boundaries_file,
            "executed_host_functions": self.executed_host_functions,
            "executed_guest_functions": self.executed_guest_functions,
            "decompiled_c": self.decompiled_c,
            "pc_port_only": self.pc_port_only,
            "unmapped_guest_functions": self.unmapped_guest_functions,
            "executed_host_helpers": self.executed_host_helpers,
            "decompiled_c_share_percent": self.decompiled_c_share_percent,
            "unresolved_boundary_events": self.boundary_events,
            "unresolved_boundary_guest_functions":
                self.unresolved_boundary_guest_functions,
            "unresolved_boundary_unattributed_events":
                self.unresolved_boundary_unattributed_events,
        }


def display_path(root: Path, path: Path) -> str:
    try:
        return str(path.resolve().relative_to(root))
    except ValueError:
        return str(path)


def classify_run(
    root: Path,
    label: str,
    hits_path: Path,
    boundaries_path: Path | None,
    symbols: SymbolTable,
    guest_map: GuestMap,
) -> RunCoverage:
    run = RunCoverage(
        label=label,
        hits_file=display_path(root, hits_path),
        boundaries_file=display_path(root, boundaries_path) if boundaries_path else None,
    )
    guest_seen: set[int] = set()
    unmapped_seen: set[int] = set()
    for address in read_hits(hits_path):
        run.executed_host_functions += 1
        name = symbols.resolve(address)
        if name is None:
            run.executed_host_helpers += 1
            continue
        m = FUNC_SYMBOL_RE.match(name)
        if not m:
            run.executed_host_helpers += 1
            continue
        vma = int(m.group(1)[len("func_"):], 16)
        boundary = guest_map.find(vma)
        if boundary is None:
            unmapped_seen.add(vma)
            continue
        if vma in guest_seen:
            continue
        guest_seen.add(vma)
        if boundary.kind == "c":
            run.decompiled_c += 1
        else:
            run.pc_port_only += 1
    run.executed_guest_functions = len(guest_seen)
    run.unmapped_guest_functions = len(unmapped_seen)

    if boundaries_path is not None:
        culprits: set[str] = set()
        for event in read_boundary_events(boundaries_path):
            run.boundary_events += 1
            culprit: str | None = None
            # frames are stored bottom->top; the innermost frame that maps to a
            # guest function is the one that hit the boundary.
            for address in reversed(event):
                name = symbols.resolve(address)
                if name is None:
                    continue
                if FUNC_SYMBOL_RE.match(name):
                    culprit = name
                    break
            if culprit is None:
                run.unresolved_boundary_unattributed_events += 1
            else:
                culprits.add(culprit)
        run.unresolved_boundary_guest_functions = sorted(culprits)
    return run


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_run_spec(spec: str, root: Path) -> tuple[str, Path, Path | None]:
    parts = spec.split(":")
    if len(parts) not in (2, 3):
        raise CoverageError(
            f"--run expects LABEL:HITS[:BOUNDARIES], got {spec!r}"
        )
    label = parts[0]
    hits = Path(parts[1])
    boundaries = Path(parts[2]) if len(parts) == 3 and parts[2] else None
    if not hits.is_absolute():
        hits = root / hits
    if boundaries is not None and not boundaries.is_absolute():
        boundaries = root / boundaries
    return label, hits, boundaries


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=REPO_ROOT)
    parser.add_argument("--binary", type=Path,
                        help="coverage-build executable for nm symbol resolution")
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG)
    parser.add_argument("--asm-dir", type=Path, default=DEFAULT_ASM_DIR)
    parser.add_argument("--run", action="append", default=[],
                        metavar="LABEL:HITS[:BOUNDARIES]",
                        help="a coverage run to classify (repeatable)")
    parser.add_argument("--hits", type=Path,
                        help="single-run convenience form")
    parser.add_argument("--label", default="run")
    parser.add_argument("--boundaries", type=Path)
    parser.add_argument("--primary", default=None,
                        help="label native_metrics should quote (default: first run)")
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--print-json", action="store_true")
    args = parser.parse_args(argv)

    root: Path = args.root.resolve()
    binary = args.binary
    if binary is not None and not binary.is_absolute():
        binary = root / binary
    if binary is None:
        binary = root / "pc_port" / "build-coverage" / "parasite-eve-port"
    try:
        guest_map = load_guest_map(root, args.config, args.asm_dir)
        symbols = load_symbols(binary)
        runs: list[RunCoverage] = []
        if args.hits is not None:
            hits = args.hits if args.hits.is_absolute() else root / args.hits
            bnd = args.boundaries
            if bnd is not None and not bnd.is_absolute():
                bnd = root / bnd
            runs.append(classify_run(root, args.label, hits, bnd, symbols, guest_map))
        for spec in args.run:
            label, hits, bnd = parse_run_spec(spec, root)
            runs.append(classify_run(root, label, hits, bnd, symbols, guest_map))
        if not runs:
            raise CoverageError("no runs given; pass --run or --hits")
    except CoverageError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    primary = args.primary or runs[0].label
    payload = {
        "schema_version": 1,
        "generated_by": "tools/progress/exec_coverage.py",
        "binary": str(binary.relative_to(root)) if str(binary).startswith(str(root)) else str(binary),
        "binary_sha256": sha256_file(binary),
        "guest_boundary_map": {
            "total": guest_map.total,
            "decompiled_c": guest_map.c_count,
            "asm_functions": guest_map.asm_count,
        },
        "primary": primary,
        "runs": {run.label: run.to_json() for run in runs},
    }

    if args.json_out is not None:
        out = args.json_out if args.json_out.is_absolute() else root / args.json_out
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    if args.print_json or args.json_out is None:
        print(json.dumps(payload, indent=2))

    for run in runs:
        print(
            f"{run.label}: executed guest functions={run.executed_guest_functions} "
            f"decompiled_c={run.decompiled_c} ({run.decompiled_c_share_percent}%) "
            f"pc_port_only={run.pc_port_only} "
            f"unmapped={run.unmapped_guest_functions} "
            f"host_helpers={run.executed_host_helpers} "
            f"unresolved_boundary_events={run.boundary_events} "
            f"unresolved_boundary_functions={len(run.unresolved_boundary_guest_functions)}",
            file=sys.stderr,
        )
        if run.unresolved_boundary_guest_functions:
            print(
                "  boundaries: " + ", ".join(run.unresolved_boundary_guest_functions),
                file=sys.stderr,
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
