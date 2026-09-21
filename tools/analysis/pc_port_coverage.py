#!/usr/bin/env python3
"""pc_port coverage — map every matched `c` span to its pc_port counterpart.

This is the inventory / gap-analysis half of "port what we have decompiled
over".  It answers, from authoritative inputs only:

  * `configs/USA/disc1.yaml`  — the matched-`c` span list (name + file/VMA/words).
  * `src/func_*.c`            — the verified decompiled body of each leaf: its
                                outgoing `func_*` calls and the data symbols it
                                declares (`extern`), i.e. its dependency surface.
  * `pc_port/**/*.c`          — which `func_*` symbols the native port defines,
                                and where.

Every matched leaf is classified into exactly one bucket:

  absent          no pc_port definition anywhere and no decomp-derived shadow
  stub            a pc_port definition whose body is a bootstrap/stub provider
  hand-translated a real pc_port definition written independently of `src/`
  ported-from-decomp
                  a pc_port definition that carries a decomp provenance header
                  (`pc_port/game/decomp/*_port.c`), i.e. derived from the
                  verified matched leaf

`ported-from-decomp` is detected structurally: the generated port TUs live in
`pc_port/game/decomp/` and carry a `decomp-source:` provenance line naming the
matched leaf's VMA/span.  Nothing here invents classifications; an unknown
`asm`-only symbol is reported as absent, never guessed.

Output is stable/diffable (sorted sections, one summary line); `--json` emits a
machine-readable report.  Re-run it as matching advances — it re-reads the YAML
and `src/` every time.

Exit status is 0 for a successful report (low coverage is not an error).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CONFIG = Path("configs/USA/disc1.yaml")
SRC_GLOB = "src/func_*.c"
PORT_GLOB = "pc_port/**/*.c"
DECOMP_PORT_DIR = Path("pc_port/game/decomp")
PORT_SUFFIX = "_port.c"

LOAD_VRAM = 0x80010000
HEADER_BYTES = 0x800

C_SPAN_RE = re.compile(
    r"-\s*\[(0x[0-9A-Fa-f]+),\s*c,\s*(func_[0-9A-Fa-f]{8})\]"
)
EDGE_RE = re.compile(r"^[ \t]*-[ \t]*\[(0x[0-9A-Fa-f]+)", re.MULTILINE)
FUNC_CALL_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\s*\(")
FUNC_ANY_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\b")
DATA_SYM_RE = re.compile(r"\b(D_[0-9A-Fa-f]{7,8})\b")
EXTERN_RE = re.compile(r"^\s*extern\s+(.+?);", re.MULTILINE)
# A generated decomp port states its provenance as a line comment.
PROVENANCE_RE = re.compile(r"decomp-source:\s*(func_[0-9A-Fa-f]{8})")
# A real pc_port definition of a func_ symbol at file scope.
DEF_RE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(func_[0-9A-Fa-f]{8})\s*\(", re.MULTILINE
)
STUB_MARKERS = ("BOOTSTRAP_RET", "Bootstrap_ReturnVoid", "Bootstrap_ReturnInt")


def vram_of(file_offset: int) -> int:
    return LOAD_VRAM + file_offset - HEADER_BYTES


def parse_matched() -> list[dict]:
    cfg = (REPO_ROOT / CONFIG).read_text(encoding="utf-8")
    offsets = [int(m.group(1), 16) for m in EDGE_RE.finditer(cfg)]
    leaves: list[dict] = []
    for m in C_SPAN_RE.finditer(cfg):
        start = int(m.group(1), 16)
        end = next((o for o in offsets if o > start), None)
        if end is None:
            raise SystemExit(f"pc_port_coverage: cannot resolve end of {m.group(2)}")
        leaves.append(
            {
                "name": m.group(2),
                "file_offset": start,
                "file_size": end - start,
                "vram": vram_of(start),
                "words": (end - start) // 4,
            }
        )
    names = [leaf["name"] for leaf in leaves]
    if len(names) != len(set(names)):
        dupes = sorted({n for n in names if names.count(n) > 1})
        raise SystemExit(f"pc_port_coverage: duplicate c spans: {dupes}")
    return sorted(leaves, key=lambda leaf: leaf["name"])


def parse_src() -> dict[str, dict]:
    """name -> {callees, globals, lines, path}."""
    out: dict[str, dict] = {}
    for path in sorted(REPO_ROOT.glob(SRC_GLOB)):
        name = path.stem
        text = path.read_text(encoding="utf-8", errors="replace")
        callees = sorted(set(FUNC_CALL_RE.findall(text)) - {name})
        globals_ = sorted(set(DATA_SYM_RE.findall(text)))
        out[name] = {
            "callees": callees,
            "globals": globals_,
            "lines": text.count("\n") + 1,
            "path": str(path.relative_to(REPO_ROOT)),
        }
    return out


def parse_ports() -> tuple[dict[str, list[str]], dict[str, str]]:
    """Return (definitions, provenance).

    definitions: func name -> sorted list of pc_port source files that define it
    provenance:  func name -> matched-leaf name a decomp port claims to derive
                 from (only for files under pc_port/game/decomp/)
    """
    definitions: dict[str, set[str]] = {}
    provenance: dict[str, str] = {}
    for path in sorted(REPO_ROOT.glob(PORT_GLOB)):
        if "build" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        rel = str(path.relative_to(REPO_ROOT))
        for m in DEF_RE.finditer(text):
            definitions.setdefault(m.group(1), set()).add(rel)
        is_decomp_port = (REPO_ROOT / DECOMP_PORT_DIR) in path.parents
        if is_decomp_port:
            for m in PROVENANCE_RE.finditer(text):
                provenance.setdefault(m.group(1), m.group(1))
    return (
        {k: sorted(v) for k, v in definitions.items()},
        provenance,
    )


def classify(
    leaf: dict, src: dict | None, definitions: dict, provenance: dict, stub_files: set[str]
) -> tuple[str, list[str], str]:
    """Return (bucket, defining_files, note)."""
    name = leaf["name"]
    files = definitions.get(name, [])
    if provenance.get(name) == name:
        return "ported-from-decomp", files, "decomp-derived shadow TU"
    if not files:
        return "absent", files, "no pc_port definition"
    if all(f in stub_files for f in files):
        return "stub", files, "bootstrap/stub provider only"
    return "hand-translated", files, "independent pc_port implementation"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    ap.add_argument(
        "--list-bucket",
        metavar="BUCKET",
        help="print the names in one bucket (ported-from-decomp, hand-translated, stub, absent)",
    )
    ap.add_argument(
        "--deps",
        metavar="NAME",
        help="print the dependency surface of one matched leaf",
    )
    args = ap.parse_args()

    leaves = parse_matched()
    src = parse_src()
    definitions, provenance = parse_ports()

    stub_files: set[str] = set()
    for path in sorted(REPO_ROOT.glob(PORT_GLOB)):
        if "build" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        if any(marker in text for marker in STUB_MARKERS):
            stub_files.add(str(path.relative_to(REPO_ROOT)))

    report = []
    for leaf in leaves:
        name = leaf["name"]
        body = src.get(name)
        bucket, files, note = classify(leaf, body, definitions, provenance, stub_files)
        report.append(
            {
                **leaf,
                "bucket": bucket,
                "port_files": files,
                "note": note,
                "callees": body["callees"] if body else [],
                "globals": body["globals"] if body else [],
                "src_lines": body["lines"] if body else 0,
                "has_src": body is not None,
            }
        )

    if args.deps:
        entry = next((r for r in report if r["name"] == args.deps), None)
        if entry is None:
            raise SystemExit(f"pc_port_coverage: {args.deps} is not a matched c span")
        print(f"{entry['name']}  vram=0x{entry['vram']:08X}  words={entry['words']}")
        print(f"  bucket:   {entry['bucket']} ({entry['note']})")
        print(f"  src:      {entry['src_lines']} lines")
        print(f"  globals:  {len(entry['globals'])} -> {' '.join(entry['globals'])}")
        print(f"  callees:  {len(entry['callees'])} -> {' '.join(entry['callees'])}")
        return 0

    counts = {b: 0 for b in ("ported-from-decomp", "hand-translated", "stub", "absent")}
    for entry in report:
        counts[entry["bucket"]] += 1

    if args.list_bucket:
        if args.list_bucket not in counts:
            raise SystemExit(
                f"pc_port_coverage: unknown bucket {args.list_bucket!r}; "
                f"expected one of {', '.join(counts)}"
            )
        for entry in report:
            if entry["bucket"] == args.list_bucket:
                print(entry["name"])
        return 0

    if args.json:
        json.dump(
            {
                "matched_leaves": len(report),
                "buckets": counts,
                "leaves": report,
            },
            sys.stdout,
            indent=2,
            sort_keys=True,
        )
        sys.stdout.write("\n")
        return 0

    total_words = sum(r["words"] for r in report)
    ported_words = sum(r["words"] for r in report if r["bucket"] == "ported-from-decomp")
    print("pc_port coverage — matched `c` spans -> native port")
    print(f"  matched c leaves: {len(report)}  ({total_words} words)")
    print("")
    for bucket in ("ported-from-decomp", "hand-translated", "stub", "absent"):
        words = sum(r["words"] for r in report if r["bucket"] == bucket)
        print(
            f"  {bucket + ':':<19} {counts[bucket]:>4} leaves  {words:>6} words"
        )
    print("")
    print(
        f"  decomp-derived coverage: {ported_words}/{total_words} words "
        f"({100.0 * ported_words / total_words:.1f}%)"
    )
    print(
        "summary="
        f"matched={len(report)} "
        f"from_decomp={counts['ported-from-decomp']} "
        f"hand={counts['hand-translated']} "
        f"stub={counts['stub']} "
        f"absent={counts['absent']}"
    )

    if os.environ.get("PC_PORT_COVERAGE_VERBOSE"):
        print("")
        print("  leaves not derived from their matched source:")
        for entry in report:
            if entry["bucket"] != "ported-from-decomp":
                print(f"    {entry['bucket']:<18} {entry['name']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
