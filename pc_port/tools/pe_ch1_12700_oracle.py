#!/usr/bin/env python3
"""PE-CH1 independent oracle: func_80012700 task spawn (29 words).

Verifies the SHA-1-exact EXE window, freelist/serial gp slots, and
all seven jal sites. Does not import production C.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC = 0x80012700
FUNC_END = 0x80012774
FREE = 0x8009CDFC  # 0x8C($gp)
SERIAL = 0x8009D308  # 0x598($gp)
GP = 0x8009CD70
MAILBOX = (0x80065484, 0x8006553C)

WORDS = [
    0x8F86008C, 0x00000000, 0x8CC20024, 0x00000000,
    0xAF82008C, 0x10A00009, 0x00000000, 0xACC50028,
    0x8CA20024, 0x00000000, 0x10400002, 0xACC20024,
    0xAC460028, 0x080049D1, 0xACA60024, 0xACC00028,
    0xACC00024, 0x97820598, 0x24030001, 0xACC0000C,
    0xACC40000, 0xACC00004, 0xACC30010, 0xA4C00008,
    0x24430001, 0xA4C2000A, 0xA7830598, 0x03E00008,
    0x00C01021,
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
    require((FUNC_END - FUNC) // 4 == 29, "29 words")
    require(len(WORDS) == 29, "word table")
    for i, want in enumerate(WORDS):
        got = load_u32(data, FUNC + i * 4)
        require(got == want, f"word {i} @{FUNC + i * 4:#x} {got:#010x}")


def verify_layout(data: bytes) -> None:
    require((WORDS[0] & 0xFFFF) == 0x8C, "lw 0x8C($gp)")
    require(GP + 0x8C == FREE, "freelist D_8009CDFC")
    require((WORDS[2] & 0xFFFF) == 0x24, "lw next +0x24")
    require((WORDS[4] & 0xFFFF) == 0x8C, "sw pop 0x8C($gp)")
    require((WORDS[5] >> 26) == 4, "beq a1")
    require((WORDS[7] & 0xFFFF) == 0x28, "sw a1 task+0x28")
    require((WORDS[15] & 0xFFFF) == 0x28, "a1==0 clear +0x28")
    require((WORDS[16] & 0xFFFF) == 0x24, "a1==0 clear +0x24")
    require((WORDS[17] & 0xFFFF) == 0x598, "lhu 0x598($gp)")
    require(GP + 0x598 == SERIAL, "serial D_8009D308")
    require(WORDS[18] == 0x24030001, "li 1")
    require((WORDS[19] & 0xFFFF) == 0x0C, "sw 0 +0x0C")
    require((WORDS[20] & 0xFFFF) == 0, "sw entry +0x00")
    require((WORDS[21] & 0xFFFF) == 4, "sw 0 +0x04")
    require((WORDS[22] & 0xFFFF) == 0x10, "sw 1 +0x10")
    require((WORDS[23] & 0xFFFF) == 8, "sh 0 +0x08")
    require((WORDS[25] & 0xFFFF) == 0x0A, "sh serial +0x0A")
    require((WORDS[26] & 0xFFFF) == 0x598, "sh serial+1")
    require(WORDS[27] == 0x03E00008, "jr $ra")
    require(WORDS[28] == 0x00C01021, "return a2")
    hits = []
    for i in range(0, len(data) - 4, 4):
        w = struct.unpack_from("<I", data, i)[0]
        if w == 0x0C0049C0:
            hits.append(0x80010000 + (i - 0x800))
    require(len(hits) == 7, f"seven jal sites, got {hits}")
    for site in MAILBOX:
        require(site in hits, f"mailbox jal {site:#x}")
        require(jal_target(load_u32(data, site)) == FUNC, f"tgt {site:#x}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    exe = root / "build" / "disc1.candidate.exe"
    require(exe.is_file(), f"missing {exe}")
    data = load_exe(exe)
    verify_words(data)
    verify_layout(data)
    print("PASS: func_80012700 29/29 words + 7 jal sites + gp slots")
    return 0


if __name__ == "__main__":
    sys.exit(main())
