#!/usr/bin/env python3
"""triage_m2c.py — classify every remaining function as C-decompilable or not.

Runs m2c over the whole size-ranked worklist and records, per function, whether
its body contains MIPS that no C compiler can be made to emit through the
project's era toolchain (handwritten GTE/cop2 moves, `syscall` trampolines,
bare `nop` padding bodies, cache ops, ...). m2c reports those as
`M2C_ERROR(/* unknown instruction: ... */)`.

The output answers a question that matters for the project goal: how much of
the remaining ~600 KB can ever be a matching C leaf, and how much is
handwritten assembly that must stay `asm` in the yaml.

Writes `build/m2c_triage.json` and prints a summary by size band.

Usage:
    python3 tools/analysis/triage_m2c.py [--jobs N] [--limit N]
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/analysis"))

import m2c_leaf  # noqa: E402

UNKNOWN_RE = re.compile(r"M2C_ERROR\(/\* unknown instruction: ([^*]+)\*/\)")
# Instructions that are inherently not reachable from C through cc1/maspsx.
HANDWRITTEN_HINT = re.compile(
    r"\b(ctc2|cop2|lwc2|swc2|syscall|cache|rfe|mtc0|mfc0|cfc2|ctc0|"
    r"lwc0|swc0|tlbr|tlbwi|tlbwr|tlbp|break)\b"
)


def classify(name: str, entry: dict, unit: str, context: Path) -> dict:
    text = m2c_leaf.run_m2c(name, unit, context)
    if text.startswith("/* m2c failed"):
        return {
            "name": name,
            "size": entry["size"],
            "unit": unit,
            "status": "m2c-error",
            "detail": text.strip()[:200],
        }
    unknown = sorted({m.strip() for m in UNKNOWN_RE.findall(text)})
    handwritten = sorted({m for m in unknown if HANDWRITTEN_HINT.search(m)})
    if handwritten:
        status = "handwritten"
    elif unknown:
        status = "unknown-instr"
    else:
        status = "draftable"
    return {
        "name": name,
        "size": entry["size"],
        "unit": unit,
        "status": status,
        "unknown": unknown,
        "handwritten": handwritten,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--out", default=str(ROOT / "build/m2c_triage.json"))
    args = ap.parse_args()

    index = m2c_leaf.build_worklist_index()
    if not index:
        print("no build/asm_worklist.json; run asm_function_worklist.py", file=sys.stderr)
        return 1
    context = m2c_leaf.generate_context()

    items = sorted(index.values(), key=lambda e: -e["size"])
    if args.limit:
        items = items[: args.limit]

    results: list[dict] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {
            pool.submit(classify, e["name"], e, e["unit"], context): e["name"]
            for e in items
        }
        for i, fut in enumerate(concurrent.futures.as_completed(futures), 1):
            results.append(fut.result())
            if i % 100 == 0:
                print(f"  {i}/{len(items)}", file=sys.stderr)

    results.sort(key=lambda r: -r["size"])
    Path(args.out).write_text(json.dumps(results, indent=1))

    bands = [
        ("<64", lambda s: s < 64),
        ("64-255", lambda s: 64 <= s < 256),
        ("256-1023", lambda s: 256 <= s < 1024),
        (">=1024", lambda s: s >= 1024),
    ]
    print(f"{'band':10s} {'funcs':>6s} {'bytes':>9s} | "
          f"{'draftable':>10s} {'handwritten':>12s} {'other':>7s}")
    for label, pred in bands:
        sel = [r for r in results if pred(r["size"])]
        nb = sum(r["size"] for r in sel)
        d = [r for r in sel if r["status"] == "draftable"]
        h = [r for r in sel if r["status"] == "handwritten"]
        o = [r for r in sel if r["status"] not in ("draftable", "handwritten")]
        print(f"{label:10s} {len(sel):6d} {nb:9d} | {len(d):10d} "
              f"{len(h):12d} {len(o):7d}")
    tot_d = sum(r["size"] for r in results if r["status"] == "draftable")
    tot_h = sum(r["size"] for r in results if r["status"] == "handwritten")
    print(f"\nC-draftable: {tot_d} bytes; handwritten: {tot_h} bytes; "
          f"total {sum(r['size'] for r in results)} bytes")
    print(f"wrote {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
