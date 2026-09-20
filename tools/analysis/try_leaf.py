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
import re
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
    ap.add_argument(
        "--force-absolute",
        default="",
        help="comma-separated symbols whose '.extern SYM, N' directive the "
        "cc1 assembly must have stripped before maspsx (mirrors the build's "
        "MASPSX_FORCE_ABSOLUTE_SYMBOLS leaf environment).",
    )
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
        # Mirror tools/build/disc1_build.py's MASPSX_FORCE_ABSOLUTE_SYMBOLS:
        # drop the named symbols' ".extern SYM, N" so maspsx treats them as
        # absolute rather than small-data-relative.  A separate output file
        # keeps the cc1 assembly available for inspection with --keep.
        if args.force_absolute:
            text = asm.read_text(encoding="utf-8", errors="replace")
            for symbol in args.force_absolute.split(","):
                symbol = symbol.strip()
                if not symbol:
                    continue
                text = re.sub(
                    rf"\t\.extern\t{re.escape(symbol)}, \d+\n", "", text
                )
            absolute = tmp / "x.absolute.s"
            absolute.write_text(text, encoding="utf-8")
            asm = absolute
        aspsx = env.get("ERA_ASPSX_VER", "2.21")
        cmd = [sys.executable, str(MASPSX), f"--aspsx-version={aspsx}"]
        # Mirror disc1_build.py: `li` is left alone unless the leaf opts in
        # with MASPSX_EXPAND_LI=1 (retail `ori $r,$zero,imm` scalar loads).
        if env.get("MASPSX_EXPAND_LI") != "1":
            cmd.append("--dont-expand-li")
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
        candidate = bytearray(raw.read_bytes())
        # Zero every word the object leaves for the linker (HI16/LO16 pairs and
        # jal/j targets) in BOTH images, so a standalone leaf can be compared
        # before the real link resolves symbols.
        rel = subprocess.run(["mipsel-linux-gnu-objdump", "-r", str(obj)],
                             check=True, stdout=subprocess.PIPE,
                             stderr=subprocess.DEVNULL).stdout.decode()
        # Only .text relocations index the .text binary.  objdump -r also
        # lists .pdr/.rel/.data relocations, whose offsets are unrelated to
        # .text; folding those in used to zero arbitrary .text words (a .pdr
        # entry at offset 0 masked the first instruction) and could hide a
        # real difference — a false "match".
        reloc = []
        section = ""
        for line in rel.splitlines():
            s = line.strip()
            if s.startswith("RELOCATION RECORDS FOR"):
                section = s
                continue
            parts = s.split()
            if (len(parts) >= 2 and parts[1].startswith("R_MIPS")
                    and "[.text]" in section):
                reloc.append(int(parts[0], 16))
        retail = bytearray(retail)
        for off in reloc:
            candidate[off:off + 4] = b"\0\0\0\0"
            retail[off:off + 4] = b"\0\0\0\0"
        retail = bytes(retail)
        candidate = bytes(candidate)

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
