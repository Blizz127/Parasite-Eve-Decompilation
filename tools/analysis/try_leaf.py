#!/usr/bin/env python3
"""Fast single-leaf compile-and-diff loop for Disc 1 matching work.

Compiles one `src/func_*.c` with the era toolchain exactly the way
`tools/build/disc1_build.py` does for one YAML C unit, then compares its
`.text` words against the retail bytes at a given file offset. This is a
*triage* aid: a full `scripts/build_us.sh` is still the only matching
authority, and a green diff here must be confirmed by that harness.

Usage:
    python3 tools/analysis/try_leaf.py SRC OFFSET SIZE \
        [--flags "-O2 -G0"] [--env MASPSX_THREE_WORD_SYMBOL_STORE=1] ...
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ERA_CPP = ROOT / "tools/era/gcc-2.7.2-psx/cpp"
ERA_CC1 = ROOT / "tools/era/gcc-2.7.2-psx/cc1"
MASPSX = ROOT / "tools/era/maspsx/maspsx.py"
EXE = ROOT / "build/extracted/disc1/SLUS_006.62"
AS_FLAGS = ["-EL", "-mips1", "-mabi=32"]


def run(cmd, **kwargs):
    return subprocess.run(cmd, check=True, **kwargs)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("source")
    ap.add_argument("offset", type=lambda s: int(s, 0))
    ap.add_argument("size", type=lambda s: int(s, 0))
    ap.add_argument("--flags", default="-O2 -G0")
    ap.add_argument("--env", action="append", default=[])
    ap.add_argument("--keep", action="store_true")
    args = ap.parse_args()

    env = os.environ.copy()
    for item in args.env:
        key, _, value = item.partition("=")
        env[key] = value
    flags = args.flags.split()

    retail = EXE.read_bytes()[args.offset : args.offset + args.size]

    with tempfile.TemporaryDirectory(prefix="pe-try-") as tmp:
        tmp = Path(tmp)
        pre = tmp / "x.i"
        asm = tmp / "x.s"
        exp = tmp / "xm.s"
        obj = tmp / "x.o"
        with pre.open("wb") as stream:
            run([str(ERA_CPP), str(ROOT / args.source)], env=env, stdout=stream,
                stderr=subprocess.DEVNULL)
        run([str(ERA_CC1), "-quiet", *flags, str(pre), "-o", str(asm)], env=env,
            stderr=subprocess.DEVNULL)
        aspsx = env.get("ERA_ASPSX_VER", "2.21")
        cmd = [sys.executable, str(MASPSX), f"--aspsx-version={aspsx}",
               "--dont-expand-li"]
        if env.get("MASPSX_EXPAND_DIV") == "1":
            cmd.append("--expand-div")
        cmd.append(str(asm))
        with exp.open("wb") as stream:
            run(cmd, env=env, stdin=subprocess.DEVNULL, stdout=stream)
        run(["mipsel-linux-gnu-as", *AS_FLAGS, "-I", str(ROOT / "include"),
             "-o", str(obj), str(exp)], env=env)
        raw = tmp / "x.bin"
        run(["mipsel-linux-gnu-objcopy", "-O", "binary", "--only-section=.text",
             str(obj), str(raw)])
        candidate = raw.read_bytes()

    print(f"retail   {len(retail):5d} bytes")
    print(f"candidate{len(candidate):5d} bytes")
    limit = max(len(retail), len(candidate))
    diffs = 0
    for off in range(0, limit, 4):
        a = (retail[off : off + 4] + b"\0" * 4)[:4]
        b = (candidate[off : off + 4] + b"\0" * 4)[:4]
        if a != b:
            diffs += 1
            if diffs <= 24:
                aw = int.from_bytes(a, "little")
                bw = int.from_bytes(b, "little")
                print(f"  0x{off:04X}: retail {aw:08X}  cand {bw:08X}")
    tail = candidate[len(retail):]
    if diffs == 0 and not any(tail):
        note = f" (+{len(tail)} pad bytes, trimmed by the build)" if tail else ""
        print(f"WORDS MATCH{note} (confirm with scripts/build_us.sh)")
        return 0
    print(f"{diffs} differing word(s)")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
