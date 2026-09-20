#!/usr/bin/env python3
"""Rank non-matching functions that are cheap to byte-match first.

The full worklist (asm_function_worklist.py) sorts by size, which targets byte
coverage.  This tool targets *throughput*: a function with no `jal` (no callee
to resolve), no `$gp`-relative access (no symbol-profile work) and a small body
is usually a leaf getter/setter/flag-twiddle that matches in one or two tries.

Reads the splat-generated asm under asm/disc1/*.s (git-ignored; run
scripts/split_us.sh first).  Metrics per function:
    size  - bytes from the `nonmatching <name>, <size>` directive
    jal   - count of `jal` / `jalr` instructions (call edges)
    gp    - count of instructions referencing `$gp`
    words - instruction count

Usage:
    python3 tools/analysis/asm_leaf_candidates.py [--max-size N] [--limit N]
"""

from __future__ import annotations

import argparse
import glob
import os
import re
import sys

NONMATCHING_RE = re.compile(r"^nonmatching\s+(\S+),\s*(0x[0-9A-Fa-f]+|\d+)\s*$")
GLABEL_RE = re.compile(r"^glabel\s+(\S+)")
ENDLABEL_RE = re.compile(r"^endlabel\s+(\S+)")
INSN_RE = re.compile(r"^\s*/\*.*?\*/\s+(\S+)\s*(.*)$")


def find_repo_root() -> str:
    path = os.path.abspath(os.path.dirname(__file__))
    while path != "/":
        if os.path.isfile(os.path.join(path, "CLAUDE.md")):
            return path
        path = os.path.dirname(path)
    return os.getcwd()


def collect(asm_dir: str) -> list[dict]:
    rows: list[dict] = []
    for path in sorted(glob.glob(os.path.join(asm_dir, "*.s"))):
        unit = os.path.splitext(os.path.basename(path))[0]
        pending: tuple[str, int] | None = None
        cur: dict | None = None
        with open(path, "r", errors="ignore") as fh:
            for line in fh:
                m = NONMATCHING_RE.match(line)
                if m:
                    pending = (m.group(1), int(m.group(2), 0))
                    continue
                if pending is not None:
                    if line.strip() == "":
                        continue
                    g = GLABEL_RE.match(line)
                    if g and g.group(1) == pending[0]:
                        cur = {"name": pending[0], "size": pending[1], "unit": unit,
                               "jal": 0, "gp": 0, "words": 0}
                        rows.append(cur)
                    pending = None
                    continue
                if cur is None:
                    continue
                if ENDLABEL_RE.match(line):
                    cur = None
                    continue
                insn = INSN_RE.match(line)
                if not insn:
                    continue
                mnemonic, operands = insn.group(1), insn.group(2)
                cur["words"] += 1
                if mnemonic in ("jal", "jalr"):
                    cur["jal"] += 1
                if "$gp" in operands:
                    cur["gp"] += 1
    return rows


def main() -> int:
    root = find_repo_root()
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--asm-dir", default=os.path.join(root, "asm", "disc1"))
    ap.add_argument("--max-size", type=lambda s: int(s, 0), default=0x40)
    ap.add_argument("--limit", type=int, default=60)
    args = ap.parse_args()

    rows = collect(args.asm_dir)
    if not rows:
        print(f"ERROR: no function boundaries under {args.asm_dir}; run scripts/split_us.sh",
              file=sys.stderr)
        return 1

    leaves = [r for r in rows if r["jal"] == 0 and r["gp"] == 0 and r["size"] <= args.max_size]
    leaves.sort(key=lambda r: (r["size"], r["name"]))
    total_bytes = sum(r["size"] for r in leaves)
    print(f"call-free, gp-free functions <= 0x{args.max_size:X}: "
          f"{len(leaves)} / {total_bytes} bytes")
    print(f"(of {len(rows)} non-matching functions total)\n")
    for r in leaves[: args.limit]:
        print(f"  0x{r['size']:04X} {r['size']:4d}B  {r['words']:3d}w  {r['name']:20s} {r['unit']}.s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
