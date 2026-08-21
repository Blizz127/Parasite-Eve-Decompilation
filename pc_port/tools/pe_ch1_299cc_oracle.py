#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_800299CC consume-edge named cut.

Verifies the SHA-1-exact EXE window for the BTL1 D_8009D28C 6→0
consume prefix (gp+0x10C sb + gp+0x51C sw $zero) and the exclusive
end at the first word after those stores. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x800299CC
FUNC_END = 0x80029A0C  # exclusive; lw record+0x4C (reload, not consume)
GP = 0x8009CD70
MODE = 0x8009D28C  # gp+0x51C
EDGE = GP + 0x10C  # 0x8009CE7C
RECORD_P = GP + 0x508  # 0x8009D278
INIT_SB_EDGE = 0x80029838  # sibling init: sb $zero, 0x10C($gp)
JAL_SITES = [0x800355E8]
COMPARE = 0x800299F8  # addiu $v0, $zero, 6
SB_EDGE = 0x80029A04
SW_MODE = 0x80029A08

WORDS = [
    0x8F840508,  # lw    $a0, 0x508($gp)
    0x27BDFE38,  # addiu $sp, $sp, -456
    0xAFB101BC,  # sw    $s1, 444($sp)
    0xAFBF01C0,  # sw    $ra, 448($sp)
    0xAFB001B8,  # sw    $s0, 440($sp)
    0x8C82004C,  # lw    $v0, 0x4C($a0)
    0x3C050008,  # lui   $a1, 0x8            ; 0x00080000
    0x00451024,  # and   $v0, $v0, $a1
    0x1040000C,  # beq   $v0, $zero, 0x80029A20
    0x24110001,  # addiu $s1, $zero, 1       ; delay (not a guest store)
    0x8F83051C,  # lw    $v1, 0x51C($gp)     ; D_8009D28C
    0x24020006,  # addiu $v0, $zero, 6
    0x14620003,  # bne   $v1, $v0, 0x80029A0C
    0x24020006,  # addiu $v0, $zero, 6       ; delay: value stored by sb
    0xA382010C,  # sb    $v0, 0x10C($gp)     ; edge byte = 6
    0xAF80051C,  # sw    $zero, 0x51C($gp)   ; consume 6 -> 0
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def simm16(imm: int) -> int:
    return imm - 0x10000 if imm & 0x8000 else imm


def branch_target(pc: int, word: int) -> int:
    return pc + 4 + simm16(word & 0xFFFF) * 4


def jal_target(word: int) -> int:
    return ((word & 0x3FFFFFF) << 2) | 0x80000000


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def verify_words(data: bytes) -> None:
    require((FUNC_END - FUNC) // 4 == 16, "16 words")
    require(len(WORDS) == 16, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require(GP == MODE - 0x51C, "gp base from D_8009D28C-0x51C")
    require(EDGE == 0x8009CE7C, "gp+0x10C")
    require(RECORD_P == 0x8009D278, "gp+0x508")

    require((WORDS[0] >> 26) == 0x23, "lw record ptr")
    require(((WORDS[0] >> 21) & 0x1F) == 28, "lw from $gp")
    require((WORDS[0] & 0xFFFF) == 0x508, "lw gp+0x508")

    require((WORDS[1] & 0xFFFF) == 0xFE38, "frame -456")
    require((WORDS[5] >> 26) == 0x23, "lw record+0x4C")
    require((WORDS[5] & 0xFFFF) == 0x4C, "imm +0x4C")
    require(WORDS[6] == 0x3C050008, "lui $a1, 0x8")
    require(WORDS[7] == 0x00451024, "and flag")

    require((WORDS[8] >> 26) == 0x4, "beq flag skip")
    require(branch_target(FUNC + 8 * 4, WORDS[8]) == 0x80029A20, "beq -> 0x80029A20")

    require((WORDS[10] >> 26) == 0x23, "lw mode")
    require(((WORDS[10] >> 21) & 0x1F) == 28, "lw mode from $gp")
    require((WORDS[10] & 0xFFFF) == 0x51C, "lw gp+0x51C")
    require(GP + (WORDS[10] & 0xFFFF) == MODE, "gp+0x51C is D_8009D28C")

    require(WORDS[11] == 0x24020006, "addiu $v0, 6")
    require(COMPARE == FUNC + 11 * 4, "compare immediate site")
    require((WORDS[12] >> 26) == 0x5, "bne mode!=6")
    require(branch_target(FUNC + 12 * 4, WORDS[12]) == FUNC_END, "bne -> exclusive end")
    require(WORDS[13] == 0x24020006, "delay addiu 6")

    sb = WORDS[14]
    require((sb >> 26) == 0x28, "sb not sh/sw")
    require(((sb >> 21) & 0x1F) == 28, "sb to $gp")
    require((sb & 0xFFFF) == 0x10C, "sb gp+0x10C")
    require(GP + 0x10C == EDGE, "edge address")
    require(SB_EDGE == FUNC + 14 * 4, "sb site")

    sw = WORDS[15]
    require((sw >> 26) == 0x2B, "sw consume")
    require(((sw >> 16) & 0x1F) == 0, "sw $zero")
    require(((sw >> 21) & 0x1F) == 28, "sw to $gp")
    require((sw & 0xFFFF) == 0x51C, "sw gp+0x51C")
    require(SW_MODE == FUNC + 15 * 4, "sw site")

    end_word = load_u32(data, FUNC_END)
    require(end_word == 0x8C82004C, "exclusive end lw record+0x4C")
    require(end_word == WORDS[5], "reload is same lw as guard")

    init = load_u32(data, INIT_SB_EDGE)
    require((init >> 26) == 0x28, "init sb")
    require((init & 0xFFFF) == 0x10C, "init zeros gp+0x10C as byte")

    enc = 0x0C000000 | ((FUNC & 0x0FFFFFFF) >> 2)
    hits = []
    for i in range(0, len(data) - 4, 4):
        if struct.unpack_from("<I", data, i)[0] == enc:
            hits.append(0x80010000 + (i - 0x800))
    require(hits == JAL_SITES, f"jal sites {hits}")
    require(jal_target(load_u32(data, JAL_SITES[0])) == FUNC, "jal full tick")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print(
        "PASS: func_800299CC_consume_cut 16/16 words + "
        "gp+0x51C D_8009D28C + sb gp+0x10C=6 + exclusive 0x80029A0C"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
