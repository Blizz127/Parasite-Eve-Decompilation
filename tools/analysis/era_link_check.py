#!/usr/bin/env python3
"""Link-level exactness check for an era leaf.

Compiles a single `src/*.c` leaf with the era toolchain (cpp -> cc1 -> maspsx
-> as), links it at its retail VMA with every undefined symbol `--defsym`'d to
its retail address (the repo's address-named-symbol convention), then compares
the linked `.text` word-for-word against the extracted retail EXE.

Usage:
  tools/analysis/era_link_check.py <src.c> <vram_hex> <size_hex> [cc1 flags...]

Prints `LINK_EXACT` and exits 0 when every word matches, else prints the
mismatch count / first offsets and exits 1.
"""
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
ERA = ROOT / "tools" / "era"
CC1 = ERA / "gcc-2.7.2-psx" / "cc1"
CPP = ERA / "gcc-2.7.2-psx" / "cpp"
MASPSX = ERA / "maspsx" / "maspsx.py"
HOST = ROOT / "tools" / "mipsel-host"
AS = HOST / "usr" / "bin" / "mipsel-linux-gnu-as"
LD = HOST / "usr" / "bin" / "mipsel-linux-gnu-ld"
OBJCOPY = HOST / "usr" / "bin" / "mipsel-linux-gnu-objcopy"
NM = HOST / "usr" / "bin" / "mipsel-linux-gnu-nm"
EXE = ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62"

SYM_TAIL_RE = re.compile(r"([0-9A-Fa-f]{6,8})$")


def elf_section(data: bytes, want: str):
    """Return the raw bytes of a named ELF32 section, or None."""
    if data[:4] != b"\x7fELF":
        return None
    e_shoff = struct.unpack_from("<I", data, 32)[0]
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]
    shstr_off = struct.unpack_from("<I", data, e_shoff + e_shstrndx * e_shentsize + 16)[0]
    for i in range(e_shnum):
        sh = e_shoff + i * e_shentsize
        name_off = struct.unpack_from("<I", data, sh)[0]
        start = shstr_off + name_off
        end = data.index(b"\x00", start)
        if data[start:end].decode("ascii", "replace") != want:
            continue
        off = struct.unpack_from("<I", data, sh + 16)[0]
        sz = struct.unpack_from("<I", data, sh + 20)[0]
        return data[off:off + sz]
    return None


def sym_address(name: str):
    """Resolve a repo address-named symbol to its retail VMA."""
    m = SYM_TAIL_RE.search(name)
    if not m:
        return None
    val = int(m.group(1), 16)
    if val < 0x80000000:
        val |= 0x80000000
    return val


