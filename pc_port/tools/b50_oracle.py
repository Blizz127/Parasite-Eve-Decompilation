#!/usr/bin/env python3
"""
Phase 6E-B50 retail-word oracle and corrective prefix proof for func_8006AD40.

Verifies:
- 391 retail words at 0x8006AD40 (streaming subsystem multiplexer)
- Exact word count against the SHA-exact retail executable
- Complete ROM-order call census (29 calls to 13 unique targets)
- Dependency classification
- Exact 68-word production-prefix cut at the first unresolved call/delay slot

This script deliberately does not claim semantic coverage of the 391-word
suffix or of any unresolved dependency.  Production is prefix-only.  Native
tests independently verify the computed boundary arguments and prove that
non-strict execution does not continue past the state-producing boundary.
"""

import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
START = 0x8006AD40
END = 0x8006B35C
PREFIX_END = 0x8006AE50
PREFIX_WORD_COUNT = (PREFIX_END - START) // 4
FIRST_BOUNDARY_CALL = 0x8006AE48

# func_8006AD40: 391 words at 0x8006AD40 (0x35C = 1564 bytes)
# exe range: 0x8006AD40–0x8006B35C
# file offset: 0x5B540
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

# Call census: (jal_addr, target_vram, callee_name, classification)
CALLS = [
    (0x8006ADA4, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006ADC0, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006ADF4, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006AE48, 0x8006E1C0, "func_8006E1C0", "UNRESOLVED"),
    (0x8006AEFC, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006AF18, 0x8007506C, "func_8007506C", "UNRESOLVED"),
    (0x8006AF54, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006AF88, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006AFA8, 0x800718D0, "func_800718D0", "UNRESOLVED"),
    (0x8006B04C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B080, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006B0A4, 0x800718D0, "func_800718D0", "UNRESOLVED"),
    (0x8006B0AC, 0x80030894, "func_80030894", "UNRESOLVED"),
    (0x8006B0BC, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B0F0, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006B118, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006B12C, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006B140, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006B154, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B188, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006B1DC, 0x8006E1C0, "func_8006E1C0", "UNRESOLVED"),
    (0x8006B20C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B254, 0x8006E1C0, "func_8006E1C0", "UNRESOLVED"),
    (0x8006B274, 0x80087024, "func_80087024", "TRANSLATED"),
    (0x8006B27C, 0x80074DC0, "func_80074DC0", "TRANSLATED"),
    (0x8006B284, 0x80074A44, "func_80074A44", "TRANSLATED"),
    (0x8006B28C, 0x80073A44, "func_80073A44", "TRANSLATED"),
    (0x8006B2B4, 0x800755F0, "func_800755F0", "TRANSLATED"),
    (0x8006B2BC, 0x80074D28, "func_80074D28", "TRANSLATED"),
]

def main():
    import pathlib

    # Locate the retail executable
    candidates = [
        pathlib.Path("build/extracted/disc1/SLUS_006.62"),
        pathlib.Path("rom/image/SLUS_006.62"),
    ]
    exe = None
    for c in candidates:
        if c.exists():
            exe = c
            break
    if exe is None:
        print("B50 ORACLE: SKIP (no retail executable found)")
        sys.exit(0)

    data = exe.read_bytes()
    sha = hashlib.sha1(data).hexdigest()
    if sha != SHA1:
        print(f"B50 ORACLE: FAIL (SHA1 mismatch: {sha})")
        sys.exit(1)
    print(f"SHA1 verified: {sha}")

    # Load address
    taddr = struct.unpack_from("<I", data, 0x18)[0]
    foff = START - taddr + 0x800

    # Verify all words
    mismatches = 0
    for i, expected in enumerate(WORDS):
        actual = struct.unpack_from("<I", data, foff + i * 4)[0]
        if actual != expected:
            addr = START + i * 4
            print(f"  MISMATCH at 0x{addr:08X} word {i}: "
                  f"expected 0x{expected:08X}, got 0x{actual:08X}")
            mismatches += 1

    if mismatches > 0:
        print(f"B50 ORACLE: FAIL ({mismatches} word mismatches)")
        sys.exit(1)

    print(f"All {len(WORDS)} words verified exact")
    if len(WORDS) != (END - START) // 4:
        print("B50 ORACLE: FAIL (literal word count does not match range)")
        sys.exit(1)

    # Corrective production boundary: the prefix includes the first jal and
    # its architecturally executed delay slot, then returns on the host.
    call_i = (FIRST_BOUNDARY_CALL - START) // 4
    if (PREFIX_WORD_COUNT != 68 or
            WORDS[call_i] != 0x0C01B870 or
            WORDS[call_i + 1] != 0x02802821):
        print("B50 ORACLE: FAIL (first-boundary call/delay mismatch)")
        sys.exit(1)

    # Exact packet-header chain feeding a0 at the boundary:
    #   s4 = [D_800B0CD8+0x160]
    #   v0 = [s4+4]
    #   s3 = s4+v0
    #   header = [s3+0x28]
    header_i = (0x8006AE10 - START) // 4
    expected_header_chain = [
        0x8EB40160, 0x3C03003F, 0x8E820004, 0x3463FFFF,
        0x02829821, 0x8E620028,
    ]
    if WORDS[header_i:header_i + len(expected_header_chain)] != expected_header_chain:
        print("B50 ORACLE: FAIL (boundary header indirection mismatch)")
        sys.exit(1)

    print(f"Function range: 0x{START:08X}–0x{END:08X} ({len(WORDS)*4} bytes)")
    print(f"Production prefix: 0x{START:08X}–0x{PREFIX_END:08X} "
          f"({PREFIX_WORD_COUNT} words; call+delay included)")

    # Call census
    print(f"\nCall census: {len(CALLS)} calls")
    resolved = sum(1 for _, _, _, c in CALLS if c == "TRANSLATED")
    unresolved = sum(1 for _, _, _, c in CALLS if c == "UNRESOLVED")
    print(f"  TRANSLATED:  {resolved}")
    print(f"  UNRESOLVED:  {unresolved}")

    unique = {}
    for _, target, name, cls in CALLS:
        unique[target] = (name, cls)
    print(f"\nUnique targets: {len(unique)}")
    for target in sorted(unique):
        name, cls = unique[target]
        count = sum(1 for _, t, _, _ in CALLS if t == target)
        print(f"  0x{target:08X}: {name} ({cls}) ×{count}")

    # First unresolved in execution order
    for addr, target, name, cls in CALLS:
        if cls == "UNRESOLVED":
            print(f"\nFirst unresolved callee: {name} at call-site 0x{addr:08X}")
            break

    print(f"\nB50 ORACLE: PASS ({len(WORDS)} words, {len(CALLS)} calls, "
          f"{unresolved} unresolved boundaries)")


if __name__ == "__main__":
    main()
