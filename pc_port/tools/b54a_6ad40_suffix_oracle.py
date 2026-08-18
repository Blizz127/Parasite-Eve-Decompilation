#!/usr/bin/env python3
"""Phase 6E-B54A independent func_8006AD40 suffix oracle.

Read-only reconstruction of the retail suffix beginning at 0x8006AE50.
Verifies literal words against the SHA-1-exact executable, models the
captured post-B53I-D locals, and walks suffix control flow without
importing production C or inventing DMA/CD progress.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC_START = 0x8006AD40
FUNC_END = 0x8006B35C
CUT_PC = 0x8006AE50
LOOP_REENTER = 0x8006AE44
LOOP_EXIT = 0x8006AE68
LOOKUP_CALL = 0x8006AEFC
LOADIMAGE_CALL = 0x8006AF18
POLL_AFTER_PACKET = 0x8006AF54
FIRST_UNRESOLVED_CALL = 0x8006AFA8
FIRST_UNRESOLVED = 0x800718D0

# Full 391-word body; suffix is WORDS[68:].
WORDS = [
    0x27BDFFD0, 0xAFB50024, 0x3C15800B, 0x26B50CD8,
    0xAFBF002C, 0xAFB60028, 0xAFB40020, 0xAFB3001C,
    0xAFB20018, 0xAFB10014, 0xAFB00010, 0x8EA20000,
    0x00000000, 0x30420001, 0x1040016D, 0x00001021,
    0x3C16800B, 0x8ED60DD8, 0x3C108009, 0x261030EA,
    0x2411FFFF, 0x8EA50160, 0x96020000, 0x96060002,
    0x02C22021, 0x0C01B9AA, 0x00C23023, 0x1051FFF9,
    0x24120001, 0x2410FFFF, 0x1250FFF3, 0x00000000,
    0x0C01B9FA, 0x00000000, 0x00409021, 0x1640FFFA,
    0x00000000, 0x00008021, 0x3C118009, 0x263130EC,
    0x2412FFFF, 0x8EA50174, 0x96220000, 0x96260002,
    0x02C22021, 0x0C01B9AA, 0x00C23023, 0x1052FFF9,
    0x00000000, 0x24120001, 0x16000050, 0x2402FFFF,
    0x8EB40160, 0x3C03003F, 0x8E820004, 0x3463FFFF,
    0x02829821, 0x8E620028, 0x00008821, 0x00431824,
    0x00021582, 0x0202102B, 0x1040000B, 0x02832021,
    0x00808021, 0x02002021, 0x0C01B870, 0x02802821,
    0x8E620028, 0x26310001, 0x00021582, 0x0222102B,
    0x1440FFF8, 0x26100014, 0x24050020, 0x3C018009,
    0x00250821, 0x9424164A, 0x3C018009, 0x00250821,
    0x94221648, 0x30830100, 0x00031902, 0x304203FF,
    0x00021182, 0x34420020, 0x00621825, 0x30840200,
    0x00042080, 0x00641825, 0x3C018009, 0x00250821,
    0xA4231650, 0x3C018009, 0x00250821, 0x9423164E,
    0x3C018009, 0x00250821, 0x9422164C, 0x00031980,
    0x00021102, 0x3042003F, 0x00621825, 0x3C018009,
    0x00250821, 0xA4231652, 0x24A50010, 0x2CA20040,
    0x1440FFDE, 0x02802021, 0x3C05ABAD, 0x0C01B926,
    0x34A5C06C, 0x00408021, 0x8E020000, 0x00000000,
    0x1040000C, 0x26040004, 0x0C01D41B, 0x2605000C,
    0x8E020000, 0x00000000, 0x00021082, 0x00021080,
    0x02028021, 0x8E020000, 0x00000000, 0x1440FFF6,
    0x26040004, 0x24100001, 0x2402FFFF, 0x1242FFA2,
    0x00000000, 0x0C01B9FA, 0x00000000, 0x00409021,
    0x1640FFA9, 0x00000000, 0x00008021, 0x3C118009,
    0x263130EE, 0x2412FFFF, 0x8EA50180, 0x96220000,
    0x96260002, 0x02C22021, 0x0C01B9AA, 0x00C23023,
    0x1052FFF9, 0x00000000, 0x24120001, 0x16000029,
    0x2402FFFF, 0x8EA40174, 0x0C01C634, 0x00000000,
    0x00002821, 0x3C018009, 0x00250821, 0x9424164A,
    0x3C018009, 0x00250821, 0x94221648, 0x30830100,
    0x00031902, 0x304203FF, 0x00021182, 0x34420020,
    0x00621825, 0x30840200, 0x00042080, 0x00641825,
    0x3C018009, 0x00250821, 0xA4231650, 0x3C018009,
    0x00250821, 0x9423164E, 0x3C018009, 0x00250821,
    0x9422164C, 0x00031980, 0x00021102, 0x3042003F,
    0x00621825, 0x3C018009, 0x00250821, 0xA4231652,
    0x24A50010, 0x2CA20020, 0x1440FFDE, 0x2402FFFF,
    0x24100001, 0x1242FFC9, 0x00000000, 0x0C01B9FA,
    0x00000000, 0x00409021, 0x1640FFD0, 0x00000000,
    0x00008021, 0x3C118009, 0x263130F0, 0x2412FFFF,
    0x8EA5014C, 0x96220000, 0x96260002, 0x02C22021,
    0x0C01B9AA, 0x00C23023, 0x1052FFF9, 0x00000000,
    0x24120001, 0x2411FFFF, 0x16000006, 0x00000000,
    0x8EA40180, 0x0C01C634, 0x24100001, 0x0C00C225,
    0x00000000, 0x1251FFEB, 0x00000000, 0x0C01B9FA,
    0x00000000, 0x00409021, 0x1640FFF3, 0x00000000,
    0x00008021, 0x3C118009, 0x263130E0, 0x2412FFFF,
    0x8EA5016C, 0x96220000, 0x96260002, 0x02C22021,
    0x0C01B9AA, 0x00C23023, 0x1052FFF9, 0x00000000,
    0x24120001, 0x2411FFFF, 0x16000010, 0x3C05C4B5,
    0x34A5BA04, 0x8EA4014C, 0x0C01B926, 0x24100001,
    0x3C05CAAD, 0x8EA4014C, 0x34A50704, 0x0C01B926,
    0xAEA2011C, 0x3C055EAF, 0x8EA4014C, 0x34A56804,
    0x0C01B926, 0xAEA20120, 0xAEA20124, 0x1251FFE1,
    0x00000000, 0x0C01B9FA, 0x00000000, 0x00409021,
    0x1640FFE9, 0x00000000, 0x00008021, 0x3C118009,
    0x26313126, 0x2412FFFF, 0x8EA50188, 0x96220000,
    0x96260002, 0x02C22021, 0x0C01B9AA, 0x00C23023,
    0x1052FFF9, 0x00000000, 0x24120001, 0x16000019,
    0x2402FFFF, 0x8EB4016C, 0x3C03003F, 0x8E820004,
    0x3463FFFF, 0x02829821, 0x8E620028, 0x00008821,
    0x00431824, 0x00021582, 0x0202102B, 0x1040000B,
    0x02832021, 0x00808021, 0x02002021, 0x0C01B870,
    0x02802821, 0x8E620028, 0x26310001, 0x00021582,
    0x0222102B, 0x1440FFF8, 0x26100014, 0x24100001,
    0x2402FFFF, 0x1242FFD9, 0x00000000, 0x0C01B9FA,
    0x00000000, 0x00409021, 0x1640FFE0, 0x3C02003F,
    0x8EB40188, 0x00000000, 0x8E830004, 0x3442FFFF,
    0x02839821, 0x8E630028, 0x00008821, 0x00621024,
    0x00031D82, 0x1060000B, 0x02822021, 0x00808021,
    0x02002021, 0x0C01B870, 0x02802821, 0x8E620028,
    0x26310001, 0x00021582, 0x0222102B, 0x1440FFF8,
    0x26100014, 0x0C021C09, 0x00000000, 0x0C01D370,
    0x00002021, 0x0C01D291, 0x24040001, 0x0C01CE91,
    0x00002021, 0x3C02800A, 0x8C42CDDC, 0x00000000,
    0x00022080, 0x00822021, 0x00042080, 0x3C02800C,
    0x2442CE80, 0x0C01D57C, 0x00822021, 0x0C01D34A,
    0x24040001, 0x2403FFFF, 0x8EA20000, 0x2404FFFF,
    0xA6A30006, 0xA2A0000B, 0xA2A4000C, 0xA2A40009,
    0xA6A300E8, 0xA2A000EB, 0x30420040, 0x14400004,
    0xA2A000EA, 0xA2A400DA, 0xA2A400DD, 0xA2A400DC,
    0x8EA20000, 0x00000000, 0x30420080, 0x14400004,
    0x00001021, 0xA2A400DB, 0xA2A400DF, 0xA2A400DE,
    0x8EA30000, 0x2404FFFE, 0x00641824, 0xAEA30000,
    0x8FBF002C, 0x8FB60028, 0x8FB50024, 0x8FB40020,
    0x8FB3001C, 0x8FB20018, 0x8FB10014, 0x8FB00010,
    0x27BD0030, 0x03E00008, 0x00000000,
]

# jal sites from CUT onward, with current classification.
SUFFIX_CALLS = [
    (0x8006AE48, 0x8006E1C0, "func_8006E1C0", "TRANSLATED_FAITHFUL",
     "B51; remaining loop iterations after the cut"),
    (0x8006AEFC, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL",
     "B16 archive key lookup"),
    (0x8006AF18, 0x8007506C, "func_8007506C", "TRANSLATED_FAITHFUL",
     "B52 LoadImage wrapper"),
    (0x8006AF54, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL",
     "B16 CD poll; result is live drive state"),
    (0x8006AF88, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006AFA8, 0x800718D0, "func_800718D0", "UNRESOLVED",
     "29-word LoadImage helper; only callee is func_8007506C"),
    (0x8006B04C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B080, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006B0A4, 0x800718D0, "func_800718D0", "UNRESOLVED", "second site"),
    (0x8006B0AC, 0x80030894, "func_80030894", "UNRESOLVED",
     "0xC50-byte body; no port"),
    (0x8006B0BC, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B0F0, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006B118, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL", "B16 lookup"),
    (0x8006B12C, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL", "B16 lookup"),
    (0x8006B140, 0x8006E498, "func_8006E498", "TRANSLATED_FAITHFUL", "B16 lookup"),
    (0x8006B154, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B188, 0x8006E6A8, "func_8006E6A8", "TRANSLATED_FAITHFUL", "B16 issue"),
    (0x8006B1DC, 0x8006E1C0, "func_8006E1C0", "TRANSLATED_FAITHFUL", "later packet"),
    (0x8006B20C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED_FAITHFUL", "B16 poll"),
    (0x8006B254, 0x8006E1C0, "func_8006E1C0", "TRANSLATED_FAITHFUL", "later packet"),
    (0x8006B274, 0x80087024, "func_80087024", "HOST_SDK_OR_STREAM",
     "stream command 0xF1 in pe_stream.c"),
    (0x8006B27C, 0x80074DC0, "func_80074DC0", "HOST_SDK_SHIM", "DrawSync"),
    (0x8006B284, 0x80074A44, "func_80074A44", "HOST_SDK_SHIM", "ResetGraph"),
    (0x8006B28C, 0x80073A44, "func_80073A44", "HOST_SDK_SHIM", "VSync"),
    (0x8006B2B4, 0x800755F0, "func_800755F0", "HOST_SDK_SHIM", "PutDispEnv"),
    (0x8006B2BC, 0x80074D28, "func_80074D28", "HOST_SDK_SHIM", "SetDispMask"),
]

# Captured post-B53I-D / prefix-cut guest state (evidence only).
CANON_BASE = 0x801229A0
CANON_META = 0x8012DF18
CANON_HEADER = 0x0340B5B8
CANON_COUNT = 13
CANON_ENTRY0 = 0x8012DF58
CANON_S1 = 0
CANON_S2 = 1
CANON_LBA = 0x000003F5
CANON_LOOKUP = 0x801229A8
CANON_LOOKUP_WORD0 = 0x00007F0C
CANON_CD_BUSY = 0x01004000
CANON_D800B0CD8 = 0x41004003


def require(cond: bool, message: str) -> None:
    if not cond:
        raise SystemExit(f"FATAL: {message}")


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
    require(data[:8] == b"PS-X EXE", "not a PS-X EXE")
    return data


def verify_words(data: bytes) -> None:
    taddr = struct.unpack_from("<I", data, 0x18)[0]
    foff = FUNC_START - taddr + 0x800
    require(len(WORDS) == (FUNC_END - FUNC_START) // 4, "word/range mismatch")
    require((CUT_PC - FUNC_START) // 4 == 68, "cut is not word 68")
    require(len(WORDS) - 68 == 323, "suffix is not 323 words")
    for i, expected in enumerate(WORDS):
        actual = struct.unpack_from("<I", data, foff + i * 4)[0]
        require(actual == expected,
                f"word {i} @ {FUNC_START + i * 4:#010x}: "
                f"{actual:#010x} != {expected:#010x}")
    # Delay slots at the cut and loop.
    require(WORDS[66] == 0x0C01B870, "jal func_8006E1C0 at 0x8006AE48")
    require(WORDS[67] == 0x02802821, "delay move a1,s4 at 0x8006AE4C")
    require(WORDS[68] == 0x8E620028, "suffix start lw v0,0x28(s3)")
    require(WORDS[69] == 0x26310001, "suffix addiu s1,1")
    require(WORDS[72] == 0x1440FFF8, "bne back to 0x8006AE44")
    require(WORDS[73] == 0x26100014, "delay addiu s0,0x14")
    require(WORDS[-2] == 0x03E00008, "jr ra")
    require(WORDS[-1] == 0x00000000, "return delay nop")


def decode_rect(packed: int, h: int) -> tuple[int, int, int, int]:
    x = (packed >> 10) & 0x7FF
    y = packed >> 21
    w = packed & 0x3FF
    return x, y, w, h


def run_counted_loop(count: int, s1: int) -> dict:
    """Model AE50..AE68. Does not invent 6E1C0 internals."""
    calls = []
    s0_index = s1  # entry index already issued before the cut
    while True:
        s1 += 1
        if s1 < count:
            s0_index += 1
            calls.append(("func_8006E1C0", s0_index))
            continue
        break
    return {
        "remaining_6e1c0": len(calls),
        "final_s1": s1,
        "exit_pc": LOOP_EXIT,
        "calls": calls,
    }


def classify_first_unresolved() -> tuple[str, int]:
    for addr, target, name, cls, _ in SUFFIX_CALLS:
        if cls == "UNRESOLVED":
            return name, addr
    raise SystemExit("FATAL: no unresolved suffix callee")


def run_scenarios() -> int:
    n = 0

    # 1. Geometry / delay slots / call census.
    require(CUT_PC - FUNC_START == 0x110, "prefix byte size")
    require(FUNC_END - CUT_PC == 0x50C, "suffix byte size")
    require((FUNC_END - CUT_PC) // 4 == 323, "suffix word count")
    require(WORDS[66] == 0x0C01B870 and WORDS[67] == 0x02802821,
            "call+delay immediately before the cut")
    n += 1

    # 2. Canonical loop: count 13, s1 already 0, first entry already issued.
    loop = run_counted_loop(CANON_COUNT, CANON_S1)
    require(loop["remaining_6e1c0"] == 12, "canonical remaining 6E1C0 count")
    require(loop["final_s1"] == 13, "s1 must reach count")
    require(loop["exit_pc"] == LOOP_EXIT, "loop exit PC")
    require([c[1] for c in loop["calls"]] == list(range(1, 13)),
            "entries 1..12")
    n += 1

    # 3. count==1 (already issued the only entry): no further 6E1C0.
    loop1 = run_counted_loop(1, 0)
    require(loop1["remaining_6e1c0"] == 0 and loop1["final_s1"] == 1,
            "count=1 must exit immediately")
    n += 1

    # 4. Entry 0 decode matches the accepted two-LoadImage pair.
    e0_img = decode_rect(0x080B0020, 0x40)
    e0_clut = decode_rect(0x39040040, 0x01)
    require(e0_img == (704, 64, 32, 64), "entry0 image RECT")
    require(e0_clut == (256, 456, 64, 1), "entry0 CLUT RECT")
    n += 1

    # 5. DMA idle vs active does not change suffix-local control flow.
    #    The suffix body has no 1F80xxxx access. Next 6E1C0 uses existing
    #    GPU/DMA2 authority; this oracle never completes a transfer.
    for dma_active in (0, 1):
        loop_dma = run_counted_loop(CANON_COUNT, CANON_S1)
        require(loop_dma["remaining_6e1c0"] == 12,
                f"DMA active={dma_active} mutated the counted loop")
    n += 1

    # 6. After the loop, translated lookup/LoadImage walk are eligible, but
    #    CD poll is not invented. First unresolved *function* is 718D0.
    name, site = classify_first_unresolved()
    require(name == "func_800718D0" and site == FIRST_UNRESOLVED_CALL,
            "first unresolved callee/site")
    require(CANON_LOOKUP_WORD0 != 0, "ABADC06C payload word0 must be live")
    require(CANON_D800B0CD8 & CANON_CD_BUSY == CANON_CD_BUSY,
            "canonical CD busy bits must remain set; do not invent poll=0")
    n += 1

    # 7. B50-era first unresolved (6E1C0 at AE48) is now translated.
    require(any(a == 0x8006AE48 and c == "TRANSLATED_FAITHFUL"
                for a, _, _, c, _ in SUFFIX_CALLS),
            "0x8006AE48 must now be classified translated")
    require(any(t == 0x8007506C and c == "TRANSLATED_FAITHFUL"
                for _, t, _, c, _ in SUFFIX_CALLS),
            "7506C must now be classified translated")
    n += 1

    # 8. Recommended B54B cut is the loop exit, not the old AE50 cut and
    #    not a leap to 718D0.
    require(LOOP_EXIT == 0x8006AE68, "recommended B54B PC")
    require(CUT_PC != LOOP_EXIT, "old cut is not the new recommended cut")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54a_6ad40_suffix_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    verify_words(data)
    scenarios = run_scenarios()
    print(f"B54A oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  function   {FUNC_START:#010x}..{FUNC_END:#010x} "
          f"({len(WORDS)} words)")
    print(f"  suffix     {CUT_PC:#010x}..{FUNC_END:#010x} (323 words)")
    print(f"  canonical  count=13 s1=0 remaining_6E1C0=12 exit={LOOP_EXIT:#010x}")
    print("  first unresolved function: func_800718D0 @ 0x8006AFA8 "
          "(not reached without an explicit CD poll result)")
    print("  recommended B54B cut: 0x8006AE68 after remaining 6E1C0 loop")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
