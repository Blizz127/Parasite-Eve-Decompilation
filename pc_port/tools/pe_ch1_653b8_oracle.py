#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_800653B8 mailbox append (18 words).

Verifies the SHA-1-exact EXE window and the documented record layout /
stride against build/disc1.candidate.exe. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800653B8
FUNC_END = 0x80065400  # exclusive; next leaf func_80065400
SOLE_JAL_SITE = 0x80017794
GP_COUNT_OFF = 0x44
TABLE = 0x800A3180
COUNT = 0x8009CDB4

WORDS = [
    0x93830044,  # lbu  $v1, 0x44($gp)
    0x8FA80010,  # lw   $t0, 0x10($sp)
    0x00031040,  # sll  $v0, $v1, 1
    0x00431021,  # addu $v0, $v0, $v1
    0x00021080,  # sll  $v0, $v0, 2
    0x3C03800A,  # lui  $v1, 0x800A
    0x24633180,  # addiu $v1, $v1, 0x3180
    0x00431021,  # addu $v0, $v0, $v1
    0xA0440003,  # sb   $a0, 3($v0)
    0xA0450002,  # sb   $a1, 2($v0)
    0x93830044,  # lbu  $v1, 0x44($gp)
    0xAC470008,  # sw   $a3, 8($v0)
    0xA4460000,  # sh   $a2, 0($v0)
    0xAC480004,  # sw   $t0, 4($v0)
    0x24630001,  # addiu $v1, $v1, 1
    0xA3830044,  # sb   $v1, 0x44($gp)
    0x03E00008,  # jr   $ra
    0x00000000,  # nop
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
    require((WORDS[0] >> 26) == 0x24, "first op lbu")
    require((WORDS[0] & 0xFFFF) == GP_COUNT_OFF, "count gp+0x44")
    require((WORDS[1] & 0xFFFF) == 0x10, "5th arg at 16($sp)")
    require((WORDS[5] >> 26) == 0x0F, "lui op")
    require((WORDS[5] & 0xFFFF) == 0x800A, "lui 0x800A")
    require((WORDS[6] & 0xFFFF) == (TABLE & 0xFFFF), "addiu 0x3180")
    require((WORDS[8] & 0xFFFF) == 3, "sb payload rec+3")
    require((WORDS[9] & 0xFFFF) == 2, "sb id rec+2")
    require((WORDS[11] & 0xFFFF) == 8, "sw sender rec+8")
    require((WORDS[12] & 0xFFFF) == 0, "sh type rec+0")
    require((WORDS[13] & 0xFFFF) == 4, "sw extra rec+4")
    require(WORDS[16] == 0x03E00008, "jr $ra")
    require(WORDS[17] == 0, "nop delay")
    require(load_u32(data, SOLE_JAL_SITE) == 0x0C0194EE, "sole jal encoding")
    require(jal_target(load_u32(data, SOLE_JAL_SITE)) == FUNC, "jal 653B8")
    require(load_u32(data, SOLE_JAL_SITE + 4) == 0xAFA00010, "extra=0 in jal slot")
    # stride: sll 1, addu, sll 2 == *3 then *4 == *12
    require(WORDS[2] == 0x00031040, "sll count,1")
    require(WORDS[4] == 0x00021080, "sll *4")
    require(COUNT == 0x8009CD70 + GP_COUNT_OFF, "gp+0x44 == D_8009CDB4")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_800653B8 18/18 words + layout/jal")
    return 0


if __name__ == "__main__":
    sys.exit(main())
