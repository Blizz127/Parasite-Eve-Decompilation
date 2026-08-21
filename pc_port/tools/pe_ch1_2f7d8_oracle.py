#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_8002F7D8 opcode 0x6F slot alloc (102 words).

Verifies the SHA-1-exact EXE window, 0x6F wrapper, SlotRecord stride,
216-byte template, and jal func_8001A680. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x8002F7D8
FUNC_END = 0x8002F970  # exclusive; matching func_8002F970
WRAPPER = 0x80018954
JAL_SITE = 0x80018964
TABLE_SLOT = 0x8009125C  # D_800910A0[0x6F]
TEMPLATE = 0x800109B0
BODY_BASE = 0x800A5D5C
INUSE = 0x800A5D58
D2EC = 0x8009D2EC
D2A0 = 0x8009D2A0
CALLEE = 0x8001A680
ACTOR = 0x8009D2F0

WORDS = [
    0x27BDFF10, 0xAFBF00E8, 0x00804821, 0x27A70010, 0x3C068001, 0x24C609B0, 0x24C800D0, 0x8CC20000,
    0x8CC30004, 0x8CC40008, 0x8CC5000C, 0xACE20000, 0xACE30004, 0xACE40008, 0xACE5000C, 0x24C60010,
    0x14C8FFF6, 0x24E70010, 0x8CC20000, 0x8CC30004, 0xACE20000, 0xACE30004, 0x00004021, 0x240C0001,
    0x3C0A800A, 0x254A5D5C, 0x27AB00E0, 0x310200FF, 0x000218C0, 0x00621823, 0x000318C0, 0x00621823,
    0x00031880, 0x3C01800A, 0x00230821, 0x8C225D58, 0x00000000, 0x14400037, 0x006A3821, 0x3C01800A,
    0x00230821, 0xAC2C5D58, 0x27A60010, 0x8CC20000, 0x8CC30004, 0x8CC40008, 0x8CC5000C, 0xACE20000,
    0xACE30004, 0xACE40008, 0xACE5000C, 0x24C60010, 0x14CBFFF6, 0x24E70010, 0x8CC20000, 0x8CC30004,
    0xACE20000, 0xACE30004, 0x310400FF, 0x000410C0, 0x00441023, 0x000210C0, 0x00441023, 0x00021080,
    0x3C03800A, 0x9063D2EC, 0x004A1021, 0xAD220000, 0x24630001, 0x3C01800A, 0xA023D2EC, 0xA0430007,
    0x8D220000, 0x008C2004, 0xAC440008, 0x8D220098, 0x00000000, 0x30422000, 0x14400013, 0x01202021,
    0x8D230000, 0x24050002, 0x2462001C, 0x0C0069A0, 0xAC620018, 0x3C02800A, 0x9042D2A0, 0x00000000,
    0x24420001, 0x3C01800A, 0xA022D2A0, 0x0800BE58, 0x00000000, 0x25080001, 0x310200FF, 0x2C420007,
    0x1440FFBB, 0x310200FF, 0x8FBF00E8, 0x27BD00F0, 0x03E00008, 0x00000000,
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x3FFFFFF) << 2) | 0x80000000


def lui_addiu(hi_word: int, lo_word: int) -> int:
    hi = (hi_word & 0xFFFF) << 16
    imm = lo_word & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    return (hi + signed) & 0xFFFFFFFF


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 102, "102 words")
    require(len(WORDS) == 102, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require(WORDS[0] == 0x27BDFF10, "frame -0xF0")
    require(lui_addiu(WORDS[4], WORDS[5]) == TEMPLATE, "template 0x800109B0")
    require((WORDS[6] & 0xFFFF) == 0xD0, "loop 0xD0")
    require(lui_addiu(WORDS[24], WORDS[25]) == BODY_BASE, "D_800A5D5C")
    require((WORDS[35] & 0xFFFF) == 0x5D58, "lw inUse D_800A5D58")
    require(WORDS[23] == 0x240C0001, "li 1 claim")
    require((WORDS[41] & 0xFFFF) == 0x5D58, "sw inUse")
    require((WORDS[65] & 0xFFFF) == 0xD2EC, "lbu D_8009D2EC")
    require((WORDS[70] & 0xFFFF) == 0xD2EC, "sb D_8009D2EC")
    require((WORDS[71] & 0xFFFF) == 7, "sb body+7")
    require((WORDS[74] & 0xFFFF) == 8, "sw body+8")
    require((WORDS[75] & 0xFFFF) == 0x98, "lw actor+0x98")
    require((WORDS[77] & 0xFFFF) == 0x2000, "andi 0x2000")
    require(WORDS[81] == 0x24050002, "a1=2")
    require(jal_target(WORDS[83]) == CALLEE, "jal func_8001A680")
    require((WORDS[84] & 0xFFFF) == 0x18, "sw body+0x18")
    require((WORDS[86] & 0xFFFF) == 0xD2A0, "lbu D_8009D2A0")
    require((WORDS[90] & 0xFFFF) == 0xD2A0, "sb D_8009D2A0")
    require((WORDS[95] & 0xFFFF) == 7, "sltiu 7")
    require(WORDS[100] == 0x03E00008, "jr $ra")
    require(load_u32(data, FUNC_END) == 0x00003021, "next leaf 2F970")
    require(load_u32(data, TABLE_SLOT) == WRAPPER, "jump table 0x6F")
    require(load_u32(data, JAL_SITE) == 0x0C00BDF6, "wrapper jal encoding")
    require(jal_target(load_u32(data, JAL_SITE)) == FUNC, "wrapper jal 2F7D8")
    imm = load_u32(data, WRAPPER + 4) & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    require((0x800A0000 + signed) & 0xFFFFFFFF == ACTOR, "wrapper D_8009D2F0")
    enc = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = []
    for i in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, i)[0] == enc:
            hits.append(0x80010000 + (i - 0x800))
    require(hits == [JAL_SITE], f"sole jal site, got {hits}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_8002F7D8 102/102 words + 0x6F wrapper + 216B template + jal 1A680")
    return 0


if __name__ == "__main__":
    sys.exit(main())
