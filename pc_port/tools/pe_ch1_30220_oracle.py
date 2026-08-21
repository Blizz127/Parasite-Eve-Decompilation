#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_80030220 opcode 0x5A slot tagged setter.

Verifies the SHA-1-exact EXE window, jump table D_80010C90, and BTL1
tag stores. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80030220
FUNC_END = 0x80030534  # exclusive; jr+nop
JT = 0x80010C90
JAL_SITES = [0x800181B4, 0x80018288]
WRAPPER = 0x80018164

BTL1 = {
    40: 0x80030250,
    41: 0x80030258,
    42: 0x80030260,
    50: 0x800303B4,
    51: 0x800303BC,
    52: 0x800303C4,
}

WORDS = [
    0x8C840000, 0x30A500FF, 0x24A5FFD8, 0x2CA20055,
    0x104000BE, 0x00051080, 0x3C018001, 0x00220821,
    0x8C220C90, 0x00000000, 0x00400008, 0x00000000,
    0x0800C14B, 0xA486000E, 0x0800C14B, 0xA0860004,
    0x0800C14B, 0xA0860005, 0x0800C14B, 0xAC860010,
    0x0800C14B, 0xAC860014, 0x0800C14B, 0xA486000C,
    0x0800C14B, 0xA0860007, 0x0800C14B, 0xA0860006,
    0x0800C14B, 0xAC860088, 0x0800C14B, 0xA486008C,
    0x0800C14B, 0xA486008E, 0x0800C14B, 0xA0860090,
    0x0800C14B, 0xA0860091, 0x0800C14B, 0xA0860092,
    0x0800C14B, 0xA0860093, 0x0800C14B, 0xA0860094,
    0x0800C14B, 0xA0860095, 0x0800C14B, 0xA4860096,
    0x8C820000, 0x2403FFEF, 0x00431024, 0x30C30001,
    0x00031900, 0x00431025, 0x0800C14B, 0xAC820000,
    0x24020002, 0x10C2000F, 0xA08600A4, 0x28C20003,
    0x10400005, 0x24020001, 0x10C20008, 0x24020190,
    0x0800C14B, 0x00000000, 0x24020003, 0x10C20008,
    0x24020014, 0x0800C14B, 0x00000000, 0x0800C14B,
    0xA48200A6, 0x24020046, 0x0800C14B, 0xA48200A6,
    0x0800C14B, 0xA48200A6, 0x3C02FFEF, 0x8C830000,
    0x3442FFFF, 0x00621824, 0x30C20001, 0x00021500,
    0x00621825, 0x0800C14B, 0xAC830000, 0x0800C14B,
    0xA4860098, 0x0800C14B, 0xA486009A, 0x0800C14B,
    0xA086009E, 0x0800C14B, 0xA086009F, 0x0800C14B,
    0xA08600AE, 0x0800C14B, 0xA08600AF, 0x0800C14B,
    0xA08600BC, 0x0800C14B, 0xA48600B0, 0x0800C14B,
    0xA48600B2, 0x0800C14B, 0xA48600B4, 0x0800C14B,
    0xA48600B6, 0x0800C14B, 0xA48600B8, 0x0800C14B,
    0xA48600BA, 0x8C8200CC, 0x2403FFFC, 0x00431024,
    0x0800C121, 0x30C30003, 0x8C8200CC, 0x2403FFF3,
    0x00431024, 0x30C30003, 0x0800C121, 0x00031880,
    0x8C8200CC, 0x2403FFCF, 0x00431024, 0x30C30003,
    0x0800C121, 0x00031900, 0x8C8200CC, 0x2403FF3F,
    0x00431024, 0x30C30003, 0x0800C121, 0x00031980,
    0x8C8200CC, 0x2403FCFF, 0x00431024, 0x30C30003,
    0x0800C121, 0x00031A00, 0x8C8200CC, 0x2403F3FF,
    0x00431024, 0x30C30003, 0x0800C121, 0x00031A80,
    0x8C8200CC, 0x2403CFFF, 0x00431024, 0x30C30003,
    0x00031B00, 0x00431025, 0x0800C14B, 0xAC8200CC,
    0x3C02FFFF, 0x8C8300CC, 0x34423FFF, 0x00621824,
    0x30C20003, 0x0800C13F, 0x00021380, 0x3C02FFFC,
    0x8C8300CC, 0x3442FFFF, 0x00621824, 0x30C20003,
    0x0800C13F, 0x00021400, 0x3C02FFFB, 0x8C8300CC,
    0x3442FFFF, 0x00621824, 0x30C20001, 0x0800C13F,
    0x00021480, 0x3C02FF07, 0x8C8300CC, 0x3442FFFF,
    0x00621824, 0x30C2001F, 0x000214C0, 0x00621825,
    0x0800C14B, 0xAC8300CC, 0x0800C14B, 0xA48600D0,
    0x0800C14B, 0xA08600D6, 0x0800C14B, 0xA08600D7,
    0x0800C14B, 0xA48600A0, 0xA48600A2, 0x03E00008,
    0x00000000,
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


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 197, "197 words")
    require(len(WORDS) == 197, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require(WORDS[0] == 0x8C840000, "lw *actor")
    require((WORDS[1] & 0xFFFF) == 0xFF, "andi tag")
    require((WORDS[2] & 0xFFFF) == 0xFFD8, "addiu -40")
    require((WORDS[3] & 0xFFFF) == 0x55, "sltiu 85")
    require(WORDS[-2] == 0x03E00008, "jr $ra")
    require(WORDS[-1] == 0, "nop delay")
    require((WORDS[13] & 0xFFFF) == 0xE, "tag40 sh +0xE")
    require((WORDS[15] & 0xFFFF) == 0x4, "tag41 sb +4")
    require((WORDS[17] & 0xFFFF) == 0x5, "tag42 sb +5")
    require((WORDS[102] & 0xFFFF) == 0xB0, "tag50 sh +0xB0")
    require((WORDS[104] & 0xFFFF) == 0xB2, "tag51 sh +0xB2")
    require((WORDS[106] & 0xFFFF) == 0xB4, "tag52 sh +0xB4")
    for tag, dest in BTL1.items():
        got = load_u32(data, JT + (tag - 40) * 4)
        require(got == dest, f"JT tag {tag}")
    require(load_u32(data, 0x80091208) == WRAPPER, "0x5A table")
    require(jal_target(load_u32(data, JAL_SITES[0])) == FUNC, "0x5A jal")
    enc = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = []
    for i in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, i)[0] == enc:
            hits.append(0x80010000 + (i - 0x800))
    require(hits == JAL_SITES, f"jal sites {hits}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_80030220 197/197 words + JT BTL1 tags 40-42/50-52")
    return 0


if __name__ == "__main__":
    sys.exit(main())