def main():
    src, vram_s, size_s = sys.argv[1], sys.argv[2], sys.argv[3]
    flags = sys.argv[4:] or ["-O2", "-G0"]
    vram = int(vram_s, 0)
    size = int(size_s, 0)
    src_path = Path(src)
    if not src_path.is_absolute():
        src_path = ROOT / src_path

    with tempfile.TemporaryDirectory() as d:
        d = Path(d)
        with open(d / "x.i", "wb") as fi:
            r = subprocess.run([str(CPP), str(src_path)], stdout=fi,
                               stderr=subprocess.DEVNULL)
        assert r.returncode == 0, "cpp failed"
        r = subprocess.run([str(CC1), "-quiet", *flags, str(d / "x.i"),
                            "-o", str(d / "x.s")])
        assert r.returncode == 0, "cc1 failed"
        # Same knob as the in-tree build: remove the `.extern` for forced-absolute
        # symbols so maspsx/as macro-expand them absolutely instead of gp-relative.
        forced = os.environ.get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
        if forced:
            text = (d / "x.s").read_text()
            for symbol in forced.split(","):
                text = re.sub(
                    rf"\t\.extern\t{re.escape(symbol)}, \d+\n", "", text)
            (d / "x.s").write_text(text)
        with open(d / "xm.s", "w") as fo:
            maspsx_command = [sys.executable, str(MASPSX),
                              f"--aspsx-version={os.environ.get('ERA_ASPSX_VER', '2.21')}",
                              "--dont-expand-li"]
            if os.environ.get("MASPSX_EXPAND_DIV") == "1":
                maspsx_command.append("--expand-div")
            maspsx_command.append(str(d / "x.s"))
            r = subprocess.run(maspsx_command, stdout=fo, stderr=subprocess.DEVNULL)
        assert r.returncode == 0, "maspsx failed"
        r = subprocess.run([str(AS), "-EL", "-mips1", "-mabi=32",
                            "-I", str(ROOT / "include"), "-o", str(d / "x.o"),
                            str(d / "xm.s")])
        assert r.returncode == 0, "as failed"

        nm = subprocess.run([str(NM), "-u", str(d / "x.o")],
                            capture_output=True, text=True).stdout
        undef = []
        for line in nm.splitlines():
            parts = line.split()
            if len(parts) >= 2 and parts[0] == "U":
                undef.append(parts[1])
        defsyms = []
        unresolved = []
        for s in undef:
            a = sym_address(s)
            if a is None:
                unresolved.append(s)
            else:
                defsyms += ["--defsym", f"{s}={a:#x}"]
        if unresolved:
            print(f"UNRESOLVED_SYMBOLS {unresolved}")
            return 1

        # Link with an explicit script so the object's .text lands exactly at the
        # retail VMA (the default link forces 16-byte section alignment, which
        # shifts absolute `j`/`jal` targets by the padding).
        leaf = src_path.stem
        script = d / "link.ld"
        script.write_text(
            "SECTIONS {\n"
            f"  . = {vram:#x};\n"
            "  .text : SUBALIGN(4) { *(.text) }\n"
            "  /DISCARD/ : { *(.MIPS.abiflags) *(.reginfo) *(.pdr) *(.comment) }\n"
            "}\n"
        )
        ldargs = [str(LD), "-T", str(script), "-o", str(d / "x.elf"),
                  str(d / "x.o"), "--defsym", "_gp=0x8009CD70", *defsyms]
        r = subprocess.run(ldargs, capture_output=True, text=True)
        if r.returncode != 0:
            print("LD FAILED\n" + r.stdout + r.stderr)
            return 1
        elf = (d / "x.elf").read_bytes()
        linked = elf_section(elf, ".text")
        nmall = subprocess.run([str(NM), str(d / "x.elf")],
                               capture_output=True, text=True).stdout
        func_addr = None
        for line in nmall.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[2] == leaf:
                func_addr = int(parts[0], 16)
                break
        if func_addr is None:
            print("NO_FUNC_SYMBOL_IN_LINKED_ELF")
            return 1
        lead = func_addr - vram
        if lead < 0 or lead > len(linked):
            print(f"BAD_LEAD {lead:#x}")
            return 1
        linked = linked[lead:]

    exe = EXE.read_bytes()
    off = vram - 0x80010000 + 0x800
    rom = exe[off:off + size]

    n = min(len(rom), len(linked), size)
    mism = 0
    first = []
    for i in range(0, n, 4):
        rw = struct.unpack_from("<I", rom, i)[0]
        lw_ = struct.unpack_from("<I", linked, i)[0] if i + 4 <= len(linked) else None
        if lw_ is None or rw != lw_:
            mism += 1
            if len(first) < 12:
                first.append((vram + i, rw, lw_))
    # trailing linked bytes beyond target must be zero pad
    extra = linked[size:]
    extra_nonzero = sum(1 for i in range(0, len(extra) - 3, 4)
                        if struct.unpack_from("<I", extra, i)[0] != 0)
    print(f"linked .text {len(linked)} bytes, target {size:#x}, word mismatches={mism}, "
          f"nonzero_pad={extra_nonzero}")
    for a, rw, lw_ in first:
        print(f"  {a:#x}: ROM {rw:08x}  LNK {lw_:08x}" if lw_ is not None
              else f"  {a:#x}: ROM {rw:08x}  LNK --------")
    if mism == 0 and extra_nonzero == 0:
        print("LINK_EXACT")
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
