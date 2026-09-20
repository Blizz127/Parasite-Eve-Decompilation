#!/usr/bin/env python3
"""auto_leaf.py — try to land retail leaves with zero hand-editing.

Pipeline per function:

  1. m2c (via tools/analysis/m2c_leaf.py) produces candidate C from the
     retail assembly.
  2. The candidate is normalised into the freestanding style this project's
     `src/*.c` leaves use: m2c's `s8/u8/...` typedefs and `M2C_*` helpers are
     expanded inline, and unknown `?` types become `s32`.
  3. The candidate is compiled against each era build profile in
     `configs/USA/disc1_build_profiles.json` and byte-compared to retail by
     `tools/analysis/try_leaf.py`.

Any function that matches is reported with the profile that matched, and the
normalised source is written to `build/auto_leaves/<name>.c` so a human or
agent can finish the job (yaml carve + authoritative `scripts/build_us.sh`).

This finds the *cheap* subset. Everything it cannot match still has the m2c
draft as a starting point. Matching authority remains the fresh build.

Usage:
    python3 tools/analysis/auto_leaf.py NAME [NAME ...]
    python3 tools/analysis/auto_leaf.py --from-file build/port_backed_names.txt --limit 40
    python3 tools/analysis/auto_leaf.py --list-profiles
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/analysis"))

import m2c_leaf  # noqa: E402

PRELUDE = """\
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;
#define M2C_UNK s32
#define M2C_UNK8 s8
#define M2C_UNK16 s16
#define M2C_UNK32 s32
#define M2C_UNK64 s64
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((char *)(expr) + (offset)))
#define M2C_BITWISE(type, expr) ((type)(expr))
#define M2C_LWL(expr) (expr)
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_ERROR(desc) (0)
#define M2C_TRAP_IF(cond) (0)
#define M2C_BREAK() (0)
#define MULT_HI(a, b) ((s32)(((s64)(a) * (s64)(b)) >> 32))
#define MULTU_HI(a, b) ((u32)(((u64)(a) * (u64)(b)) >> 32))
#define DMULT_HI(a, b) ((s64)(((s64)(a) * (s64)(b)) >> 32))
"""

# Profiles worth sweeping first: the ones that actually matched leaves before.
DEFAULT_PROFILES = [
    "era_o2_g0",
    "era_o1_g0",
    "era_o2_g0_three_word",
    "era_o2_g0_fill_store_delay_slot",
    "era_o2_g0_fill_indexed_store_delay_slot",
    "era_o2_g0_fill_epilogue_delay_slot",
    "era_o2_g0_passthrough_load",
    "era_o2_g0_expand_div",
    "era_o2_g8",
    "era_o1_g0_sched2",
    "era_o1_g0_sched2_three_word",
    "era_o2_g0_aspsx_230",
]


def load_profiles() -> dict[str, dict]:
    path = ROOT / "configs/USA/disc1_build_profiles.json"
    return json.loads(path.read_text())["profiles"]


def normalise(text: str) -> str:
    """Make m2c output compilable in the project's freestanding leaf style."""
    # Drop m2c's own extern decls for symbols the build already knows; keep
    # them, they are harmless and give the symbol a type.
    text = re.sub(r"\bextern \? (\w+);", r"extern s32 \1;", text)
    # Unknown `?` type in declarations and casts.
    text = re.sub(r"\b\? (\w+);", r"s32 \1;", text)
    text = re.sub(r"\(\?\)", "(s32)", text)
    text = re.sub(r"\(\? \*\)", "(s32 *)", text)
    text = text.replace("? *", "s32 *")
    return PRELUDE + "\n" + text


def _needs_distrobox() -> bool:
    """The era toolchain (cpp/cc1/as) only exists inside the pe-mipsel box."""
    import shutil

    return shutil.which("mipsel-linux-gnu-as") is None


def compile_with_profile(source: Path, offset: int, size: int, profile: dict) -> tuple[bool, str]:
    cmd = [
        sys.executable,
        str(ROOT / "tools/analysis/try_leaf.py"),
        str(source),
        hex(offset),
        hex(size),
        "--flags",
        " ".join(profile.get("flags", [])) or "-O2 -G0",
    ]
    for key, value in (profile.get("environment") or {}).items():
        if key == "MASPSX_FORCE_ABSOLUTE_SYMBOLS":
            cmd += ["--force-absolute", value]
        else:
            cmd += ["--env", f"{key}={value}"]
    if _needs_distrobox():
        import shlex

        inner = "cd " + shlex.quote(str(ROOT)) + " && " + " ".join(
            shlex.quote(part) for part in cmd
        )
        cmd = ["distrobox", "enter", "pe-mipsel", "--", "bash", "-lc", inner]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    return "WORDS MATCH" in proc.stdout, proc.stdout + proc.stderr


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("names", nargs="*")
    ap.add_argument("--from-file")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--profiles", help="comma-separated profile names")
    ap.add_argument("--list-profiles", action="store_true")
    ap.add_argument("--out-dir", default=str(ROOT / "build/auto_leaves"))
    args = ap.parse_args()

    profiles = load_profiles()
    if args.list_profiles:
        for name in sorted(profiles):
            print(name)
        return 0

    names = list(args.names)
    if args.from_file:
        names += [
            line.split()[0]
            for line in Path(args.from_file).read_text().splitlines()
            if line.strip() and not line.startswith("#")
        ]
    if not names:
        ap.error("no function names given")
    if args.limit:
        names = names[: args.limit]

    chosen = (
        [p.strip() for p in args.profiles.split(",") if p.strip()]
        if args.profiles
        else DEFAULT_PROFILES
    )
    for p in chosen:
        if p not in profiles:
            print(f"unknown profile {p}", file=sys.stderr)
            return 2

    index = m2c_leaf.build_worklist_index()
    context = m2c_leaf.generate_context()
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    matched: list[tuple[str, str]] = []
    for name in names:
        entry = index.get(name)
        unit = m2c_leaf.unit_for(name, index)
        if entry is None or unit is None:
            print(f"{name}: no worklist entry / asm; skipped")
            continue
        raw = m2c_leaf.run_m2c(name, unit, context)
        if raw.startswith("/* m2c failed"):
            print(f"{name}: m2c failure")
            continue
        source_text = normalise(raw)
        scratch = out_dir / f"{name}.c"
        scratch.write_text(source_text)
        offset = int(name[5:], 16) - 0x8000F800
        hit = None
        for pname in chosen:
            ok, _log = compile_with_profile(scratch, offset, entry["size"], profiles[pname])
            if ok:
                hit = pname
                break
        if hit:
            matched.append((name, hit))
            print(f"{name}: MATCH with {hit}  -> {scratch.relative_to(ROOT)}")
        else:
            print(f"{name}: no profile matched ({entry['size']} bytes)")

    print(f"\nmatched {len(matched)}/{len(names)}: {matched}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
