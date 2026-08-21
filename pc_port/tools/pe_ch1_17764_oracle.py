#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_80017764 opcode 0x1C send (18 words).

Verifies the SHA-1-exact EXE window, jump-table slot, and jal-to-653B8
contract. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80017764
FUNC_END = 0x800177AC  # exclusive; next leaf func_800177AC
JAL_653B8 = 0x80017794
APPEND = 0x800653B8
TABLE_SLOT = 0x80091110  # D_800910A0[0x1C]
ACTOR = 0x8009D2F0

WORDS = [
    0x27BDFFE0,  # addiu $sp, $sp, -32
    0xAFBF0018,  # sw    $ra, 24($sp)
    0x8C820008,  # lw    $v0, 8($a0)          ptr payload
    0x8C830004,  # lw    $v1, 4($a0)          ptr dest id
    0x90480000,  # lbu   $t0, 0($v0)          payload
    0x90650000,  # lbu   $a1, 0($v1)          dest id
    0x8C820000,  # lw    $v0, 0($a0)          ptr dest type
    0x3C03800A,  # lui   $v1, 0x800A
    0x8C63D2F0,  # lw    $v1, -0x2D10($v1)    D_8009D2F0
    0x94460000,  # lhu   $a2, 0($v0)          dest type
    0x94670024,  # lhu   $a3, 0x24($v1)       actor serial
    0x01002021,  # addu  $a0, $t0, $zero
    0x0C0194EE,  # jal   func_800653B8
    0xAFA00010,  # sw    $zero, 16($sp)       extra = 0
    0x8FBF0018,  # lw    $ra, 24($sp)
    0x24020001,  # addiu $v0, $zero, 1
    0x03E00008,  # jr    $ra
    0x27BD0020,  # addiu $sp, $sp, 32
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
    require((FUNC_END - FUNC) // 4 == 18, "18 words")
    require(len(WORDS) == 18, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[2] & 0xFFFF) == 8, "lw args+8 payload ptr")
    require((WORDS[3] & 0xFFFF) == 4, "lw args+4 id ptr")
    require((WORDS[4] >> 26) == 0x24, "lbu payload")
    require((WORDS[5] >> 26) == 0x24, "lbu id")
    require((WORDS[6] & 0xFFFF) == 0, "lw args+0 type ptr")
    imm = WORDS[8] & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    require((0x800A0000 + signed) & 0xFFFFFFFF == ACTOR, "lw D_8009D2F0")
    require((WORDS[9] >> 26) == 0x25, "lhu type")
    require((WORDS[10] & 0xFFFF) == 0x24, "lhu actor+0x24")
    require(WORDS[12] == 0x0C0194EE, "jal encoding")
    require(jal_target(WORDS[12]) == APPEND, "jal 653B8")
    require(WORDS[13] == 0xAFA00010, "extra=0 in jal slot")
    require(WORDS[15] == 0x24020001, "return 1")
    require(WORDS[16] == 0x03E00008, "jr $ra")
    require(load_u32(data, TABLE_SLOT) == FUNC, "jump table 0x1C")
    require(load_u32(data, JAL_653B8) == WORDS[12], "jal site")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_80017764 18/18 words + 0x1C table + jal 653B8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
