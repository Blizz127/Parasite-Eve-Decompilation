#!/usr/bin/env python3
"""Rank the remaining non-matching functions that ALREADY have a port body.

`pc_port/` contains ~579 hand-written `*_port.c` files that transcribe retail
behaviour so the native port can run. Many of those bodies are a behavioural
re-implementation of a function that is still `nonmatching` in the retail
build. For a decompilation agent those are the cheapest remaining leaves: the
semantics are already understood and written down, so the work is reduced to
re-expressing them in era-cc1 C that reproduces the retail instruction stream.

This tool joins `asm_function_worklist.py`'s size-ranked queue (which needs a
splat split first: `scripts/split_us.sh`) against every function definition
found under `pc_port/`, and reports the intersection ranked by retail size.

Usage:
    python3 tools/analysis/port_backed_worklist.py [--top N]
        [--worklist build/asm_worklist.json] [--json PATH] [--txt PATH]

Exit status: 0 on success, 1 if the worklist JSON is missing (run
`asm_function_worklist.py` first).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

DEF_RE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(func_[0-9A-Fa-f]{8})\s*\(", re.M
)


def strip_comments(text: str) -> str:
    """Mirror gen_decomp_ports.collect_defined_symbols: comments hide braces."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def collect_port_definitions(port_dir: Path) -> dict[str, str]:
    """Map guest function name -> first pc_port file that defines it."""
    defs: dict[str, str] = {}
    for dirpath, _dirnames, filenames in os.walk(port_dir):
        for name in filenames:
            if not name.endswith((".c", ".h")):
                continue
            path = Path(dirpath) / name
            try:
                text = strip_comments(path.read_text(errors="ignore"))
            except OSError:
                continue
            for match in DEF_RE.finditer(text):
                defs.setdefault(match.group(1), str(path.relative_to(ROOT)))
    return defs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--top", type=int, default=30)
    ap.add_argument("--worklist", default=str(ROOT / "build/asm_worklist.json"))
    ap.add_argument("--port-dir", default=str(ROOT / "pc_port"))
    ap.add_argument("--json", default=str(ROOT / "build/port_backed_worklist.json"))
    ap.add_argument("--txt", default=str(ROOT / "build/port_backed_names.txt"))
    args = ap.parse_args()

    worklist_path = Path(args.worklist)
    if not worklist_path.is_file():
        print(
            f"missing {worklist_path}; run scripts/split_us.sh then "
            "tools/analysis/asm_function_worklist.py",
            file=sys.stderr,
        )
        return 1

    worklist = json.loads(worklist_path.read_text())["worklist"]
    defs = collect_port_definitions(Path(args.port_dir))

    rows = [
        {
            "name": f["name"],
            "size": f["size"],
            "unit": f["unit"],
            "port_file": defs[f["name"]],
        }
        for f in worklist
        if f["name"] in defs
    ]
    rows.sort(key=lambda r: -r["size"])

    total_bytes = sum(r["size"] for r in rows)
    grand_total = sum(f["size"] for f in worklist)
    print(
        f"remaining nonmatching functions: {len(worklist)} "
        f"({grand_total} bytes)"
    )
    print(
        f"of those, already have a pc_port body: {len(rows)} "
        f"({total_bytes} bytes, {total_bytes / grand_total * 100:.1f}%)"
    )
    print("\nTop {0} by size:".format(args.top))
    for r in rows[: args.top]:
        print(f"  {r['size']:6d}  {r['name']}  {r['unit']:8s} {r['port_file']}")

    json_path = Path(args.json)
    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(
        json.dumps(
            {"count": len(rows), "bytes": total_bytes, "entries": rows}, indent=1
        )
    )
    Path(args.txt).write_text(
        "".join(
            f"{r['name']} {r['size']} {r['unit']} {r['port_file']}\n" for r in rows
        )
    )
    print(f"\nwrote {json_path} and {args.txt}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
