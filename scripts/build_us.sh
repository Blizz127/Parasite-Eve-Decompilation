#!/usr/bin/env bash
# Phase 5EH: Disc 1 rebuild with 207 C leaves (delay-slot sw family + era + first arg+return leaf)
# (prior 98 + 5 memset/memcpy countdown leaves through func_8008D820).
#
# Assembles splat-generated .s → .o with MIPS LE binutils, compiles the
# production C units with documented GCC flags, links in ROM order, packs a
# PS-X EXE-sized candidate, and compares SHA-1 to the original.
#
# Toolchain (documented Phase 4G / 4J):
#   Distrobox container: pe-mipsel  (Debian trixie)
#   binutils-mipsel-linux-gnu 2.44: mipsel-linux-gnu-{as,ld,objcopy,objdump,readelf}
#   gcc-mipsel-linux-gnu 14.2.0:    mipsel-linux-gnu-gcc
#   Assembler flags: -EL -mips1 -mabi=32 -I include/
#   C flags (Phase 4J exact match for this leaf):
#     -EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls
#     -ffreestanding -fno-builtin -O1
#
# Exit codes:
#   0  exact SHA-1 match against original SLUS_006.62
#   1  real failure, or rebuild produced a non-matching candidate
#   2  usage / missing prerequisites
#
# Usage:
#   scripts/build_us.sh              # assemble + compile + link + pack + compare
#   scripts/build_us.sh --assemble-only
#   scripts/build_us.sh --help
#
# Does NOT claim matching unless compare succeeds.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

MODE="full"
if [[ $# -gt 0 ]]; then
    case "$1" in
        --assemble-only) MODE="assemble" ;;
        --help|-h)
            sed -n '2,28p' "$0" | sed 's/^# \?//'
            exit 0
            ;;
        *)
            echo "Usage: $0 [--assemble-only|--help]" >&2
            exit 2
            ;;
    esac
fi

# --- constants ---
EXE="$ROOT/build/extracted/disc1/SLUS_006.62"
EXPECTED_SHA1="452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
T_SIZE=0x1EE000
HEADER_SIZE=0x800
ASFLAGS_DEFAULT="-EL -mips1 -mabi=32"
# Phase 4J: exact 5-instruction match for func_80090C38 with these flags.
CFLAGS_LEAF="-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestanding -fno-builtin -O1"

# Phase 5DB file-span sizes (config subsegment edges; exclusive end).
# 2A0C:     0x2A0C  → 0x869C  = 0x5C90
# C 17E9C:  0x869C  → 0x86A4  = 0x8
# 86A4:     0x86A4  → 0x87DC  = 0x138
# C 17FDC:  0x87DC  → 0x87F0  = 0x14
# C 17FF0:  0x87F0  → 0x8804  = 0x14
# 8804:     0x8804  → 0x9850  = 0x104C
# C 19050:  0x9850  → 0x9858  = 0x8
# C 19058:  0x9858  → 0x9860  = 0x8
# 9860:     0x9860  → 0x98AC  = 0x4C
# C 190AC:  0x98AC  → 0x98B4  = 0x8
# C 190B4:  0x98B4  → 0x98BC  = 0x8
# 98BC:     0x98BC  → 0x9AB8  = 0x1FC
# C 192B8:  0x9AB8  → 0x9AC8  = 0x10
# C 192C8:  0x9AC8  → 0x9ADC  = 0x14
# 9ADC:     0x9ADC  → 0xA3E4  = 0x908
# C 38D0C:  0x2950C → 0x2951C = 0x10
# 2951C:    0x2951C → 0x2E02C = 0x4B10
# C 3D82C:  0x2E02C → 0x2E034 = 0x8
# 2E034:    0x2E034 → 0x2E7C8 = 0x794
# C 3DFC8:  0x2E7C8 → 0x2E7D0 = 0x8
# 2E7D8:    0x2E7D8 → 0x307AC = 0x1FD4
# C 3FFAC:  0x307AC → 0x307BC = 0x10
# C 3FFBC:  0x307BC → 0x307CC = 0x10
# 307CC:    0x307CC → 0x330C4 = 0x28F8
# C 428C4:  0x330C4 → 0x330D4 = 0x10
# 330D4:    0x330D4 → 0x33110 = 0x3C
# C 42910:  0x33110 → 0x33128 = 0x18
# 33128:    0x33128 → 0x33328 = 0x200
# C 42B28:  0x33328 → 0x33338 = 0x10
# C 42B38:  0x33338 → 0x33350 = 0x18
# C 42B50:  0x33350 → 0x3336C = 0x1C
# C 42B6C:  0x3336C → 0x333C8 = 0x5C
# C 42BC8:  0x333C8 → 0x333D8 = 0x10
# C 42BD8..42C64: 0x333D8 → 0x33478 = 8×0x14 (opaque-word batch)
# 33478:    0x33478 → 0x334B8 = 0x40
# C 42CB8:  0x334B8 → 0x334C4 = 0xC
# C 4DA9C:  0x3E29C → 0x3E2A4 = 0x8
# 3E2A4:    0x3E2A4 → 0x41518 = 0x3274
# C 50D18:  0x41518 → 0x41520 = 0x8
# 41520:    0x41520 → 0x42034 = 0xB14
# C 51834:  0x42034 → 0x4204C = 0x18
# 4204C:    0x4204C → 0x42648 = 0x5FC
# C 51E48:  0x42648 → 0x42658 = 0x10
# 42658:    0x42658 → 0x42D14 = 0x6BC
# C 52514:  0x42D14 → 0x42D24 = 0x10
# C 52524:  0x42D24 → 0x42D34 = 0x10
# 42D34:    0x42D34 → 0x42D7C = 0x48
# C 5257C:  0x42D7C → 0x42D94 = 0x18
# 42D94:    0x42D94 → 0x42FC0 = 0x22C
# C 527C0:  0x42FC0 → 0x42FC8 = 0x8
# 42FC8:    0x42FC8 → 0x4C4A8 = 0x94E0
# C 5BCA8:  0x4C4A8 → 0x4C4B0 = 0x8
# 4C4B0:    0x4C4B0 → 0x4F084 = 0x2BD4
# C 5E884:  0x4F084 → 0x4F094 = 0x10
# 4F094:    0x4F094 → 0x5F3D4 = 0x10340
# C 6EBD4:  0x5F3D4 → 0x5F3E4 = 0x10
# 5F3E4:    0x5F3E4 → 0x645E8 = 0x5204
# C 73DE8:  0x645E8 → 0x645F8 = 0x10
# C 73DF8:  0x645F8 → 0x64610 = 0x18
# 64610:    0x64610 → 0x64B30 = 0x520
# C 74A14:  0x65214 → 0x65228 = 0x14
# C 74A28:  0x65228 → 0x65238 = 0x10
# 65238:    0x65238 → 0x654B8 = 0x280
# C 74CB8:  0x654B8 → 0x654C8 = 0x10
# 654C8:    0x654C8 → 0x66B3C = 0x1674
# C 7633C:  0x66B3C → 0x66B54 = 0x18
# 66B54:    0x66B54 → 0x6AB24 = 0x3FD0
# C 7A324:  0x6AB24 → 0x6AB34 = 0x10
# C 7A334:  0x6AB34 → 0x6AB44 = 0x10
# C 7A344:  0x6AB44 → 0x6AB54 = 0x10
# C 7A354:  0x6AB54 → 0x6AB60 = 0xC
# 6AB60:    0x6AB60 → 0x6ABEC = 0x8C
# C 7A3EC:  0x6ABEC → 0x6AC00 = 0x14
# 6AC00:    0x6AC00 → 0x6ACA8 = 0xA8
# C 7A4A8:  0x6ACA8 → 0x6ACBC = 0x14
# C 7A4BC:  0x6ACBC → 0x6ACD0 = 0x14
# 6ACD0:    0x6ACD0 → 0x6C930 = 0x1C60
# C 7C130:  0x6C930 → 0x6C93C = 0xC
# 6C93C:    0x6C93C → 0x6E6A4 = 0x1D68
# C 7DEA4:  0x6E6A4 → 0x6E6B0 = 0xC
# C 7DEB0:  0x6E6B0 → 0x6E6C0 = 0x10
# 6E6C0:    0x6E6C0 → 0x6FF78 = 0x18B8
# C 7F778:  0x6FF78 → 0x6FF88 = 0x10
# 6FF88:    0x6FF88 → 0x703C0 = 0x438
# C 7FBC0:  0x703C0 → 0x703CC = 0xC
# C 7FBCC:  0x703CC → 0x703D8 = 0xC
# C 7FBD8:  0x703D8 → 0x703E4 = 0xC
# C 7FBE4:  0x703E4 → 0x703F0 = 0xC
# 703F0:    0x703F0 → 0x70408 = 0x18
# C 7FC08:  0x70408 → 0x70418 = 0x10
# C 7FC18:  0x70418 → 0x70428 = 0x10
# C 7FC28:  0x70428 → 0x70434 = 0xC
# C 7FC34:  0x70434 → 0x70444 = 0x10
# C 7FC44:  0x70444 → 0x70454 = 0x10
# C 7FC54:  0x70454 → 0x70464 = 0x10
# 70464:    0x70464 → 0x704AC = 0x48
# C 7FCAC:  0x704AC → 0x704BC = 0x10
# 704BC:    0x704BC → 0x71130 = 0xC74
# C 80930:  0x71130 → 0x71140 = 0x10
# C 80940:  0x71140 → 0x71150 = 0x10
# 71150:    0x71150 → 0x714C8 = 0x378
# C 80CC8:  0x714C8 → 0x714DC = 0x14
# 714DC:    0x714DC → 0x71A54 = 0x578
# C 81254:  0x71A54 → 0x71A68 = 0x14
# 71A68:    0x71A68 → 0x72AAC = 0x1044
# C 822AC:  0x72AAC → 0x72ABC = 0x10
# 72ABC:    0x72ABC → 0x734DC = 0xA20
# C 82CDC:  0x734DC → 0x734F0 = 0x14
# 734F0:    0x734F0 → 0x73DA4 = 0x8B4
# C 870E0:  0x778E0 → 0x778F0 = 0x10
# 778F0:    0x778F0 → 0x77998 = 0xA8
# C 87198:  0x77998 → 0x779AC = 0x14
# 779AC:    0x779AC → 0x77C14 = 0x268
# C 87414:  0x77C14 → 0x77C28 = 0x14
# 77C28:    0x77C28 → 0x7B31C = 0x36F4
# C 8AB1C:  0x7B31C → 0x7B39C = 0x80
# 7B39C:    0x7B39C → 0x7D27C = 0x1EE0
# C 8CA7C:  0x7D27C → 0x7D284 = 0x8
# 7D284:    0x7D284 → 0x7DFC0 = 0xD3C
# C 8D7C0:  0x7DFC0 → 0x7DFD0 = 0x10
# 7DFD0:    0x7DFD0 → 0x7FE94 = 0x1EC4
# C 8F694:  0x7FE94 → 0x7FEA8 = 0x14
# C 8F6A8:  0x7FEA8 → 0x7FEB0 = 0x8
# 7FEB0:    0x7FEB0 → 0x80068 = 0x1B8
# C 8F868:  0x80068 → 0x80080 = 0x18
# C 8F880:  0x80080 → 0x80098 = 0x18
# 80098:    0x80098 → 0x804B4 = 0x41C
# C 8FCB4:  0x804B4 → 0x804BC = 0x8
# 804BC:    0x804BC → 0x80CA0 = 0x7E4
# C 904A0:  0x80CA0 → 0x80CAC = 0xC
# C 904AC:  0x80CAC → 0x80CB4 = 0x8
# C 904B4:  0x80CB4 → 0x80CBC = 0x8
# C 904BC:  0x80CBC → 0x80CC4 = 0x8
# 80CC4:    0x80CC4 → 0x80EB4 = 0x1F0
# C 906B4:  0x80EB4 → 0x80EE4 = 0x30
# 80EE4:    0x80EE4 → 0x8120C = 0x328
# C 90A0C:  0x8120C → 0x81220 = 0x14
# 81220:    0x81220 → 0x81438 = 0x218
# C 90C38:  0x81438 → 0x8144C = 0x14
# C 90C4C:  0x8144C → 0x81460 = 0x14
# C 90C60:  0x81460 → 0x81474 = 0x14
# C 90C74:  0x81474 → 0x81488 = 0x14
# 81488:    0x81488 → 0x81754 = 0x2CC
# C 90F54:  0x81754 → 0x81768 = 0x14
# 81768:    0x81768 → 0x818A0 = 0x138
# 800.rodata: 0x800 → 0x2A0C = 0x220C
# B2AF8:    0xB2AF8 → 0xB3340 = 0x848
# C C2B40:  0xB3340 → 0xB3350 = 0x10
# C C2B50:  0xB3350 → 0xB3368 = 0x18
# B3368:    0xB3368 → 0xB85C4 = 0x525C
# C C7DC4:  0xB85C4 → 0xB85D4 = 0x10
# C C7DD4:  0xB85D4 → 0xB85DC = 0x8
# C C7DDC:  0xB85DC → 0xB85E4 = 0x8
# B85E4:    0xB85E4 → 0xB8A68 = 0x484
# C C8268:  0xB8A68 → 0xB8A70 = 0x8
# B8A70:    0xB8A70 → 0xB9A60 = 0xFF0
# B93C0:    0xB93C0 → 0xB9708 = 0x348
# C C8F08:  0xB9708 → 0xB9718 = 0x10
# C C8F18:  0xB9718 → 0xB9720 = 0x8
# C C8F20:  0xB9720 → 0xB9728 = 0x8
# B9728:    0xB9728 → 0xB9A60 = 0x338
# C C9260:  0xB9A60 → 0xB9A68 = 0x8
# B9A68:    0xB9A68 → 0xBA6A0 = 0xC38
# C C9EA0:  0xBA6A0 → 0xBA6A8 = 0x8
# BA6A8:    0xBA6A8 → 0xBB4D4 = 0xE2C
# C CACD4:  0xBB4D4 → 0xBB4DC = 0x8
# BB4DC:    0xBB4DC → 0xBDADC = 0x2600
# C CD2DC:  0xBDADC → 0xBDAE4 = 0x8
# C CD2E4:  0xBDAE4 → 0xBDAEC = 0x8
# BDAEC:    0xBDAEC → 0xBDD9C = 0x2B0
# C CD59C:  0xBDD9C → 0xBDDA4 = 0x8
# BDDA4:    0xBDDA4 → 0xBE504 = 0x760
# C CDD04:  0xBE504 → 0xBE50C = 0x8
# BE50C:    0xBE50C → 0xBEBAC = 0x6A0
# C CE3AC:  0xBEBAC → 0xBEBB4 = 0x8
# BEBB4:    0xBEBB4 → 0x1EE800 = 0x12FC4C
# 818A0.rodata: 0x818A0 → 0xB2AF8 = 0x31258
SIZE_800_RODATA=0x220c
SIZE_C_LEAF=0x14
SIZE_800_RODATA=0x220c
SIZE_2A0C=0x5c90
SIZE_C_17E9C=0x8
SIZE_86A4=0x138
SIZE_C_17FDC=0x14
SIZE_C_17FF0=0x14
SIZE_8804=0x104c
SIZE_C_19050=0x8
SIZE_C_19058=0x8
SIZE_9860=0x4c
SIZE_C_190AC=0x8
SIZE_C_190B4=0x8
SIZE_98BC=0x1fc
SIZE_C_192B8=0x10
SIZE_C_192C8=0x14
SIZE_9ADC=0x908
SIZE_C_19BE4=0x20
SIZE_A404=0x72f8
SIZE_C_20EFC=0x1c
SIZE_11718=0x12b08
SIZE_C_33A20=0xc
SIZE_2422C=0x3778
SIZE_C_371A4=0xc
SIZE_279B0=0x2a4
SIZE_C_37454=0x18
SIZE_27C6C=0x148
SIZE_C_375B4=0x10
SIZE_C_375C4=0xc
SIZE_C_375D0=0x10
SIZE_27DE0=0x284
SIZE_C_37864=0xc
SIZE_28070=0x10d0
SIZE_C_38940=0x14
SIZE_29154=0x3b8
SIZE_C_38D0C=0x10
SIZE_2951C=0x4b10
SIZE_C_3D82C=0x8
SIZE_2E034=0x794
SIZE_C_3DFC8=0x8
SIZE_C_3DFD0=0x8
SIZE_2E7D8=0x1fd4
SIZE_C_3FFAC=0x10
SIZE_C_3FFBC=0x10
SIZE_307CC=0x28f8
SIZE_C_428C4=0x10
SIZE_330D4=0x3c
SIZE_C_42910=0x18
SIZE_33128=0x200
SIZE_C_42B28=0x10
SIZE_C_42B38=0x18
SIZE_C_42B50=0x1c
SIZE_C_42B6C=0x5c
SIZE_C_42BC8=0x10
SIZE_C_42BD8=0x14
SIZE_C_42BEC=0x14
SIZE_C_42C00=0x14
SIZE_C_42C14=0x14
SIZE_C_42C28=0x14
SIZE_C_42C3C=0x14
SIZE_C_42C50=0x14
SIZE_C_42C64=0x14
SIZE_33478=0x40
SIZE_C_42CB8=0xc
SIZE_334C4=0x20c
SIZE_C_42ED0=0xc
SIZE_336DC=0x44
SIZE_C_42F20=0x18
SIZE_C_42F38=0xc
SIZE_33744=0xf4
SIZE_C_43038=0x14
SIZE_3384C=0x1f4
SIZE_C_43240=0xc
SIZE_33A4C=0x674
SIZE_C_438C0=0x20
SIZE_C_438E0=0xc
SIZE_340EC=0x9738
SIZE_C_4D024=0xc
SIZE_3D830=0x24c
SIZE_C_4D27C=0xc
SIZE_C_4D288=0x10
SIZE_3DA98=0x804
SIZE_C_4DA9C=0x8
SIZE_3E2A4=0xecc
SIZE_C_4E970=0xc
SIZE_3F17C=0xacc
SIZE_C_4F448=0x1c
SIZE_3FC64=0x3a4
SIZE_C_4F808=0x30
SIZE_40038=0x14e0
SIZE_C_50D18=0x8
SIZE_41520=0x364
SIZE_C_51084=0x14
SIZE_41898=0x1ac
SIZE_C_51244=0x14
SIZE_41A58=0x2a0
SIZE_C_514F8=0xc
SIZE_C_51504=0xc
SIZE_41D10=0x324
SIZE_C_51834=0x18
SIZE_4204C=0x5fc
SIZE_C_51E48=0x10
SIZE_C_51E58=0xc
SIZE_42664=0x6b0
SIZE_C_52514=0x10
SIZE_C_52524=0x10
SIZE_42D34=0x48
SIZE_C_5257C=0x18
SIZE_42D94=0x220
SIZE_C_527B4=0xc
SIZE_C_527C0=0x8
SIZE_42FC8=0x6e8
SIZE_C_52EB0=0x10
SIZE_436C0=0x4c
SIZE_C_52F0C=0x18
SIZE_43724=0x1364
SIZE_C_54288=0xc
SIZE_C_54294=0xc
SIZE_44AA0=0x3c2c
SIZE_C_57ECC=0xc
SIZE_486D8=0x39b8
SIZE_C_5B890=0xc
SIZE_C_5B89C=0xc
SIZE_4C0A8=0x3f0
SIZE_C_5BC98=0x10
SIZE_C_5BCA8=0x8
SIZE_C_5BCB0=0xc
SIZE_4C4BC=0x220
SIZE_C_5BEDC=0xc
SIZE_4C6E8=0x5a0
SIZE_C_5C488=0x10
SIZE_4CC98=0x1c7c
SIZE_C_5E114=0xc
SIZE_C_5E120=0xc
SIZE_4E92C=0x450
SIZE_C_5E57C=0xc
SIZE_4ED88=0x15c
SIZE_C_5E6E4=0xc
SIZE_4EEF0=0x194
SIZE_C_5E884=0x10
SIZE_C_5E894=0x10
SIZE_4F0A4=0x2b4
SIZE_C_5EB58=0xc
SIZE_4F364=0x364
SIZE_C_5EEC8=0xc
SIZE_4F6D4=0x25cc
SIZE_C_614A0=0xc
SIZE_51CAC=0xe04
SIZE_C_622B0=0xc
SIZE_52ABC=0x6f4
SIZE_C_629B0=0xc
SIZE_531BC=0x2fc
SIZE_C_62CB8=0xc
SIZE_C_62CC4=0xc
SIZE_C_62CD0=0x14
SIZE_534E4=0x4b4
SIZE_C_63198=0x14
SIZE_C_631AC=0x14
SIZE_539C0=0x1888
SIZE_C_64A48=0xc
SIZE_55254=0x1cc
SIZE_C_64C20=0x10
SIZE_55430=0x9fa4
SIZE_C_6EBD4=0x10
SIZE_5F3E4=0x5204
SIZE_C_73DE8=0x10
SIZE_C_73DF8=0x18
SIZE_64610=0x520
SIZE_C_74330=0x24
SIZE_64B54=0x150
SIZE_C_744A4=0x24
SIZE_64CC8=0x284
SIZE_C_7474C=0x24
SIZE_64F70=0x2a4
SIZE_C_74A14=0x14
SIZE_C_74A28=0x10
SIZE_65238=0x280
SIZE_C_74CB8=0x10
SIZE_654C8=0x1674
SIZE_C_7633C=0x18
SIZE_66B54=0x16d4
SIZE_C_77A28=0x24
SIZE_6824C=0x118
SIZE_C_77B64=0x14
SIZE_68378=0xc
SIZE_C_77B84=0x14
SIZE_68398=0xc
SIZE_C_77BA4=0x14
SIZE_683B8=0xc
SIZE_C_77BC4=0x14
SIZE_683D8=0xc
SIZE_C_77BE4=0x14
SIZE_683F8=0xc
SIZE_C_77C04=0x14
SIZE_68418=0xc
SIZE_C_77C24=0x14
SIZE_68438=0xc
SIZE_C_77C44=0x14
SIZE_68458=0xc
SIZE_C_77C64=0x14
SIZE_68478=0x26ac
SIZE_C_7A324=0x10
SIZE_C_7A334=0x10
SIZE_C_7A344=0x10
SIZE_C_7A354=0xc
SIZE_6AB60=0x8c
SIZE_C_7A3EC=0x14
SIZE_6AC00=0xa8
SIZE_C_7A4A8=0x14
SIZE_C_7A4BC=0x14
SIZE_6ACD0=0x1c60
SIZE_C_7C130=0xc
SIZE_6C93C=0x1d68
SIZE_C_7DEA4=0xc
SIZE_C_7DEB0=0x10
SIZE_6E6C0=0x18b8
SIZE_C_7F778=0x10
SIZE_6FF88=0x438
SIZE_C_7FBC0=0xc
SIZE_C_7FBCC=0xc
SIZE_C_7FBD8=0xc
SIZE_C_7FBE4=0xc
SIZE_703F0=0x18
SIZE_C_7FC08=0x10
SIZE_C_7FC18=0x10
SIZE_C_7FC28=0xc
SIZE_C_7FC34=0x10
SIZE_C_7FC44=0x10
SIZE_C_7FC54=0x10
SIZE_70464=0x48
SIZE_C_7FCAC=0x10
SIZE_704BC=0xc74
SIZE_C_80930=0x10
SIZE_C_80940=0x10
SIZE_71150=0x378
SIZE_C_80CC8=0x14
SIZE_714DC=0x578
SIZE_C_81254=0x14
SIZE_71A68=0x1044
SIZE_C_822AC=0x10
SIZE_72ABC=0xa20
SIZE_C_82CDC=0x14
SIZE_734F0=0x8b4
SIZE_C_835A4=0xc
SIZE_C_835B0=0x10
SIZE_73DC0=0x8b0
SIZE_C_83E70=0x14
SIZE_74684=0x60
SIZE_C_83EE4=0x14
SIZE_746F8=0x8a8
SIZE_C_847A0=0x10
SIZE_74FB0=0xf78
SIZE_C_85728=0x1c
SIZE_75F44=0x199c
SIZE_C_870E0=0x10
SIZE_778F0=0xa8
SIZE_C_87198=0x14
SIZE_779AC=0x268
SIZE_C_87414=0x14
SIZE_77C28=0x36f4
SIZE_C_8AB1C=0x80
SIZE_7B39C=0x1ee0
SIZE_C_8CA7C=0x8
SIZE_7D284=0xd3c
SIZE_C_8D7C0=0x10
SIZE_7DFD0=0x50
SIZE_C_8D820=0x24
SIZE_7E044=0x1e50
SIZE_C_8F694=0x14
SIZE_C_8F6A8=0x8
SIZE_7FEB0=0x1b8
SIZE_C_8F868=0x18
SIZE_C_8F880=0x18
SIZE_80098=0x41c
SIZE_C_8FCB4=0x8
SIZE_804BC=0x7e4
SIZE_C_904A0=0xc
SIZE_C_904AC=0x8
SIZE_C_904B4=0x8
SIZE_C_904BC=0x8
SIZE_80CC4=0x1f0
SIZE_C_906B4=0x30
SIZE_80EE4=0x328
SIZE_C_90A0C=0x14
SIZE_81220=0x218
SIZE_C_90C38=0x14
SIZE_C_90C4C=0x14
SIZE_C_90C60=0x14
SIZE_C_90C74=0x14
SIZE_81488=0x2cc
SIZE_C_90F54=0x14
SIZE_81768=0x138
SIZE_818A0_RODATA=0x31258
SIZE_B2AF8=0x848
SIZE_C_C2B40=0x10
SIZE_C_C2B50=0x18
SIZE_B3368=0x525c
SIZE_C_C7DC4=0x10
SIZE_C_C7DD4=0x8
SIZE_C_C7DDC=0x8
SIZE_B85E4=0x484
SIZE_C_C8268=0x8
SIZE_B8A70=0x944
SIZE_C_C8BB4=0xc
SIZE_B93C0=0x348
SIZE_C_C8F08=0x10
SIZE_C_C8F18=0x8
SIZE_C_C8F20=0x8
SIZE_B9728=0x338
SIZE_C_C9260=0x8
SIZE_B9A68=0x700
SIZE_C_C9968=0xc
SIZE_BA174=0x28c
SIZE_C_C9C00=0x10
SIZE_BA410=0x290
SIZE_C_C9EA0=0x8
SIZE_BA6A8=0x600
SIZE_C_CA4A8=0xc
SIZE_BACB4=0x2e4
SIZE_C_CA798=0x10
SIZE_BAFA8=0x52c
SIZE_C_CACD4=0x8
SIZE_BB4DC=0xe48
SIZE_C_CBB24=0xc
SIZE_BC330=0x474
SIZE_C_CBFA4=0x10
SIZE_BC7B4=0xfcc
SIZE_C_CCF80=0x10
SIZE_BD790=0x34c
SIZE_C_CD2DC=0x8
SIZE_C_CD2E4=0x8
SIZE_BDAEC=0x2b0
SIZE_C_CD59C=0x8
SIZE_C_CD5A4=0xc
SIZE_BDDB0=0x16c
SIZE_C_CD71C=0xc
SIZE_BDF28=0x238
SIZE_C_CD960=0x10
SIZE_BE170=0x394
SIZE_C_CDD04=0x8
SIZE_BE50C=0x234
SIZE_C_CDF40=0xc
SIZE_BE74C=0x290
SIZE_C_CE1DC=0x10
SIZE_BE9EC=0x1c0
SIZE_C_CE3AC=0x8
SIZE_BEBB4=0xb0
SIZE_C_CE464=0xc
SIZE_BEC70=0x63e0
SIZE_C_D4850=0x10
SIZE_C5060=0x1297a0
SIZE_B2AF8=0x848
SIZE_C_C2B40=0x10
SIZE_C_C2B50=0x18
SIZE_B3368=0x525c
SIZE_C_C7DC4=0x10
SIZE_C_C7DD4=0x8
SIZE_C_C7DDC=0x8
SIZE_B85E4=0x484
SIZE_C_C8268=0x8
SIZE_B8A70=0x944
SIZE_C_C8BB4=0xc
SIZE_B93C0=0x348
SIZE_C_C8F08=0x10
SIZE_C_C8F18=0x8
SIZE_C_C8F20=0x8
SIZE_B9728=0x338
SIZE_C_C9260=0x8
SIZE_B9A68=0x700
SIZE_C_C9968=0xc
SIZE_BA174=0x28c
SIZE_C_C9C00=0x10
SIZE_BA410=0x290
SIZE_C_C9EA0=0x8
SIZE_BA6A8=0x600
SIZE_C_CA4A8=0xc
SIZE_BACB4=0x2e4
SIZE_C_CA798=0x10
SIZE_BAFA8=0x52c
SIZE_C_CACD4=0x8
SIZE_BB4DC=0xe48
SIZE_C_CBB24=0xc
SIZE_BC330=0x474
SIZE_C_CBFA4=0x10
SIZE_BC7B4=0xfcc
SIZE_C_CCF80=0x10
SIZE_BD790=0x34c
SIZE_C_CD2DC=0x8
SIZE_C_CD2E4=0x8
SIZE_BDAEC=0x2b0
SIZE_C_CD59C=0x8
SIZE_C_CD5A4=0xc
SIZE_BDDB0=0x16c
SIZE_C_CD71C=0xc
SIZE_BDF28=0x238
SIZE_C_CD960=0x10
SIZE_BE170=0x394
SIZE_C_CDD04=0x8
SIZE_BE50C=0x234
SIZE_C_CDF40=0xc
SIZE_BE74C=0x290
SIZE_C_CE1DC=0x10
SIZE_BE9EC=0x1c0
SIZE_C_CE3AC=0x8
SIZE_BEBB4=0xb0
SIZE_C_CE464=0xc
SIZE_BEC70=0x63e0
SIZE_C_D4850=0x10
SIZE_C5060=0x1297a0

# Object paths (ROM-order units; splat ld is still C-layout and unused for link).
OBJECTS=(
    "build/asm/disc1/header.s.o"
    "build/asm/disc1/data/800.rodata.s.o"
    "build/asm/disc1/2A0C.s.o"
    "build/src/func_80017E9C.c.o"
    "build/asm/disc1/86A4.s.o"
    "build/src/func_80017FDC.c.o"
    "build/src/func_80017FF0.c.o"
    "build/asm/disc1/8804.s.o"
    "build/src/func_80019050.c.o"
    "build/src/func_80019058.c.o"
    "build/asm/disc1/9860.s.o"
    "build/src/func_800190AC.c.o"
    "build/src/func_800190B4.c.o"
    "build/asm/disc1/98BC.s.o"
    "build/src/func_800192B8.c.o"
    "build/src/func_800192C8.c.o"
    "build/asm/disc1/9ADC.s.o"
    "build/src/func_80019BE4.c.o"
    "build/asm/disc1/A404.s.o"
    "build/src/func_80020EFC.c.o"
    "build/asm/disc1/11718.s.o"
    "build/src/func_80033A20.c.o"
    "build/asm/disc1/2422C.s.o"
    "build/src/func_800371A4.c.o"
    "build/asm/disc1/279B0.s.o"
    "build/src/func_80037454.c.o"
    "build/asm/disc1/27C6C.s.o"
    "build/src/func_800375B4.c.o"
    "build/src/func_800375C4.c.o"
    "build/src/func_800375D0.c.o"
    "build/asm/disc1/27DE0.s.o"
    "build/src/func_80037864.c.o"
    "build/asm/disc1/28070.s.o"
    "build/src/func_80038940.c.o"
    "build/asm/disc1/29154.s.o"
    "build/src/func_80038D0C.c.o"
    "build/asm/disc1/2951C.s.o"
    "build/src/func_8003D82C.c.o"
    "build/asm/disc1/2E034.s.o"
    "build/src/func_8003DFC8.c.o"
    "build/src/func_8003DFD0.c.o"
    "build/asm/disc1/2E7D8.s.o"
    "build/src/func_8003FFAC.c.o"
    "build/src/func_8003FFBC.c.o"
    "build/asm/disc1/307CC.s.o"
    "build/src/func_800428C4.c.o"
    "build/asm/disc1/330D4.s.o"
    "build/src/func_80042910.c.o"
    "build/asm/disc1/33128.s.o"
    "build/src/func_80042B28.c.o"
    "build/src/func_80042B38.c.o"
    "build/src/func_80042B50.c.o"
    "build/src/func_80042B6C.c.o"
    "build/src/func_80042BC8.c.o"
    "build/src/func_80042BD8.c.o"
    "build/src/func_80042BEC.c.o"
    "build/src/func_80042C00.c.o"
    "build/src/func_80042C14.c.o"
    "build/src/func_80042C28.c.o"
    "build/src/func_80042C3C.c.o"
    "build/src/func_80042C50.c.o"
    "build/src/func_80042C64.c.o"
    "build/asm/disc1/33478.s.o"
    "build/src/func_80042CB8.c.o"
    "build/asm/disc1/334C4.s.o"
    "build/src/func_80042ED0.c.o"
    "build/asm/disc1/336DC.s.o"
    "build/src/func_80042F20.c.o"
    "build/src/func_80042F38.c.o"
    "build/asm/disc1/33744.s.o"
    "build/src/func_80043038.c.o"
    "build/asm/disc1/3384C.s.o"
    "build/src/func_80043240.c.o"
    "build/asm/disc1/33A4C.s.o"
    "build/src/func_800438C0.c.o"
    "build/src/func_800438E0.c.o"
    "build/asm/disc1/340EC.s.o"
    "build/src/func_8004D024.c.o"
    "build/asm/disc1/3D830.s.o"
    "build/src/func_8004D27C.c.o"
    "build/src/func_8004D288.c.o"
    "build/asm/disc1/3DA98.s.o"
    "build/src/func_8004DA9C.c.o"
    "build/asm/disc1/3E2A4.s.o"
    "build/src/func_8004E970.c.o"
    "build/asm/disc1/3F17C.s.o"
    "build/src/func_8004F448.c.o"
    "build/asm/disc1/3FC64.s.o"
    "build/src/func_8004F808.c.o"
    "build/asm/disc1/40038.s.o"
    "build/src/func_80050D18.c.o"
    "build/asm/disc1/41520.s.o"
    "build/src/func_80051084.c.o"
    "build/asm/disc1/41898.s.o"
    "build/src/func_80051244.c.o"
    "build/asm/disc1/41A58.s.o"
    "build/src/func_800514F8.c.o"
    "build/src/func_80051504.c.o"
    "build/asm/disc1/41D10.s.o"
    "build/src/func_80051834.c.o"
    "build/asm/disc1/4204C.s.o"
    "build/src/func_80051E48.c.o"
    "build/src/func_80051E58.c.o"
    "build/asm/disc1/42664.s.o"
    "build/src/func_80052514.c.o"
    "build/src/func_80052524.c.o"
    "build/asm/disc1/42D34.s.o"
    "build/src/func_8005257C.c.o"
    "build/asm/disc1/42D94.s.o"
    "build/src/func_800527B4.c.o"
    "build/src/func_800527C0.c.o"
    "build/asm/disc1/42FC8.s.o"
    "build/src/func_80052EB0.c.o"
    "build/asm/disc1/436C0.s.o"
    "build/src/func_80052F0C.c.o"
    "build/asm/disc1/43724.s.o"
    "build/src/func_80054288.c.o"
    "build/src/func_80054294.c.o"
    "build/asm/disc1/44AA0.s.o"
    "build/src/func_80057ECC.c.o"
    "build/asm/disc1/486D8.s.o"
    "build/src/func_8005B890.c.o"
    "build/src/func_8005B89C.c.o"
    "build/asm/disc1/4C0A8.s.o"
    "build/src/func_8005BC98.c.o"
    "build/src/func_8005BCA8.c.o"
    "build/src/func_8005BCB0.c.o"
    "build/asm/disc1/4C4BC.s.o"
    "build/src/func_8005BEDC.c.o"
    "build/asm/disc1/4C6E8.s.o"
    "build/src/func_8005C488.c.o"
    "build/asm/disc1/4CC98.s.o"
    "build/src/func_8005E114.c.o"
    "build/src/func_8005E120.c.o"
    "build/asm/disc1/4E92C.s.o"
    "build/src/func_8005E57C.c.o"
    "build/asm/disc1/4ED88.s.o"
    "build/src/func_8005E6E4.c.o"
    "build/asm/disc1/4EEF0.s.o"
    "build/src/func_8005E884.c.o"
    "build/src/func_8005E894.c.o"
    "build/asm/disc1/4F0A4.s.o"
    "build/src/func_8005EB58.c.o"
    "build/asm/disc1/4F364.s.o"
    "build/src/func_8005EEC8.c.o"
    "build/asm/disc1/4F6D4.s.o"
    "build/src/func_800614A0.c.o"
    "build/asm/disc1/51CAC.s.o"
    "build/src/func_800622B0.c.o"
    "build/asm/disc1/52ABC.s.o"
    "build/src/func_800629B0.c.o"
    "build/asm/disc1/531BC.s.o"
    "build/src/func_80062CB8.c.o"
    "build/src/func_80062CC4.c.o"
    "build/src/func_80062CD0.c.o"
    "build/asm/disc1/534E4.s.o"
    "build/src/func_80063198.c.o"
    "build/src/func_800631AC.c.o"
    "build/asm/disc1/539C0.s.o"
    "build/src/func_80064A48.c.o"
    "build/asm/disc1/55254.s.o"
    "build/src/func_80064C20.c.o"
    "build/asm/disc1/55430.s.o"
    "build/src/func_8006EBD4.c.o"
    "build/asm/disc1/5F3E4.s.o"
    "build/src/func_80073DE8.c.o"
    "build/src/func_80073DF8.c.o"
    "build/asm/disc1/64610.s.o"
    "build/src/func_80074330.c.o"
    "build/asm/disc1/64B54.s.o"
    "build/src/func_800744A4.c.o"
    "build/asm/disc1/64CC8.s.o"
    "build/src/func_8007474C.c.o"
    "build/asm/disc1/64F70.s.o"
    "build/src/func_80074A14.c.o"
    "build/src/func_80074A28.c.o"
    "build/asm/disc1/65238.s.o"
    "build/src/func_80074CB8.c.o"
    "build/asm/disc1/654C8.s.o"
    "build/src/func_8007633C.c.o"
    "build/asm/disc1/66B54.s.o"
    "build/src/func_80077A28.c.o"
    "build/asm/disc1/6824C.s.o"
    "build/src/func_80077B64.c.o"
    "build/asm/disc1/68378.s.o"
    "build/src/func_80077B84.c.o"
    "build/asm/disc1/68398.s.o"
    "build/src/func_80077BA4.c.o"
    "build/asm/disc1/683B8.s.o"
    "build/src/func_80077BC4.c.o"
    "build/asm/disc1/683D8.s.o"
    "build/src/func_80077BE4.c.o"
    "build/asm/disc1/683F8.s.o"
    "build/src/func_80077C04.c.o"
    "build/asm/disc1/68418.s.o"
    "build/src/func_80077C24.c.o"
    "build/asm/disc1/68438.s.o"
    "build/src/func_80077C44.c.o"
    "build/asm/disc1/68458.s.o"
    "build/src/func_80077C64.c.o"
    "build/asm/disc1/68478.s.o"
    "build/src/func_8007A324.c.o"
    "build/src/func_8007A334.c.o"
    "build/src/func_8007A344.c.o"
    "build/src/func_8007A354.c.o"
    "build/asm/disc1/6AB60.s.o"
    "build/src/func_8007A3EC.c.o"
    "build/asm/disc1/6AC00.s.o"
    "build/src/func_8007A4A8.c.o"
    "build/src/func_8007A4BC.c.o"
    "build/asm/disc1/6ACD0.s.o"
    "build/src/func_8007C130.c.o"
    "build/asm/disc1/6C93C.s.o"
    "build/src/func_8007DEA4.c.o"
    "build/src/func_8007DEB0.c.o"
    "build/asm/disc1/6E6C0.s.o"
    "build/src/func_8007F778.c.o"
    "build/asm/disc1/6FF88.s.o"
    "build/src/func_8007FBC0.c.o"
    "build/src/func_8007FBCC.c.o"
    "build/src/func_8007FBD8.c.o"
    "build/src/func_8007FBE4.c.o"
    "build/asm/disc1/703F0.s.o"
    "build/src/func_8007FC08.c.o"
    "build/src/func_8007FC18.c.o"
    "build/src/func_8007FC28.c.o"
    "build/src/func_8007FC34.c.o"
    "build/src/func_8007FC44.c.o"
    "build/src/func_8007FC54.c.o"
    "build/asm/disc1/70464.s.o"
    "build/src/func_8007FCAC.c.o"
    "build/asm/disc1/704BC.s.o"
    "build/src/func_80080930.c.o"
    "build/src/func_80080940.c.o"
    "build/asm/disc1/71150.s.o"
    "build/src/func_80080CC8.c.o"
    "build/asm/disc1/714DC.s.o"
    "build/src/func_80081254.c.o"
    "build/asm/disc1/71A68.s.o"
    "build/src/func_800822AC.c.o"
    "build/asm/disc1/72ABC.s.o"
    "build/src/func_80082CDC.c.o"
    "build/asm/disc1/734F0.s.o"
    "build/src/func_800835A4.c.o"
    "build/src/func_800835B0.c.o"
    "build/asm/disc1/73DC0.s.o"
    "build/src/func_80083E70.c.o"
    "build/asm/disc1/74684.s.o"
    "build/src/func_80083EE4.c.o"
    "build/asm/disc1/746F8.s.o"
    "build/src/func_800847A0.c.o"
    "build/asm/disc1/74FB0.s.o"
    "build/src/func_80085728.c.o"
    "build/asm/disc1/75F44.s.o"
    "build/src/func_800870E0.c.o"
    "build/asm/disc1/778F0.s.o"
    "build/src/func_80087198.c.o"
    "build/asm/disc1/779AC.s.o"
    "build/src/func_80087414.c.o"
    "build/asm/disc1/77C28.s.o"
    "build/src/func_8008AB1C.c.o"
    "build/asm/disc1/7B39C.s.o"
    "build/src/func_8008CA7C.c.o"
    "build/asm/disc1/7D284.s.o"
    "build/src/func_8008D7C0.c.o"
    "build/asm/disc1/7DFD0.s.o"
    "build/src/func_8008D820.c.o"
    "build/asm/disc1/7E044.s.o"
    "build/src/func_8008F694.c.o"
    "build/src/func_8008F6A8.c.o"
    "build/asm/disc1/7FEB0.s.o"
    "build/src/func_8008F868.c.o"
    "build/src/func_8008F880.c.o"
    "build/asm/disc1/80098.s.o"
    "build/src/func_8008FCB4.c.o"
    "build/asm/disc1/804BC.s.o"
    "build/src/func_800904A0.c.o"
    "build/src/func_800904AC.c.o"
    "build/src/func_800904B4.c.o"
    "build/src/func_800904BC.c.o"
    "build/asm/disc1/80CC4.s.o"
    "build/src/func_800906B4.c.o"
    "build/asm/disc1/80EE4.s.o"
    "build/src/func_80090A0C.c.o"
    "build/asm/disc1/81220.s.o"
    "build/src/func_80090C38.c.o"
    "build/src/func_80090C4C.c.o"
    "build/src/func_80090C60.c.o"
    "build/src/func_80090C74.c.o"
    "build/asm/disc1/81488.s.o"
    "build/src/func_80090F54.c.o"
    "build/asm/disc1/81768.s.o"
    "build/asm/disc1/data/818A0.rodata.s.o"
    "build/asm/disc1/B2AF8.s.o"
    "build/src/func_800C2B40.c.o"
    "build/src/func_800C2B50.c.o"
    "build/asm/disc1/B3368.s.o"
    "build/src/func_800C7DC4.c.o"
    "build/src/func_800C7DD4.c.o"
    "build/src/func_800C7DDC.c.o"
    "build/asm/disc1/B85E4.s.o"
    "build/src/func_800C8268.c.o"
    "build/asm/disc1/B8A70.s.o"
    "build/src/func_800C8BB4.c.o"
    "build/asm/disc1/B93C0.s.o"
    "build/src/func_800C8F08.c.o"
    "build/src/func_800C8F18.c.o"
    "build/src/func_800C8F20.c.o"
    "build/asm/disc1/B9728.s.o"
    "build/src/func_800C9260.c.o"
    "build/asm/disc1/B9A68.s.o"
    "build/src/func_800C9968.c.o"
    "build/asm/disc1/BA174.s.o"
    "build/src/func_800C9C00.c.o"
    "build/asm/disc1/BA410.s.o"
    "build/src/func_800C9EA0.c.o"
    "build/asm/disc1/BA6A8.s.o"
    "build/src/func_800CA4A8.c.o"
    "build/asm/disc1/BACB4.s.o"
    "build/src/func_800CA798.c.o"
    "build/asm/disc1/BAFA8.s.o"
    "build/src/func_800CACD4.c.o"
    "build/asm/disc1/BB4DC.s.o"
    "build/src/func_800CBB24.c.o"
    "build/asm/disc1/BC330.s.o"
    "build/src/func_800CBFA4.c.o"
    "build/asm/disc1/BC7B4.s.o"
    "build/src/func_800CCF80.c.o"
    "build/asm/disc1/BD790.s.o"
    "build/src/func_800CD2DC.c.o"
    "build/src/func_800CD2E4.c.o"
    "build/asm/disc1/BDAEC.s.o"
    "build/src/func_800CD59C.c.o"
    "build/src/func_800CD5A4.c.o"
    "build/asm/disc1/BDDB0.s.o"
    "build/src/func_800CD71C.c.o"
    "build/asm/disc1/BDF28.s.o"
    "build/src/func_800CD960.c.o"
    "build/asm/disc1/BE170.s.o"
    "build/src/func_800CDD04.c.o"
    "build/asm/disc1/BE50C.s.o"
    "build/src/func_800CDF40.c.o"
    "build/asm/disc1/BE74C.s.o"
    "build/src/func_800CE1DC.c.o"
    "build/asm/disc1/BE9EC.s.o"
    "build/src/func_800CE3AC.c.o"
    "build/asm/disc1/BEBB4.s.o"
    "build/src/func_800CE464.c.o"
    "build/asm/disc1/BEC70.s.o"
    "build/src/func_800D4850.c.o"
    "build/asm/disc1/C5060.s.o"
)
SOURCES=(
    "asm/disc1/header.s"
    "asm/disc1/data/800.rodata.s"
    "asm/disc1/2A0C.s"
    "src/func_80017E9C.c"
    "asm/disc1/86A4.s"
    "src/func_80017FDC.c"
    "src/func_80017FF0.c"
    "asm/disc1/8804.s"
    "src/func_80019050.c"
    "src/func_80019058.c"
    "asm/disc1/9860.s"
    "src/func_800190AC.c"
    "src/func_800190B4.c"
    "asm/disc1/98BC.s"
    "src/func_800192B8.c"
    "src/func_800192C8.c"
    "asm/disc1/9ADC.s"
    "src/func_80019BE4.c"
    "asm/disc1/A404.s"
    "src/func_80020EFC.c"
    "asm/disc1/11718.s"
    "src/func_80033A20.c"
    "asm/disc1/2422C.s"
    "src/func_800371A4.c"
    "asm/disc1/279B0.s"
    "src/func_80037454.c"
    "asm/disc1/27C6C.s"
    "src/func_800375B4.c"
    "src/func_800375C4.c"
    "src/func_800375D0.c"
    "asm/disc1/27DE0.s"
    "src/func_80037864.c"
    "asm/disc1/28070.s"
    "src/func_80038940.c"
    "asm/disc1/29154.s"
    "src/func_80038D0C.c"
    "asm/disc1/2951C.s"
    "src/func_8003D82C.c"
    "asm/disc1/2E034.s"
    "src/func_8003DFC8.c"
    "src/func_8003DFD0.c"
    "asm/disc1/2E7D8.s"
    "src/func_8003FFAC.c"
    "src/func_8003FFBC.c"
    "asm/disc1/307CC.s"
    "src/func_800428C4.c"
    "asm/disc1/330D4.s"
    "src/func_80042910.c"
    "asm/disc1/33128.s"
    "src/func_80042B28.c"
    "src/func_80042B38.c"
    "src/func_80042B50.c"
    "src/func_80042B6C.c"
    "src/func_80042BC8.c"
    "src/func_80042BD8.c"
    "src/func_80042BEC.c"
    "src/func_80042C00.c"
    "src/func_80042C14.c"
    "src/func_80042C28.c"
    "src/func_80042C3C.c"
    "src/func_80042C50.c"
    "src/func_80042C64.c"
    "asm/disc1/33478.s"
    "src/func_80042CB8.c"
    "asm/disc1/334C4.s"
    "src/func_80042ED0.c"
    "asm/disc1/336DC.s"
    "src/func_80042F20.c"
    "src/func_80042F38.c"
    "asm/disc1/33744.s"
    "src/func_80043038.c"
    "asm/disc1/3384C.s"
    "src/func_80043240.c"
    "asm/disc1/33A4C.s"
    "src/func_800438C0.c"
    "src/func_800438E0.c"
    "asm/disc1/340EC.s"
    "src/func_8004D024.c"
    "asm/disc1/3D830.s"
    "src/func_8004D27C.c"
    "src/func_8004D288.c"
    "asm/disc1/3DA98.s"
    "src/func_8004DA9C.c"
    "asm/disc1/3E2A4.s"
    "src/func_8004E970.c"
    "asm/disc1/3F17C.s"
    "src/func_8004F448.c"
    "asm/disc1/3FC64.s"
    "src/func_8004F808.c"
    "asm/disc1/40038.s"
    "src/func_80050D18.c"
    "asm/disc1/41520.s"
    "src/func_80051084.c"
    "asm/disc1/41898.s"
    "src/func_80051244.c"
    "asm/disc1/41A58.s"
    "src/func_800514F8.c"
    "src/func_80051504.c"
    "asm/disc1/41D10.s"
    "src/func_80051834.c"
    "asm/disc1/4204C.s"
    "src/func_80051E48.c"
    "src/func_80051E58.c"
    "asm/disc1/42664.s"
    "src/func_80052514.c"
    "src/func_80052524.c"
    "asm/disc1/42D34.s"
    "src/func_8005257C.c"
    "asm/disc1/42D94.s"
    "src/func_800527B4.c"
    "src/func_800527C0.c"
    "asm/disc1/42FC8.s"
    "src/func_80052EB0.c"
    "asm/disc1/436C0.s"
    "src/func_80052F0C.c"
    "asm/disc1/43724.s"
    "src/func_80054288.c"
    "src/func_80054294.c"
    "asm/disc1/44AA0.s"
    "src/func_80057ECC.c"
    "asm/disc1/486D8.s"
    "src/func_8005B890.c"
    "src/func_8005B89C.c"
    "asm/disc1/4C0A8.s"
    "src/func_8005BC98.c"
    "src/func_8005BCA8.c"
    "src/func_8005BCB0.c"
    "asm/disc1/4C4BC.s"
    "src/func_8005BEDC.c"
    "asm/disc1/4C6E8.s"
    "src/func_8005C488.c"
    "asm/disc1/4CC98.s"
    "src/func_8005E114.c"
    "src/func_8005E120.c"
    "asm/disc1/4E92C.s"
    "src/func_8005E57C.c"
    "asm/disc1/4ED88.s"
    "src/func_8005E6E4.c"
    "asm/disc1/4EEF0.s"
    "src/func_8005E884.c"
    "src/func_8005E894.c"
    "asm/disc1/4F0A4.s"
    "src/func_8005EB58.c"
    "asm/disc1/4F364.s"
    "src/func_8005EEC8.c"
    "asm/disc1/4F6D4.s"
    "src/func_800614A0.c"
    "asm/disc1/51CAC.s"
    "src/func_800622B0.c"
    "asm/disc1/52ABC.s"
    "src/func_800629B0.c"
    "asm/disc1/531BC.s"
    "src/func_80062CB8.c"
    "src/func_80062CC4.c"
    "src/func_80062CD0.c"
    "asm/disc1/534E4.s"
    "src/func_80063198.c"
    "src/func_800631AC.c"
    "asm/disc1/539C0.s"
    "src/func_80064A48.c"
    "asm/disc1/55254.s"
    "src/func_80064C20.c"
    "asm/disc1/55430.s"
    "src/func_8006EBD4.c"
    "asm/disc1/5F3E4.s"
    "src/func_80073DE8.c"
    "src/func_80073DF8.c"
    "asm/disc1/64610.s"
    "src/func_80074330.c"
    "asm/disc1/64B54.s"
    "src/func_800744A4.c"
    "asm/disc1/64CC8.s"
    "src/func_8007474C.c"
    "asm/disc1/64F70.s"
    "src/func_80074A14.c"
    "src/func_80074A28.c"
    "asm/disc1/65238.s"
    "src/func_80074CB8.c"
    "asm/disc1/654C8.s"
    "src/func_8007633C.c"
    "asm/disc1/66B54.s"
    "src/func_80077A28.c"
    "asm/disc1/6824C.s"
    "src/func_80077B64.c"
    "asm/disc1/68378.s"
    "src/func_80077B84.c"
    "asm/disc1/68398.s"
    "src/func_80077BA4.c"
    "asm/disc1/683B8.s"
    "src/func_80077BC4.c"
    "asm/disc1/683D8.s"
    "src/func_80077BE4.c"
    "asm/disc1/683F8.s"
    "src/func_80077C04.c"
    "asm/disc1/68418.s"
    "src/func_80077C24.c"
    "asm/disc1/68438.s"
    "src/func_80077C44.c"
    "asm/disc1/68458.s"
    "src/func_80077C64.c"
    "asm/disc1/68478.s"
    "src/func_8007A324.c"
    "src/func_8007A334.c"
    "src/func_8007A344.c"
    "src/func_8007A354.c"
    "asm/disc1/6AB60.s"
    "src/func_8007A3EC.c"
    "asm/disc1/6AC00.s"
    "src/func_8007A4A8.c"
    "src/func_8007A4BC.c"
    "asm/disc1/6ACD0.s"
    "src/func_8007C130.c"
    "asm/disc1/6C93C.s"
    "src/func_8007DEA4.c"
    "src/func_8007DEB0.c"
    "asm/disc1/6E6C0.s"
    "src/func_8007F778.c"
    "asm/disc1/6FF88.s"
    "src/func_8007FBC0.c"
    "src/func_8007FBCC.c"
    "src/func_8007FBD8.c"
    "src/func_8007FBE4.c"
    "asm/disc1/703F0.s"
    "src/func_8007FC08.c"
    "src/func_8007FC18.c"
    "src/func_8007FC28.c"
    "src/func_8007FC34.c"
    "src/func_8007FC44.c"
    "src/func_8007FC54.c"
    "asm/disc1/70464.s"
    "src/func_8007FCAC.c"
    "asm/disc1/704BC.s"
    "src/func_80080930.c"
    "src/func_80080940.c"
    "asm/disc1/71150.s"
    "src/func_80080CC8.c"
    "asm/disc1/714DC.s"
    "src/func_80081254.c"
    "asm/disc1/71A68.s"
    "src/func_800822AC.c"
    "asm/disc1/72ABC.s"
    "src/func_80082CDC.c"
    "asm/disc1/734F0.s"
    "src/func_800835A4.c"
    "src/func_800835B0.c"
    "asm/disc1/73DC0.s"
    "src/func_80083E70.c"
    "asm/disc1/74684.s"
    "src/func_80083EE4.c"
    "asm/disc1/746F8.s"
    "src/func_800847A0.c"
    "asm/disc1/74FB0.s"
    "src/func_80085728.c"
    "asm/disc1/75F44.s"
    "src/func_800870E0.c"
    "asm/disc1/778F0.s"
    "src/func_80087198.c"
    "asm/disc1/779AC.s"
    "src/func_80087414.c"
    "asm/disc1/77C28.s"
    "src/func_8008AB1C.c"
    "asm/disc1/7B39C.s"
    "src/func_8008CA7C.c"
    "asm/disc1/7D284.s"
    "src/func_8008D7C0.c"
    "asm/disc1/7DFD0.s"
    "src/func_8008D820.c"
    "asm/disc1/7E044.s"
    "src/func_8008F694.c"
    "src/func_8008F6A8.c"
    "asm/disc1/7FEB0.s"
    "src/func_8008F868.c"
    "src/func_8008F880.c"
    "asm/disc1/80098.s"
    "src/func_8008FCB4.c"
    "asm/disc1/804BC.s"
    "src/func_800904A0.c"
    "src/func_800904AC.c"
    "src/func_800904B4.c"
    "src/func_800904BC.c"
    "asm/disc1/80CC4.s"
    "src/func_800906B4.c"
    "asm/disc1/80EE4.s"
    "src/func_80090A0C.c"
    "asm/disc1/81220.s"
    "src/func_80090C38.c"
    "src/func_80090C4C.c"
    "src/func_80090C60.c"
    "src/func_80090C74.c"
    "asm/disc1/81488.s"
    "src/func_80090F54.c"
    "asm/disc1/81768.s"
    "asm/disc1/data/818A0.rodata.s"
    "asm/disc1/B2AF8.s"
    "src/func_800C2B40.c"
    "src/func_800C2B50.c"
    "asm/disc1/B3368.s"
    "src/func_800C7DC4.c"
    "src/func_800C7DD4.c"
    "src/func_800C7DDC.c"
    "asm/disc1/B85E4.s"
    "src/func_800C8268.c"
    "asm/disc1/B8A70.s"
    "src/func_800C8BB4.c"
    "asm/disc1/B93C0.s"
    "src/func_800C8F08.c"
    "src/func_800C8F18.c"
    "src/func_800C8F20.c"
    "asm/disc1/B9728.s"
    "src/func_800C9260.c"
    "asm/disc1/B9A68.s"
    "src/func_800C9968.c"
    "asm/disc1/BA174.s"
    "src/func_800C9C00.c"
    "asm/disc1/BA410.s"
    "src/func_800C9EA0.c"
    "asm/disc1/BA6A8.s"
    "src/func_800CA4A8.c"
    "asm/disc1/BACB4.s"
    "src/func_800CA798.c"
    "asm/disc1/BAFA8.s"
    "src/func_800CACD4.c"
    "asm/disc1/BB4DC.s"
    "src/func_800CBB24.c"
    "asm/disc1/BC330.s"
    "src/func_800CBFA4.c"
    "asm/disc1/BC7B4.s"
    "src/func_800CCF80.c"
    "asm/disc1/BD790.s"
    "src/func_800CD2DC.c"
    "src/func_800CD2E4.c"
    "asm/disc1/BDAEC.s"
    "src/func_800CD59C.c"
    "src/func_800CD5A4.c"
    "asm/disc1/BDDB0.s"
    "src/func_800CD71C.c"
    "asm/disc1/BDF28.s"
    "src/func_800CD960.c"
    "asm/disc1/BE170.s"
    "src/func_800CDD04.c"
    "asm/disc1/BE50C.s"
    "src/func_800CDF40.c"
    "asm/disc1/BE74C.s"
    "src/func_800CE1DC.c"
    "asm/disc1/BE9EC.s"
    "src/func_800CE3AC.c"
    "asm/disc1/BEBB4.s"
    "src/func_800CE464.c"
    "asm/disc1/BEC70.s"
    "src/func_800D4850.c"
    "asm/disc1/C5060.s"
)

die() { echo "ERROR: $*" >&2; exit 1; }
info() { echo "  $*"; }
step() { echo; echo "=== $* ==="; }

# --- root / inputs ---
[[ -f "$ROOT/CLAUDE.md" && -d "$ROOT/configs/USA" ]] || die "not repo root ($ROOT)"

step "Prerequisites"
if [[ ! -f "$EXE" ]]; then
    die "missing $EXE — run scripts/extract_us.sh 1"
fi
actual_sha1="$(sha1sum "$EXE" | cut -d' ' -f1)"
[[ "$actual_sha1" == "$EXPECTED_SHA1" ]] || die "EXE SHA-1 mismatch (got $actual_sha1)"
info "OK original EXE SHA-1 $actual_sha1"

for s in "${SOURCES[@]}"; do
    [[ -f "$ROOT/$s" ]] || die "missing $s — run scripts/split_us.sh"
done
info "OK all split sources + C leaves present (${#SOURCES[@]} units)"
[[ -f "$ROOT/linkers/disc1.ld" ]] || die "missing linkers/disc1.ld — run scripts/split_us.sh"
info "OK linkers/disc1.ld present"

# --- toolchain discovery ---
step "Toolchain"
AS="" LD="" OBJCOPY="" READELF="" CC=""
if command -v mipsel-linux-gnu-as >/dev/null 2>&1 \
    && command -v mipsel-linux-gnu-gcc >/dev/null 2>&1; then
    AS="$(command -v mipsel-linux-gnu-as)"
    LD="$(command -v mipsel-linux-gnu-ld)"
    OBJCOPY="$(command -v mipsel-linux-gnu-objcopy)"
    READELF="$(command -v mipsel-linux-gnu-readelf)"
    CC="$(command -v mipsel-linux-gnu-gcc)"
    TOOL_NOTE="host PATH"
elif command -v distrobox >/dev/null 2>&1 && distrobox list 2>/dev/null | grep -q 'pe-mipsel'; then
    # Run later steps via distrobox enter
    RUNNER=(distrobox enter pe-mipsel --)
    AS="mipsel-linux-gnu-as"
    LD="mipsel-linux-gnu-ld"
    OBJCOPY="mipsel-linux-gnu-objcopy"
    READELF="mipsel-linux-gnu-readelf"
    CC="mipsel-linux-gnu-gcc"
    TOOL_NOTE="distrobox pe-mipsel"
else
    die "mipsel-linux-gnu-{as,gcc} not on PATH and Distrobox pe-mipsel not found.
Install/use Phase 4G/4J path: distrobox create -i docker.io/library/debian:trixie -n pe-mipsel
  then: distrobox enter pe-mipsel -- sudo apt-get install -y binutils-mipsel-linux-gnu gcc-mipsel-linux-gnu
Or put mipsel-linux-gnu-{as,ld,objcopy,readelf,gcc} on PATH."
fi

# If tools are on host PATH, no runner prefix.
if [[ -z "${RUNNER+x}" ]]; then
    RUNNER=()
fi

run() { "${RUNNER[@]}" "$@"; }

# --- Era compiler (GCC 2.7.2-psx + maspsx) for functions GCC 14.2 cannot match ---
# The retail EXE was built with Psy-Q ccpsx (a GCC 2.7.x MIPS backend). Proven
# era fingerprints include move->addu, $at absolute-store macros, index-first
# operand order, and $v0/$v1 allocation. lui;ori synthesis is still untested.
# Era leaves compile via
# era cpp -> era cc1 -> maspsx (aspsx assembler-macro layer) -> GNU as. All other
# leaves stay on GCC 14.2 and keep byte-identical output. See scripts/setup_era.sh.
ERA_DIR="$ROOT/tools/era"
ERA_CPP="$ERA_DIR/gcc-2.7.2-psx/cpp"
ERA_CC1="$ERA_DIR/gcc-2.7.2-psx/cc1"
MASPSX="$ERA_DIR/maspsx/maspsx.py"
ERA_ASPSX_VER="2.21"

# era_compile <src.c> <out.o> [cc1 flags...]   (cc1/cpp are host x86 binaries; as via run)
era_compile() {
    local src="$1" out="$2"; shift 2
    [[ -x "$ERA_CC1" && -x "$ERA_CPP" && -f "$MASPSX" ]] \
        || die "era toolchain missing — run scripts/setup_era.sh (needed for era leaves)"
    local d; d="$(mktemp -d)"
    "$ERA_CPP" "$src" > "$d/x.i" 2>/dev/null
    "$ERA_CC1" -quiet "$@" "$d/x.i" -o "$d/x.s"
    # Close stdin: maspsx treats non-TTY as pipe mode and can hang on open agent sockets.
    # --dont-expand-li: maspsx expands li→ori for positive small consts; ROM wants addiu — defer to GNU as.
    python3 "$MASPSX" --aspsx-version="$ERA_ASPSX_VER" --dont-expand-li "$d/x.s" > "$d/xm.s" </dev/null
    run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o "$out" "$d/xm.s"
    rm -rf "$d"
}

AS_VER="$(run "$AS" --version 2>/dev/null | head -n1 || true)"
LD_VER="$(run "$LD" --version 2>/dev/null | head -n1 || true)"
CC_VER="$(run "$CC" --version 2>/dev/null | head -n1 || true)"
info "using: $TOOL_NOTE"
info "as:  $AS ($AS_VER)"
info "ld:  $LD ($LD_VER)"
info "cc:  $CC ($CC_VER)"
info "assembler flags: $ASFLAGS_DEFAULT -I $ROOT/include"
info "C leaf flags:    $CFLAGS_LEAF"

# --- assemble ---
step "Assemble (asm units → build/asm/disc1/**/*.s.o)"
mkdir -p build/asm/disc1/data build/src
# shellcheck disable=SC2086
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/header.s.o            asm/disc1/header.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/data/800.rodata.s.o asm/disc1/data/800.rodata.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2A0C.s.o asm/disc1/2A0C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/86A4.s.o asm/disc1/86A4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/8804.s.o asm/disc1/8804.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/9860.s.o asm/disc1/9860.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/98BC.s.o asm/disc1/98BC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/9ADC.s.o asm/disc1/9ADC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/A404.s.o asm/disc1/A404.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/11718.s.o asm/disc1/11718.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2422C.s.o asm/disc1/2422C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/279B0.s.o asm/disc1/279B0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/27C6C.s.o asm/disc1/27C6C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/27DE0.s.o asm/disc1/27DE0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/28070.s.o asm/disc1/28070.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/29154.s.o asm/disc1/29154.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2951C.s.o asm/disc1/2951C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2E034.s.o asm/disc1/2E034.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2E7D8.s.o asm/disc1/2E7D8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/307CC.s.o asm/disc1/307CC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/330D4.s.o asm/disc1/330D4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/33128.s.o asm/disc1/33128.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/33478.s.o asm/disc1/33478.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/334C4.s.o asm/disc1/334C4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/336DC.s.o asm/disc1/336DC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/33744.s.o asm/disc1/33744.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/3384C.s.o asm/disc1/3384C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/33A4C.s.o asm/disc1/33A4C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/340EC.s.o asm/disc1/340EC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/3D830.s.o asm/disc1/3D830.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/3DA98.s.o asm/disc1/3DA98.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/3E2A4.s.o asm/disc1/3E2A4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/3F17C.s.o asm/disc1/3F17C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/3FC64.s.o asm/disc1/3FC64.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/40038.s.o asm/disc1/40038.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/41520.s.o asm/disc1/41520.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/41898.s.o asm/disc1/41898.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/41A58.s.o asm/disc1/41A58.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/41D10.s.o asm/disc1/41D10.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4204C.s.o asm/disc1/4204C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/42664.s.o asm/disc1/42664.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/42D34.s.o asm/disc1/42D34.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/42D94.s.o asm/disc1/42D94.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/42FC8.s.o asm/disc1/42FC8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/436C0.s.o asm/disc1/436C0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/43724.s.o asm/disc1/43724.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/44AA0.s.o asm/disc1/44AA0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/486D8.s.o asm/disc1/486D8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4C0A8.s.o asm/disc1/4C0A8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4C4BC.s.o asm/disc1/4C4BC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4C6E8.s.o asm/disc1/4C6E8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4CC98.s.o asm/disc1/4CC98.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4E92C.s.o asm/disc1/4E92C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4ED88.s.o asm/disc1/4ED88.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4EEF0.s.o asm/disc1/4EEF0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4F0A4.s.o asm/disc1/4F0A4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4F364.s.o asm/disc1/4F364.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/4F6D4.s.o asm/disc1/4F6D4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/51CAC.s.o asm/disc1/51CAC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/52ABC.s.o asm/disc1/52ABC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/531BC.s.o asm/disc1/531BC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/534E4.s.o asm/disc1/534E4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/539C0.s.o asm/disc1/539C0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/55254.s.o asm/disc1/55254.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/55430.s.o asm/disc1/55430.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/5F3E4.s.o asm/disc1/5F3E4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/64610.s.o asm/disc1/64610.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/64B54.s.o asm/disc1/64B54.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/64CC8.s.o asm/disc1/64CC8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/64F70.s.o asm/disc1/64F70.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/65238.s.o asm/disc1/65238.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/654C8.s.o asm/disc1/654C8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/66B54.s.o asm/disc1/66B54.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6824C.s.o asm/disc1/6824C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/68378.s.o asm/disc1/68378.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/68398.s.o asm/disc1/68398.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/683B8.s.o asm/disc1/683B8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/683D8.s.o asm/disc1/683D8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/683F8.s.o asm/disc1/683F8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/68418.s.o asm/disc1/68418.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/68438.s.o asm/disc1/68438.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/68458.s.o asm/disc1/68458.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/68478.s.o asm/disc1/68478.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6AB60.s.o asm/disc1/6AB60.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6AC00.s.o asm/disc1/6AC00.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6ACD0.s.o asm/disc1/6ACD0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6C93C.s.o asm/disc1/6C93C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6E6C0.s.o asm/disc1/6E6C0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/6FF88.s.o asm/disc1/6FF88.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/703F0.s.o asm/disc1/703F0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/70464.s.o asm/disc1/70464.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/704BC.s.o asm/disc1/704BC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/71150.s.o asm/disc1/71150.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/714DC.s.o asm/disc1/714DC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/71A68.s.o asm/disc1/71A68.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/72ABC.s.o asm/disc1/72ABC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/734F0.s.o asm/disc1/734F0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/73DC0.s.o asm/disc1/73DC0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/74684.s.o asm/disc1/74684.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/746F8.s.o asm/disc1/746F8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/74FB0.s.o asm/disc1/74FB0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/75F44.s.o asm/disc1/75F44.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/778F0.s.o asm/disc1/778F0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/779AC.s.o asm/disc1/779AC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/77C28.s.o asm/disc1/77C28.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/7B39C.s.o asm/disc1/7B39C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/7D284.s.o asm/disc1/7D284.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/7DFD0.s.o asm/disc1/7DFD0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/7E044.s.o asm/disc1/7E044.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/7FEB0.s.o asm/disc1/7FEB0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/80098.s.o asm/disc1/80098.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/804BC.s.o asm/disc1/804BC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/80CC4.s.o asm/disc1/80CC4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/80EE4.s.o asm/disc1/80EE4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/81220.s.o asm/disc1/81220.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/81488.s.o asm/disc1/81488.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/81768.s.o asm/disc1/81768.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/data/818A0.rodata.s.o asm/disc1/data/818A0.rodata.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B2AF8.s.o asm/disc1/B2AF8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B3368.s.o asm/disc1/B3368.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B85E4.s.o asm/disc1/B85E4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B8A70.s.o asm/disc1/B8A70.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B93C0.s.o asm/disc1/B93C0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B9728.s.o asm/disc1/B9728.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B9A68.s.o asm/disc1/B9A68.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BA174.s.o asm/disc1/BA174.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BA410.s.o asm/disc1/BA410.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BA6A8.s.o asm/disc1/BA6A8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BACB4.s.o asm/disc1/BACB4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BAFA8.s.o asm/disc1/BAFA8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BB4DC.s.o asm/disc1/BB4DC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BC330.s.o asm/disc1/BC330.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BC7B4.s.o asm/disc1/BC7B4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BD790.s.o asm/disc1/BD790.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BDAEC.s.o asm/disc1/BDAEC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BDDB0.s.o asm/disc1/BDDB0.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BDF28.s.o asm/disc1/BDF28.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BE170.s.o asm/disc1/BE170.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BE50C.s.o asm/disc1/BE50C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BE74C.s.o asm/disc1/BE74C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BE9EC.s.o asm/disc1/BE9EC.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BEBB4.s.o asm/disc1/BEBB4.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/BEC70.s.o asm/disc1/BEC70.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/C5060.s.o asm/disc1/C5060.s

step "Compile C leaves (207 C leaves (incl. gp batches + era + 5EF + 5EG + 5EH))"
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80017E9C.c.o src/func_80017E9C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80019050.c.o src/func_80019050.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80019058.c.o src/func_80019058.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800190AC.c.o src/func_800190AC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800190B4.c.o src/func_800190B4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80038D0C.c.o src/func_80038D0C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8003D82C.c.o src/func_8003D82C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8003DFC8.c.o src/func_8003DFC8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8003FFBC.c.o src/func_8003FFBC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800428C4.c.o src/func_800428C4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80042B28.c.o src/func_80042B28.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80042BC8.c.o src/func_80042BC8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8004DA9C.c.o src/func_8004DA9C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80050D18.c.o src/func_80050D18.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80051834.c.o src/func_80051834.c
run "$CC" $CFLAGS_LEAF -fno-delayed-branch -c -o build/src/func_80051E48.c.o src/func_80051E48.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80052514.c.o src/func_80052514.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80052524.c.o src/func_80052524.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8005257C.c.o src/func_8005257C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800527C0.c.o src/func_800527C0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8005BCA8.c.o src/func_8005BCA8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8005E884.c.o src/func_8005E884.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80063198.c.o src/func_80063198.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800631AC.c.o src/func_800631AC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80064C20.c.o src/func_80064C20.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8006EBD4.c.o src/func_8006EBD4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80073DE8.c.o src/func_80073DE8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80073DF8.c.o src/func_80073DF8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80074330.c.o src/func_80074330.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800744A4.c.o src/func_800744A4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007474C.c.o src/func_8007474C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80074A28.c.o src/func_80074A28.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80074CB8.c.o src/func_80074CB8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007633C.c.o src/func_8007633C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077A28.c.o src/func_80077A28.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077B64.c.o src/func_80077B64.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077B84.c.o src/func_80077B84.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077BA4.c.o src/func_80077BA4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077BC4.c.o src/func_80077BC4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077BE4.c.o src/func_80077BE4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077C04.c.o src/func_80077C04.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077C24.c.o src/func_80077C24.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077C44.c.o src/func_80077C44.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80077C64.c.o src/func_80077C64.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007A324.c.o src/func_8007A324.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007A334.c.o src/func_8007A334.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007A344.c.o src/func_8007A344.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007A354.c.o src/func_8007A354.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007DEB0.c.o src/func_8007DEB0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007F778.c.o src/func_8007F778.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FC08.c.o src/func_8007FC08.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FC18.c.o src/func_8007FC18.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FC28.c.o src/func_8007FC28.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FC34.c.o src/func_8007FC34.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FC44.c.o src/func_8007FC44.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FC54.c.o src/func_8007FC54.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8007FCAC.c.o src/func_8007FCAC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80080940.c.o src/func_80080940.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800822AC.c.o src/func_800822AC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800835A4.c.o src/func_800835A4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800835B0.c.o src/func_800835B0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80083E70.c.o src/func_80083E70.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80083EE4.c.o src/func_80083EE4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800847A0.c.o src/func_800847A0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800870E0.c.o src/func_800870E0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008CA7C.c.o src/func_8008CA7C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008D7C0.c.o src/func_8008D7C0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008D820.c.o src/func_8008D820.c
era_compile src/func_80017FDC.c build/src/func_80017FDC.c.o -O2 -G0
era_compile src/func_80017FF0.c build/src/func_80017FF0.c.o -O2 -G0
era_compile src/func_800192B8.c build/src/func_800192B8.c.o -O2 -G0
era_compile src/func_800192C8.c build/src/func_800192C8.c.o -O2 -G0
era_compile src/func_8003DFD0.c build/src/func_8003DFD0.c.o -O2 -G0
era_compile src/func_8003FFAC.c build/src/func_8003FFAC.c.o -O2 -G0
era_compile src/func_80042910.c build/src/func_80042910.c.o -O2 -G0
era_compile src/func_80042B38.c build/src/func_80042B38.c.o -O2 -G0
era_compile src/func_80042B50.c build/src/func_80042B50.c.o -O2 -G0
era_compile src/func_80042B6C.c build/src/func_80042B6C.c.o -O2 -G0
era_compile src/func_80042BD8.c build/src/func_80042BD8.c.o -O2 -G0
era_compile src/func_80042BEC.c build/src/func_80042BEC.c.o -O2 -G0
era_compile src/func_80042C00.c build/src/func_80042C00.c.o -O2 -G0
era_compile src/func_80042C14.c build/src/func_80042C14.c.o -O2 -G0
era_compile src/func_80042C28.c build/src/func_80042C28.c.o -O2 -G0
era_compile src/func_80042C3C.c build/src/func_80042C3C.c.o -O2 -G0
era_compile src/func_80042C50.c build/src/func_80042C50.c.o -O2 -G0
era_compile src/func_80042C64.c build/src/func_80042C64.c.o -O2 -G0
# 5EF delay-slot family: maspsx LOCAL PATCH opt-in (sw store fills the j $31
# delay slot; see tools/era/maspsx/maspsx/__init__.py patch log). Env var flows
# through era_compile to the maspsx child process.
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_80074A14.c build/src/func_80074A14.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007A3EC.c build/src/func_8007A3EC.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007A4A8.c build/src/func_8007A4A8.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007A4BC.c build/src/func_8007A4BC.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007C130.c build/src/func_8007C130.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007DEA4.c build/src/func_8007DEA4.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007FBC0.c build/src/func_8007FBC0.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007FBCC.c build/src/func_8007FBCC.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007FBD8.c build/src/func_8007FBD8.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_8007FBE4.c build/src/func_8007FBE4.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_80080930.c build/src/func_80080930.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_80080CC8.c build/src/func_80080CC8.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_80081254.c build/src/func_80081254.c.o -O2 -G0
MASPSX_FILL_STORE_DELAY_SLOT=1 era_compile src/func_80082CDC.c build/src/func_80082CDC.c.o -O2 -G0
era_compile src/func_80087198.c build/src/func_80087198.c.o -O2 -G0
era_compile src/func_80087414.c build/src/func_80087414.c.o -O2 -G0
era_compile src/func_8008AB1C.c build/src/func_8008AB1C.c.o -O1 -G0
era_compile src/func_80085728.c build/src/func_80085728.c.o -O2 -G0
era_compile src/func_800C7DC4.c build/src/func_800C7DC4.c.o -O2 -G0
era_compile src/func_800C7DD4.c build/src/func_800C7DD4.c.o -O2 -G0
era_compile src/func_800C7DDC.c build/src/func_800C7DDC.c.o -O2 -G0
era_compile src/func_800C8F08.c build/src/func_800C8F08.c.o -O2 -G0
era_compile src/func_800C8F18.c build/src/func_800C8F18.c.o -O2 -G0
era_compile src/func_800C8F20.c build/src/func_800C8F20.c.o -O2 -G0
era_compile src/func_800C9C00.c build/src/func_800C9C00.c.o -O2 -G0
era_compile src/func_800CA798.c build/src/func_800CA798.c.o -O2 -G0
era_compile src/func_800CBFA4.c build/src/func_800CBFA4.c.o -O2 -G0
era_compile src/func_800CCF80.c build/src/func_800CCF80.c.o -O2 -G0
era_compile src/func_800CD960.c build/src/func_800CD960.c.o -O2 -G0
era_compile src/func_800CE1DC.c build/src/func_800CE1DC.c.o -O2 -G0
era_compile src/func_800D4850.c build/src/func_800D4850.c.o -O2 -G0
era_compile src/func_800438C0.c build/src/func_800438C0.c.o -O2 -G8
run "$CC" $CFLAGS_LEAF -G 8 -fno-tree-ter -c -o build/src/func_80052F0C.c.o src/func_80052F0C.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80019BE4.c.o src/func_80019BE4.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8004F448.c.o src/func_8004F448.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8004F808.c.o src/func_8004F808.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80042F20.c.o src/func_80042F20.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80043038.c.o src/func_80043038.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80062CD0.c.o src/func_80062CD0.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80051084.c.o src/func_80051084.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80051244.c.o src/func_80051244.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80037454.c.o src/func_80037454.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_800375B4.c.o src/func_800375B4.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_800375D0.c.o src/func_800375D0.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80038940.c.o src/func_80038940.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80020EFC.c.o src/func_80020EFC.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8004D288.c.o src/func_8004D288.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005BC98.c.o src/func_8005BC98.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005C488.c.o src/func_8005C488.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80052EB0.c.o src/func_80052EB0.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005E894.c.o src/func_8005E894.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80033A20.c.o src/func_80033A20.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80037864.c.o src/func_80037864.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80042ED0.c.o src/func_80042ED0.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_800438E0.c.o src/func_800438E0.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8004D27C.c.o src/func_8004D27C.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8004E970.c.o src/func_8004E970.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_800514F8.c.o src/func_800514F8.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80051E58.c.o src/func_80051E58.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_800527B4.c.o src/func_800527B4.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80054288.c.o src/func_80054288.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80054294.c.o src/func_80054294.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80057ECC.c.o src/func_80057ECC.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8005B89C.c.o src/func_8005B89C.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8005BCB0.c.o src/func_8005BCB0.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8005BEDC.c.o src/func_8005BEDC.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_8005E120.c.o src/func_8005E120.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_800614A0.c.o src/func_800614A0.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_800629B0.c.o src/func_800629B0.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80062CC4.c.o src/func_80062CC4.c
run "$CC" $CFLAGS_LEAF -G 8 -c -o build/src/func_80064A48.c.o src/func_80064A48.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_800371A4.c.o src/func_800371A4.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_800375C4.c.o src/func_800375C4.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80042CB8.c.o src/func_80042CB8.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80042F38.c.o src/func_80042F38.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80043240.c.o src/func_80043240.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8004D024.c.o src/func_8004D024.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80051504.c.o src/func_80051504.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005B890.c.o src/func_8005B890.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005E114.c.o src/func_8005E114.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005E57C.c.o src/func_8005E57C.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005E6E4.c.o src/func_8005E6E4.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005EB58.c.o src/func_8005EB58.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_8005EEC8.c.o src/func_8005EEC8.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_800622B0.c.o src/func_800622B0.c
run "$CC" $CFLAGS_LEAF -G 8 -fno-delayed-branch -c -o build/src/func_80062CB8.c.o src/func_80062CB8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008F694.c.o src/func_8008F694.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008F6A8.c.o src/func_8008F6A8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008F868.c.o src/func_8008F868.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008F880.c.o src/func_8008F880.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_8008FCB4.c.o src/func_8008FCB4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800904A0.c.o src/func_800904A0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800904AC.c.o src/func_800904AC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800904B4.c.o src/func_800904B4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800904BC.c.o src/func_800904BC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800906B4.c.o src/func_800906B4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090A0C.c.o src/func_80090A0C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C38.c.o src/func_80090C38.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C4C.c.o src/func_80090C4C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C60.c.o src/func_80090C60.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C74.c.o src/func_80090C74.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090F54.c.o src/func_80090F54.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C2B40.c.o src/func_800C2B40.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C2B50.c.o src/func_800C2B50.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C8268.c.o src/func_800C8268.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C8BB4.c.o src/func_800C8BB4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C9260.c.o src/func_800C9260.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C9968.c.o src/func_800C9968.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C9EA0.c.o src/func_800C9EA0.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CA4A8.c.o src/func_800CA4A8.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CACD4.c.o src/func_800CACD4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CBB24.c.o src/func_800CBB24.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CD2DC.c.o src/func_800CD2DC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CD2E4.c.o src/func_800CD2E4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CD59C.c.o src/func_800CD59C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CD5A4.c.o src/func_800CD5A4.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CD71C.c.o src/func_800CD71C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CDD04.c.o src/func_800CDD04.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CDF40.c.o src/func_800CDF40.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CE3AC.c.o src/func_800CE3AC.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800CE464.c.o src/func_800CE464.c

# Phase 4I / 5B–5G: GNU as / GCC pad section sizes to sh_addralign with trailing
# zeros (.text align 16 → object may be 0x20 for a 0x14-byte function body).
# Original PE1 file spans are exact; strip proven zero-only tail pad and lower
# align to 4 so ld does not re-insert gaps or shift ROM layout.
step "Trim ELF section alignment padding (Phase 4I/5B–5S)"
TRIM="$ROOT/tools/trim_elf_section_pad.py"
[[ -f "$TRIM" ]] || die "missing $TRIM"
python3 "$TRIM" build/asm/disc1/data/800.rodata.s.o .rodata "$SIZE_800_RODATA"
python3 "$TRIM" build/asm/disc1/2A0C.s.o .text "$SIZE_2A0C"
python3 "$TRIM" build/src/func_80017E9C.c.o .text "$SIZE_C_17E9C"
python3 "$TRIM" build/asm/disc1/86A4.s.o .text "$SIZE_86A4"
python3 "$TRIM" build/src/func_80017FDC.c.o .text "$SIZE_C_17FDC"
python3 "$TRIM" build/src/func_80017FF0.c.o .text "$SIZE_C_17FF0"
python3 "$TRIM" build/asm/disc1/8804.s.o .text "$SIZE_8804"
python3 "$TRIM" build/src/func_80019050.c.o .text "$SIZE_C_19050"
python3 "$TRIM" build/src/func_80019058.c.o .text "$SIZE_C_19058"
python3 "$TRIM" build/asm/disc1/9860.s.o .text "$SIZE_9860"
python3 "$TRIM" build/src/func_800190AC.c.o .text "$SIZE_C_190AC"
python3 "$TRIM" build/src/func_800190B4.c.o .text "$SIZE_C_190B4"
python3 "$TRIM" build/asm/disc1/98BC.s.o .text "$SIZE_98BC"
python3 "$TRIM" build/src/func_800192B8.c.o .text "$SIZE_C_192B8"
python3 "$TRIM" build/src/func_800192C8.c.o .text "$SIZE_C_192C8"
python3 "$TRIM" build/asm/disc1/9ADC.s.o .text "$SIZE_9ADC"
python3 "$TRIM" build/src/func_80019BE4.c.o .text "$SIZE_C_19BE4"
python3 "$TRIM" build/asm/disc1/A404.s.o .text "$SIZE_A404"
python3 "$TRIM" build/src/func_80020EFC.c.o .text "$SIZE_C_20EFC"
python3 "$TRIM" build/asm/disc1/11718.s.o .text "$SIZE_11718"
python3 "$TRIM" build/src/func_80033A20.c.o .text "$SIZE_C_33A20"
python3 "$TRIM" build/asm/disc1/2422C.s.o .text "$SIZE_2422C"
python3 "$TRIM" build/src/func_800371A4.c.o .text "$SIZE_C_371A4"
python3 "$TRIM" build/asm/disc1/279B0.s.o .text "$SIZE_279B0"
python3 "$TRIM" build/src/func_80037454.c.o .text "$SIZE_C_37454"
python3 "$TRIM" build/asm/disc1/27C6C.s.o .text "$SIZE_27C6C"
python3 "$TRIM" build/src/func_800375B4.c.o .text "$SIZE_C_375B4"
python3 "$TRIM" build/src/func_800375C4.c.o .text "$SIZE_C_375C4"
python3 "$TRIM" build/src/func_800375D0.c.o .text "$SIZE_C_375D0"
python3 "$TRIM" build/asm/disc1/27DE0.s.o .text "$SIZE_27DE0"
python3 "$TRIM" build/src/func_80037864.c.o .text "$SIZE_C_37864"
python3 "$TRIM" build/asm/disc1/28070.s.o .text "$SIZE_28070"
python3 "$TRIM" build/src/func_80038940.c.o .text "$SIZE_C_38940"
python3 "$TRIM" build/asm/disc1/29154.s.o .text "$SIZE_29154"
python3 "$TRIM" build/src/func_80038D0C.c.o .text "$SIZE_C_38D0C"
python3 "$TRIM" build/asm/disc1/2951C.s.o .text "$SIZE_2951C"
python3 "$TRIM" build/src/func_8003D82C.c.o .text "$SIZE_C_3D82C"
python3 "$TRIM" build/asm/disc1/2E034.s.o .text "$SIZE_2E034"
python3 "$TRIM" build/src/func_8003DFC8.c.o .text "$SIZE_C_3DFC8"
python3 "$TRIM" build/src/func_8003DFD0.c.o .text "$SIZE_C_3DFD0"
python3 "$TRIM" build/asm/disc1/2E7D8.s.o .text "$SIZE_2E7D8"
python3 "$TRIM" build/src/func_8003FFAC.c.o .text "$SIZE_C_3FFAC"
python3 "$TRIM" build/src/func_8003FFBC.c.o .text "$SIZE_C_3FFBC"
python3 "$TRIM" build/asm/disc1/307CC.s.o .text "$SIZE_307CC"
python3 "$TRIM" build/src/func_800428C4.c.o .text "$SIZE_C_428C4"
python3 "$TRIM" build/asm/disc1/330D4.s.o .text "$SIZE_330D4"
python3 "$TRIM" build/src/func_80042910.c.o .text "$SIZE_C_42910"
python3 "$TRIM" build/asm/disc1/33128.s.o .text "$SIZE_33128"
python3 "$TRIM" build/src/func_80042B28.c.o .text "$SIZE_C_42B28"
python3 "$TRIM" build/src/func_80042B38.c.o .text "$SIZE_C_42B38"
python3 "$TRIM" build/src/func_80042B50.c.o .text "$SIZE_C_42B50"
python3 "$TRIM" build/src/func_80042B6C.c.o .text "$SIZE_C_42B6C"
python3 "$TRIM" build/src/func_80042BC8.c.o .text "$SIZE_C_42BC8"
python3 "$TRIM" build/src/func_80042BD8.c.o .text "$SIZE_C_42BD8"
python3 "$TRIM" build/src/func_80042BEC.c.o .text "$SIZE_C_42BEC"
python3 "$TRIM" build/src/func_80042C00.c.o .text "$SIZE_C_42C00"
python3 "$TRIM" build/src/func_80042C14.c.o .text "$SIZE_C_42C14"
python3 "$TRIM" build/src/func_80042C28.c.o .text "$SIZE_C_42C28"
python3 "$TRIM" build/src/func_80042C3C.c.o .text "$SIZE_C_42C3C"
python3 "$TRIM" build/src/func_80042C50.c.o .text "$SIZE_C_42C50"
python3 "$TRIM" build/src/func_80042C64.c.o .text "$SIZE_C_42C64"
python3 "$TRIM" build/asm/disc1/33478.s.o .text "$SIZE_33478"
python3 "$TRIM" build/src/func_80042CB8.c.o .text "$SIZE_C_42CB8"
python3 "$TRIM" build/asm/disc1/334C4.s.o .text "$SIZE_334C4"
python3 "$TRIM" build/src/func_80042ED0.c.o .text "$SIZE_C_42ED0"
python3 "$TRIM" build/asm/disc1/336DC.s.o .text "$SIZE_336DC"
python3 "$TRIM" build/src/func_80042F20.c.o .text "$SIZE_C_42F20"
python3 "$TRIM" build/src/func_80042F38.c.o .text "$SIZE_C_42F38"
python3 "$TRIM" build/asm/disc1/33744.s.o .text "$SIZE_33744"
python3 "$TRIM" build/src/func_80043038.c.o .text "$SIZE_C_43038"
python3 "$TRIM" build/asm/disc1/3384C.s.o .text "$SIZE_3384C"
python3 "$TRIM" build/src/func_80043240.c.o .text "$SIZE_C_43240"
python3 "$TRIM" build/asm/disc1/33A4C.s.o .text "$SIZE_33A4C"
python3 "$TRIM" build/src/func_800438C0.c.o .text "$SIZE_C_438C0"
python3 "$TRIM" build/src/func_800438E0.c.o .text "$SIZE_C_438E0"
python3 "$TRIM" build/asm/disc1/340EC.s.o .text "$SIZE_340EC"
python3 "$TRIM" build/src/func_8004D024.c.o .text "$SIZE_C_4D024"
python3 "$TRIM" build/asm/disc1/3D830.s.o .text "$SIZE_3D830"
python3 "$TRIM" build/src/func_8004D27C.c.o .text "$SIZE_C_4D27C"
python3 "$TRIM" build/src/func_8004D288.c.o .text "$SIZE_C_4D288"
python3 "$TRIM" build/asm/disc1/3DA98.s.o .text "$SIZE_3DA98"
python3 "$TRIM" build/src/func_8004DA9C.c.o .text "$SIZE_C_4DA9C"
python3 "$TRIM" build/asm/disc1/3E2A4.s.o .text "$SIZE_3E2A4"
python3 "$TRIM" build/src/func_8004E970.c.o .text "$SIZE_C_4E970"
python3 "$TRIM" build/asm/disc1/3F17C.s.o .text "$SIZE_3F17C"
python3 "$TRIM" build/src/func_8004F448.c.o .text "$SIZE_C_4F448"
python3 "$TRIM" build/asm/disc1/3FC64.s.o .text "$SIZE_3FC64"
python3 "$TRIM" build/src/func_8004F808.c.o .text "$SIZE_C_4F808"
python3 "$TRIM" build/asm/disc1/40038.s.o .text "$SIZE_40038"
python3 "$TRIM" build/src/func_80050D18.c.o .text "$SIZE_C_50D18"
python3 "$TRIM" build/asm/disc1/41520.s.o .text "$SIZE_41520"
python3 "$TRIM" build/src/func_80051084.c.o .text "$SIZE_C_51084"
python3 "$TRIM" build/asm/disc1/41898.s.o .text "$SIZE_41898"
python3 "$TRIM" build/src/func_80051244.c.o .text "$SIZE_C_51244"
python3 "$TRIM" build/asm/disc1/41A58.s.o .text "$SIZE_41A58"
python3 "$TRIM" build/src/func_800514F8.c.o .text "$SIZE_C_514F8"
python3 "$TRIM" build/src/func_80051504.c.o .text "$SIZE_C_51504"
python3 "$TRIM" build/asm/disc1/41D10.s.o .text "$SIZE_41D10"
python3 "$TRIM" build/src/func_80051834.c.o .text "$SIZE_C_51834"
python3 "$TRIM" build/asm/disc1/4204C.s.o .text "$SIZE_4204C"
python3 "$TRIM" build/src/func_80051E48.c.o .text "$SIZE_C_51E48"
python3 "$TRIM" build/src/func_80051E58.c.o .text "$SIZE_C_51E58"
python3 "$TRIM" build/asm/disc1/42664.s.o .text "$SIZE_42664"
python3 "$TRIM" build/src/func_80052514.c.o .text "$SIZE_C_52514"
python3 "$TRIM" build/src/func_80052524.c.o .text "$SIZE_C_52524"
python3 "$TRIM" build/asm/disc1/42D34.s.o .text "$SIZE_42D34"
python3 "$TRIM" build/src/func_8005257C.c.o .text "$SIZE_C_5257C"
python3 "$TRIM" build/asm/disc1/42D94.s.o .text "$SIZE_42D94"
python3 "$TRIM" build/src/func_800527B4.c.o .text "$SIZE_C_527B4"
python3 "$TRIM" build/src/func_800527C0.c.o .text "$SIZE_C_527C0"
python3 "$TRIM" build/asm/disc1/42FC8.s.o .text "$SIZE_42FC8"
python3 "$TRIM" build/src/func_80052EB0.c.o .text "$SIZE_C_52EB0"
python3 "$TRIM" build/asm/disc1/436C0.s.o .text "$SIZE_436C0"
python3 "$TRIM" build/src/func_80052F0C.c.o .text "$SIZE_C_52F0C"
python3 "$TRIM" build/asm/disc1/43724.s.o .text "$SIZE_43724"
python3 "$TRIM" build/src/func_80054288.c.o .text "$SIZE_C_54288"
python3 "$TRIM" build/src/func_80054294.c.o .text "$SIZE_C_54294"
python3 "$TRIM" build/asm/disc1/44AA0.s.o .text "$SIZE_44AA0"
python3 "$TRIM" build/src/func_80057ECC.c.o .text "$SIZE_C_57ECC"
python3 "$TRIM" build/asm/disc1/486D8.s.o .text "$SIZE_486D8"
python3 "$TRIM" build/src/func_8005B890.c.o .text "$SIZE_C_5B890"
python3 "$TRIM" build/src/func_8005B89C.c.o .text "$SIZE_C_5B89C"
python3 "$TRIM" build/asm/disc1/4C0A8.s.o .text "$SIZE_4C0A8"
python3 "$TRIM" build/src/func_8005BC98.c.o .text "$SIZE_C_5BC98"
python3 "$TRIM" build/src/func_8005BCA8.c.o .text "$SIZE_C_5BCA8"
python3 "$TRIM" build/src/func_8005BCB0.c.o .text "$SIZE_C_5BCB0"
python3 "$TRIM" build/asm/disc1/4C4BC.s.o .text "$SIZE_4C4BC"
python3 "$TRIM" build/src/func_8005BEDC.c.o .text "$SIZE_C_5BEDC"
python3 "$TRIM" build/asm/disc1/4C6E8.s.o .text "$SIZE_4C6E8"
python3 "$TRIM" build/src/func_8005C488.c.o .text "$SIZE_C_5C488"
python3 "$TRIM" build/asm/disc1/4CC98.s.o .text "$SIZE_4CC98"
python3 "$TRIM" build/src/func_8005E114.c.o .text "$SIZE_C_5E114"
python3 "$TRIM" build/src/func_8005E120.c.o .text "$SIZE_C_5E120"
python3 "$TRIM" build/asm/disc1/4E92C.s.o .text "$SIZE_4E92C"
python3 "$TRIM" build/src/func_8005E57C.c.o .text "$SIZE_C_5E57C"
python3 "$TRIM" build/asm/disc1/4ED88.s.o .text "$SIZE_4ED88"
python3 "$TRIM" build/src/func_8005E6E4.c.o .text "$SIZE_C_5E6E4"
python3 "$TRIM" build/asm/disc1/4EEF0.s.o .text "$SIZE_4EEF0"
python3 "$TRIM" build/src/func_8005E884.c.o .text "$SIZE_C_5E884"
python3 "$TRIM" build/src/func_8005E894.c.o .text "$SIZE_C_5E894"
python3 "$TRIM" build/asm/disc1/4F0A4.s.o .text "$SIZE_4F0A4"
python3 "$TRIM" build/src/func_8005EB58.c.o .text "$SIZE_C_5EB58"
python3 "$TRIM" build/asm/disc1/4F364.s.o .text "$SIZE_4F364"
python3 "$TRIM" build/src/func_8005EEC8.c.o .text "$SIZE_C_5EEC8"
python3 "$TRIM" build/asm/disc1/4F6D4.s.o .text "$SIZE_4F6D4"
python3 "$TRIM" build/src/func_800614A0.c.o .text "$SIZE_C_614A0"
python3 "$TRIM" build/asm/disc1/51CAC.s.o .text "$SIZE_51CAC"
python3 "$TRIM" build/src/func_800622B0.c.o .text "$SIZE_C_622B0"
python3 "$TRIM" build/asm/disc1/52ABC.s.o .text "$SIZE_52ABC"
python3 "$TRIM" build/src/func_800629B0.c.o .text "$SIZE_C_629B0"
python3 "$TRIM" build/asm/disc1/531BC.s.o .text "$SIZE_531BC"
python3 "$TRIM" build/src/func_80062CB8.c.o .text "$SIZE_C_62CB8"
python3 "$TRIM" build/src/func_80062CC4.c.o .text "$SIZE_C_62CC4"
python3 "$TRIM" build/src/func_80062CD0.c.o .text "$SIZE_C_62CD0"
python3 "$TRIM" build/asm/disc1/534E4.s.o .text "$SIZE_534E4"
python3 "$TRIM" build/src/func_80063198.c.o .text "$SIZE_C_63198"
python3 "$TRIM" build/src/func_800631AC.c.o .text "$SIZE_C_631AC"
python3 "$TRIM" build/asm/disc1/539C0.s.o .text "$SIZE_539C0"
python3 "$TRIM" build/src/func_80064A48.c.o .text "$SIZE_C_64A48"
python3 "$TRIM" build/asm/disc1/55254.s.o .text "$SIZE_55254"
python3 "$TRIM" build/src/func_80064C20.c.o .text "$SIZE_C_64C20"
python3 "$TRIM" build/asm/disc1/55430.s.o .text "$SIZE_55430"
python3 "$TRIM" build/src/func_8006EBD4.c.o .text "$SIZE_C_6EBD4"
python3 "$TRIM" build/asm/disc1/5F3E4.s.o .text "$SIZE_5F3E4"
python3 "$TRIM" build/src/func_80073DE8.c.o .text "$SIZE_C_73DE8"
python3 "$TRIM" build/src/func_80073DF8.c.o .text "$SIZE_C_73DF8"
python3 "$TRIM" build/asm/disc1/64610.s.o .text "$SIZE_64610"
python3 "$TRIM" build/src/func_80074330.c.o .text "$SIZE_C_74330"
python3 "$TRIM" build/asm/disc1/64B54.s.o .text "$SIZE_64B54"
python3 "$TRIM" build/src/func_800744A4.c.o .text "$SIZE_C_744A4"
python3 "$TRIM" build/asm/disc1/64CC8.s.o .text "$SIZE_64CC8"
python3 "$TRIM" build/src/func_8007474C.c.o .text "$SIZE_C_7474C"
python3 "$TRIM" build/asm/disc1/64F70.s.o .text "$SIZE_64F70"
python3 "$TRIM" build/src/func_80074A14.c.o .text "$SIZE_C_74A14"
python3 "$TRIM" build/src/func_80074A28.c.o .text "$SIZE_C_74A28"
python3 "$TRIM" build/asm/disc1/65238.s.o .text "$SIZE_65238"
python3 "$TRIM" build/src/func_80074CB8.c.o .text "$SIZE_C_74CB8"
python3 "$TRIM" build/asm/disc1/654C8.s.o .text "$SIZE_654C8"
python3 "$TRIM" build/src/func_8007633C.c.o .text "$SIZE_C_7633C"
python3 "$TRIM" build/asm/disc1/66B54.s.o .text "$SIZE_66B54"
python3 "$TRIM" build/src/func_80077A28.c.o .text "$SIZE_C_77A28"
python3 "$TRIM" build/asm/disc1/6824C.s.o .text "$SIZE_6824C"
python3 "$TRIM" build/src/func_80077B64.c.o .text "$SIZE_C_77B64"
python3 "$TRIM" build/asm/disc1/68378.s.o .text "$SIZE_68378"
python3 "$TRIM" build/src/func_80077B84.c.o .text "$SIZE_C_77B84"
python3 "$TRIM" build/asm/disc1/68398.s.o .text "$SIZE_68398"
python3 "$TRIM" build/src/func_80077BA4.c.o .text "$SIZE_C_77BA4"
python3 "$TRIM" build/asm/disc1/683B8.s.o .text "$SIZE_683B8"
python3 "$TRIM" build/src/func_80077BC4.c.o .text "$SIZE_C_77BC4"
python3 "$TRIM" build/asm/disc1/683D8.s.o .text "$SIZE_683D8"
python3 "$TRIM" build/src/func_80077BE4.c.o .text "$SIZE_C_77BE4"
python3 "$TRIM" build/asm/disc1/683F8.s.o .text "$SIZE_683F8"
python3 "$TRIM" build/src/func_80077C04.c.o .text "$SIZE_C_77C04"
python3 "$TRIM" build/asm/disc1/68418.s.o .text "$SIZE_68418"
python3 "$TRIM" build/src/func_80077C24.c.o .text "$SIZE_C_77C24"
python3 "$TRIM" build/asm/disc1/68438.s.o .text "$SIZE_68438"
python3 "$TRIM" build/src/func_80077C44.c.o .text "$SIZE_C_77C44"
python3 "$TRIM" build/asm/disc1/68458.s.o .text "$SIZE_68458"
python3 "$TRIM" build/src/func_80077C64.c.o .text "$SIZE_C_77C64"
python3 "$TRIM" build/asm/disc1/68478.s.o .text "$SIZE_68478"
python3 "$TRIM" build/src/func_8007A324.c.o .text "$SIZE_C_7A324"
python3 "$TRIM" build/src/func_8007A334.c.o .text "$SIZE_C_7A334"
python3 "$TRIM" build/src/func_8007A344.c.o .text "$SIZE_C_7A344"
python3 "$TRIM" build/src/func_8007A354.c.o .text "$SIZE_C_7A354"
python3 "$TRIM" build/asm/disc1/6AB60.s.o .text "$SIZE_6AB60"
python3 "$TRIM" build/src/func_8007A3EC.c.o .text "$SIZE_C_7A3EC"
python3 "$TRIM" build/asm/disc1/6AC00.s.o .text "$SIZE_6AC00"
python3 "$TRIM" build/src/func_8007A4A8.c.o .text "$SIZE_C_7A4A8"
python3 "$TRIM" build/src/func_8007A4BC.c.o .text "$SIZE_C_7A4BC"
python3 "$TRIM" build/asm/disc1/6ACD0.s.o .text "$SIZE_6ACD0"
python3 "$TRIM" build/src/func_8007C130.c.o .text "$SIZE_C_7C130"
python3 "$TRIM" build/asm/disc1/6C93C.s.o .text "$SIZE_6C93C"
python3 "$TRIM" build/src/func_8007DEA4.c.o .text "$SIZE_C_7DEA4"
python3 "$TRIM" build/src/func_8007DEB0.c.o .text "$SIZE_C_7DEB0"
python3 "$TRIM" build/asm/disc1/6E6C0.s.o .text "$SIZE_6E6C0"
python3 "$TRIM" build/src/func_8007F778.c.o .text "$SIZE_C_7F778"
python3 "$TRIM" build/asm/disc1/6FF88.s.o .text "$SIZE_6FF88"
python3 "$TRIM" build/src/func_8007FBC0.c.o .text "$SIZE_C_7FBC0"
python3 "$TRIM" build/src/func_8007FBCC.c.o .text "$SIZE_C_7FBCC"
python3 "$TRIM" build/src/func_8007FBD8.c.o .text "$SIZE_C_7FBD8"
python3 "$TRIM" build/src/func_8007FBE4.c.o .text "$SIZE_C_7FBE4"
python3 "$TRIM" build/asm/disc1/703F0.s.o .text "$SIZE_703F0"
python3 "$TRIM" build/src/func_8007FC08.c.o .text "$SIZE_C_7FC08"
python3 "$TRIM" build/src/func_8007FC18.c.o .text "$SIZE_C_7FC18"
python3 "$TRIM" build/src/func_8007FC28.c.o .text "$SIZE_C_7FC28"
python3 "$TRIM" build/src/func_8007FC34.c.o .text "$SIZE_C_7FC34"
python3 "$TRIM" build/src/func_8007FC44.c.o .text "$SIZE_C_7FC44"
python3 "$TRIM" build/src/func_8007FC54.c.o .text "$SIZE_C_7FC54"
python3 "$TRIM" build/asm/disc1/70464.s.o .text "$SIZE_70464"
python3 "$TRIM" build/src/func_8007FCAC.c.o .text "$SIZE_C_7FCAC"
python3 "$TRIM" build/asm/disc1/704BC.s.o .text "$SIZE_704BC"
python3 "$TRIM" build/src/func_80080930.c.o .text "$SIZE_C_80930"
python3 "$TRIM" build/src/func_80080940.c.o .text "$SIZE_C_80940"
python3 "$TRIM" build/asm/disc1/71150.s.o .text "$SIZE_71150"
python3 "$TRIM" build/src/func_80080CC8.c.o .text "$SIZE_C_80CC8"
python3 "$TRIM" build/asm/disc1/714DC.s.o .text "$SIZE_714DC"
python3 "$TRIM" build/src/func_80081254.c.o .text "$SIZE_C_81254"
python3 "$TRIM" build/asm/disc1/71A68.s.o .text "$SIZE_71A68"
python3 "$TRIM" build/src/func_800822AC.c.o .text "$SIZE_C_822AC"
python3 "$TRIM" build/asm/disc1/72ABC.s.o .text "$SIZE_72ABC"
python3 "$TRIM" build/src/func_80082CDC.c.o .text "$SIZE_C_82CDC"
python3 "$TRIM" build/asm/disc1/734F0.s.o .text "$SIZE_734F0"
python3 "$TRIM" build/src/func_800835A4.c.o .text "$SIZE_C_835A4"
python3 "$TRIM" build/src/func_800835B0.c.o .text "$SIZE_C_835B0"
python3 "$TRIM" build/asm/disc1/73DC0.s.o .text "$SIZE_73DC0"
python3 "$TRIM" build/src/func_80083E70.c.o .text "$SIZE_C_83E70"
python3 "$TRIM" build/asm/disc1/74684.s.o .text "$SIZE_74684"
python3 "$TRIM" build/src/func_80083EE4.c.o .text "$SIZE_C_83EE4"
python3 "$TRIM" build/asm/disc1/746F8.s.o .text "$SIZE_746F8"
python3 "$TRIM" build/src/func_800847A0.c.o .text "$SIZE_C_847A0"
python3 "$TRIM" build/asm/disc1/74FB0.s.o .text "$SIZE_74FB0"
python3 "$TRIM" build/src/func_80085728.c.o .text "$SIZE_C_85728"
python3 "$TRIM" build/asm/disc1/75F44.s.o .text "$SIZE_75F44"
python3 "$TRIM" build/src/func_800870E0.c.o .text "$SIZE_C_870E0"
python3 "$TRIM" build/asm/disc1/778F0.s.o .text "$SIZE_778F0"
python3 "$TRIM" build/src/func_80087198.c.o .text "$SIZE_C_87198"
python3 "$TRIM" build/asm/disc1/779AC.s.o .text "$SIZE_779AC"
python3 "$TRIM" build/src/func_80087414.c.o .text "$SIZE_C_87414"
python3 "$TRIM" build/asm/disc1/77C28.s.o .text "$SIZE_77C28"
python3 "$TRIM" build/src/func_8008AB1C.c.o .text "$SIZE_C_8AB1C"
python3 "$TRIM" build/asm/disc1/7B39C.s.o .text "$SIZE_7B39C"
python3 "$TRIM" build/src/func_8008CA7C.c.o .text "$SIZE_C_8CA7C"
python3 "$TRIM" build/asm/disc1/7D284.s.o .text "$SIZE_7D284"
python3 "$TRIM" build/src/func_8008D7C0.c.o .text "$SIZE_C_8D7C0"
python3 "$TRIM" build/asm/disc1/7DFD0.s.o .text "$SIZE_7DFD0"
python3 "$TRIM" build/src/func_8008D820.c.o .text "$SIZE_C_8D820"
python3 "$TRIM" build/asm/disc1/7E044.s.o .text "$SIZE_7E044"
python3 "$TRIM" build/src/func_8008F694.c.o .text "$SIZE_C_8F694"
python3 "$TRIM" build/src/func_8008F6A8.c.o .text "$SIZE_C_8F6A8"
python3 "$TRIM" build/asm/disc1/7FEB0.s.o .text "$SIZE_7FEB0"
python3 "$TRIM" build/src/func_8008F868.c.o .text "$SIZE_C_8F868"
python3 "$TRIM" build/src/func_8008F880.c.o .text "$SIZE_C_8F880"
python3 "$TRIM" build/asm/disc1/80098.s.o .text "$SIZE_80098"
python3 "$TRIM" build/src/func_8008FCB4.c.o .text "$SIZE_C_8FCB4"
python3 "$TRIM" build/asm/disc1/804BC.s.o .text "$SIZE_804BC"
python3 "$TRIM" build/src/func_800904A0.c.o .text "$SIZE_C_904A0"
python3 "$TRIM" build/src/func_800904AC.c.o .text "$SIZE_C_904AC"
python3 "$TRIM" build/src/func_800904B4.c.o .text "$SIZE_C_904B4"
python3 "$TRIM" build/src/func_800904BC.c.o .text "$SIZE_C_904BC"
python3 "$TRIM" build/asm/disc1/80CC4.s.o .text "$SIZE_80CC4"
python3 "$TRIM" build/src/func_800906B4.c.o .text "$SIZE_C_906B4"
python3 "$TRIM" build/asm/disc1/80EE4.s.o .text "$SIZE_80EE4"
python3 "$TRIM" build/src/func_80090A0C.c.o .text "$SIZE_C_90A0C"
python3 "$TRIM" build/asm/disc1/81220.s.o .text "$SIZE_81220"
python3 "$TRIM" build/src/func_80090C38.c.o .text "$SIZE_C_90C38"
python3 "$TRIM" build/src/func_80090C4C.c.o .text "$SIZE_C_90C4C"
python3 "$TRIM" build/src/func_80090C60.c.o .text "$SIZE_C_90C60"
python3 "$TRIM" build/src/func_80090C74.c.o .text "$SIZE_C_90C74"
python3 "$TRIM" build/asm/disc1/81488.s.o .text "$SIZE_81488"
python3 "$TRIM" build/src/func_80090F54.c.o .text "$SIZE_C_90F54"
python3 "$TRIM" build/asm/disc1/81768.s.o .text "$SIZE_81768"
python3 "$TRIM" build/asm/disc1/data/818A0.rodata.s.o .rodata "$SIZE_818A0_RODATA"
python3 "$TRIM" build/asm/disc1/B2AF8.s.o .text "$SIZE_B2AF8"
python3 "$TRIM" build/src/func_800C2B40.c.o .text "$SIZE_C_C2B40"
python3 "$TRIM" build/src/func_800C2B50.c.o .text "$SIZE_C_C2B50"
python3 "$TRIM" build/asm/disc1/B3368.s.o .text "$SIZE_B3368"
python3 "$TRIM" build/src/func_800C7DC4.c.o .text "$SIZE_C_C7DC4"
python3 "$TRIM" build/src/func_800C7DD4.c.o .text "$SIZE_C_C7DD4"
python3 "$TRIM" build/src/func_800C7DDC.c.o .text "$SIZE_C_C7DDC"
python3 "$TRIM" build/asm/disc1/B85E4.s.o .text "$SIZE_B85E4"
python3 "$TRIM" build/src/func_800C8268.c.o .text "$SIZE_C_C8268"
python3 "$TRIM" build/asm/disc1/B8A70.s.o .text "$SIZE_B8A70"
python3 "$TRIM" build/src/func_800C8BB4.c.o .text "$SIZE_C_C8BB4"
python3 "$TRIM" build/asm/disc1/B93C0.s.o .text "$SIZE_B93C0"
python3 "$TRIM" build/src/func_800C8F08.c.o .text "$SIZE_C_C8F08"
python3 "$TRIM" build/src/func_800C8F18.c.o .text "$SIZE_C_C8F18"
python3 "$TRIM" build/src/func_800C8F20.c.o .text "$SIZE_C_C8F20"
python3 "$TRIM" build/asm/disc1/B9728.s.o .text "$SIZE_B9728"
python3 "$TRIM" build/src/func_800C9260.c.o .text "$SIZE_C_C9260"
python3 "$TRIM" build/asm/disc1/B9A68.s.o .text "$SIZE_B9A68"
python3 "$TRIM" build/src/func_800C9968.c.o .text "$SIZE_C_C9968"
python3 "$TRIM" build/asm/disc1/BA174.s.o .text "$SIZE_BA174"
python3 "$TRIM" build/src/func_800C9C00.c.o .text "$SIZE_C_C9C00"
python3 "$TRIM" build/asm/disc1/BA410.s.o .text "$SIZE_BA410"
python3 "$TRIM" build/src/func_800C9EA0.c.o .text "$SIZE_C_C9EA0"
python3 "$TRIM" build/asm/disc1/BA6A8.s.o .text "$SIZE_BA6A8"
python3 "$TRIM" build/src/func_800CA4A8.c.o .text "$SIZE_C_CA4A8"
python3 "$TRIM" build/asm/disc1/BACB4.s.o .text "$SIZE_BACB4"
python3 "$TRIM" build/src/func_800CA798.c.o .text "$SIZE_C_CA798"
python3 "$TRIM" build/asm/disc1/BAFA8.s.o .text "$SIZE_BAFA8"
python3 "$TRIM" build/src/func_800CACD4.c.o .text "$SIZE_C_CACD4"
python3 "$TRIM" build/asm/disc1/BB4DC.s.o .text "$SIZE_BB4DC"
python3 "$TRIM" build/src/func_800CBB24.c.o .text "$SIZE_C_CBB24"
python3 "$TRIM" build/asm/disc1/BC330.s.o .text "$SIZE_BC330"
python3 "$TRIM" build/src/func_800CBFA4.c.o .text "$SIZE_C_CBFA4"
python3 "$TRIM" build/asm/disc1/BC7B4.s.o .text "$SIZE_BC7B4"
python3 "$TRIM" build/src/func_800CCF80.c.o .text "$SIZE_C_CCF80"
python3 "$TRIM" build/asm/disc1/BD790.s.o .text "$SIZE_BD790"
python3 "$TRIM" build/src/func_800CD2DC.c.o .text "$SIZE_C_CD2DC"
python3 "$TRIM" build/src/func_800CD2E4.c.o .text "$SIZE_C_CD2E4"
python3 "$TRIM" build/asm/disc1/BDAEC.s.o .text "$SIZE_BDAEC"
python3 "$TRIM" build/src/func_800CD59C.c.o .text "$SIZE_C_CD59C"
python3 "$TRIM" build/src/func_800CD5A4.c.o .text "$SIZE_C_CD5A4"
python3 "$TRIM" build/asm/disc1/BDDB0.s.o .text "$SIZE_BDDB0"
python3 "$TRIM" build/src/func_800CD71C.c.o .text "$SIZE_C_CD71C"
python3 "$TRIM" build/asm/disc1/BDF28.s.o .text "$SIZE_BDF28"
python3 "$TRIM" build/src/func_800CD960.c.o .text "$SIZE_C_CD960"
python3 "$TRIM" build/asm/disc1/BE170.s.o .text "$SIZE_BE170"
python3 "$TRIM" build/src/func_800CDD04.c.o .text "$SIZE_C_CDD04"
python3 "$TRIM" build/asm/disc1/BE50C.s.o .text "$SIZE_BE50C"
python3 "$TRIM" build/src/func_800CDF40.c.o .text "$SIZE_C_CDF40"
python3 "$TRIM" build/asm/disc1/BE74C.s.o .text "$SIZE_BE74C"
python3 "$TRIM" build/src/func_800CE1DC.c.o .text "$SIZE_C_CE1DC"
python3 "$TRIM" build/asm/disc1/BE9EC.s.o .text "$SIZE_BE9EC"
python3 "$TRIM" build/src/func_800CE3AC.c.o .text "$SIZE_C_CE3AC"
python3 "$TRIM" build/asm/disc1/BEBB4.s.o .text "$SIZE_BEBB4"
python3 "$TRIM" build/src/func_800CE464.c.o .text "$SIZE_C_CE464"
python3 "$TRIM" build/asm/disc1/BEC70.s.o .text "$SIZE_BEC70"
python3 "$TRIM" build/src/func_800D4850.c.o .text "$SIZE_C_D4850"
python3 "$TRIM" build/asm/disc1/C5060.s.o .text "$SIZE_C5060"
python3 "$TRIM" build/asm/disc1/header.s.o .data 0x800 || true

if [[ "$MODE" == "assemble" ]]; then
    echo
    echo "Assemble/compile-only complete (with section-pad trim)."
    echo "Objects under build/asm/ and build/src/ (git-ignored)."
    echo "Matching/rebuild: NOT claimed (link/pack not run)."
    exit 0
fi

# --- absolute symbol scripts for link ---
step "Prepare absolute symbols for link"
# splat auto lists (scratchpad, kernel, out-of-image, etc.)
ABS_LD="build/abs_syms.ld"
{
    if [[ -f undefined_syms_auto.txt ]]; then cat undefined_syms_auto.txt; fi
    if [[ -f undefined_funcs_auto.txt ]]; then cat undefined_funcs_auto.txt; fi
    # null jtbl terminator used by spimdisasm in this split
    echo ".L00000000_main = 0;"
    echo "_gp = 0x8009CD70;"
} >"$ABS_LD"

# ROM-order link: PE1 image is interleaved (prefix rodata, main text, mid
# rodata, tail text). splat's linkers/disc1.ld uses C layout (all .text then
# all .rodata) and is not used for the production pack.
ROM_ORDER_LD="build/disc1_romorder.ld"
cat >"$ROM_ORDER_LD" <<'LDEOF'
/* Phase 5EF ROM-order link script (207 C leaves (incl. gp batches + era + 5EF + 5EG + 5EH)).
 * splat's linkers/disc1.ld places all .text then all .rodata (C layout).
 * PE1 image order is interleaved: prefix rodata, main text (with C leaves),
 * mid rodata, tail text (with C leaf).
 * This script places inputs in file order for PS-X EXE packing.
 */
SECTIONS
{
    .header : AT(0)
    {
        build/asm/disc1/header.s.o(.data)
    }

    .main 0x80010000 : AT(0x800)
    {
        build/asm/disc1/data/800.rodata.s.o(.rodata)
        build/asm/disc1/2A0C.s.o(.text)
        build/src/func_80017E9C.c.o(.text)
        build/asm/disc1/86A4.s.o(.text)
        build/src/func_80017FDC.c.o(.text)
        build/src/func_80017FF0.c.o(.text)
        build/asm/disc1/8804.s.o(.text)
        build/src/func_80019050.c.o(.text)
        build/src/func_80019058.c.o(.text)
        build/asm/disc1/9860.s.o(.text)
        build/src/func_800190AC.c.o(.text)
        build/src/func_800190B4.c.o(.text)
        build/asm/disc1/98BC.s.o(.text)
        build/src/func_800192B8.c.o(.text)
        build/src/func_800192C8.c.o(.text)
        build/asm/disc1/9ADC.s.o(.text)
        build/src/func_80019BE4.c.o(.text)
        build/asm/disc1/A404.s.o(.text)
        build/src/func_80020EFC.c.o(.text)
        build/asm/disc1/11718.s.o(.text)
        build/src/func_80033A20.c.o(.text)
        build/asm/disc1/2422C.s.o(.text)
        build/src/func_800371A4.c.o(.text)
        build/asm/disc1/279B0.s.o(.text)
        build/src/func_80037454.c.o(.text)
        build/asm/disc1/27C6C.s.o(.text)
        build/src/func_800375B4.c.o(.text)
        build/src/func_800375C4.c.o(.text)
        build/src/func_800375D0.c.o(.text)
        build/asm/disc1/27DE0.s.o(.text)
        build/src/func_80037864.c.o(.text)
        build/asm/disc1/28070.s.o(.text)
        build/src/func_80038940.c.o(.text)
        build/asm/disc1/29154.s.o(.text)
        build/src/func_80038D0C.c.o(.text)
        build/asm/disc1/2951C.s.o(.text)
        build/src/func_8003D82C.c.o(.text)
        build/asm/disc1/2E034.s.o(.text)
        build/src/func_8003DFC8.c.o(.text)
        build/src/func_8003DFD0.c.o(.text)
        build/asm/disc1/2E7D8.s.o(.text)
        build/src/func_8003FFAC.c.o(.text)
        build/src/func_8003FFBC.c.o(.text)
        build/asm/disc1/307CC.s.o(.text)
        build/src/func_800428C4.c.o(.text)
        build/asm/disc1/330D4.s.o(.text)
        build/src/func_80042910.c.o(.text)
        build/asm/disc1/33128.s.o(.text)
        build/src/func_80042B28.c.o(.text)
        build/src/func_80042B38.c.o(.text)
        build/src/func_80042B50.c.o(.text)
        build/src/func_80042B6C.c.o(.text)
        build/src/func_80042BC8.c.o(.text)
        build/src/func_80042BD8.c.o(.text)
        build/src/func_80042BEC.c.o(.text)
        build/src/func_80042C00.c.o(.text)
        build/src/func_80042C14.c.o(.text)
        build/src/func_80042C28.c.o(.text)
        build/src/func_80042C3C.c.o(.text)
        build/src/func_80042C50.c.o(.text)
        build/src/func_80042C64.c.o(.text)
        build/asm/disc1/33478.s.o(.text)
        build/src/func_80042CB8.c.o(.text)
        build/asm/disc1/334C4.s.o(.text)
        build/src/func_80042ED0.c.o(.text)
        build/asm/disc1/336DC.s.o(.text)
        build/src/func_80042F20.c.o(.text)
        build/src/func_80042F38.c.o(.text)
        build/asm/disc1/33744.s.o(.text)
        build/src/func_80043038.c.o(.text)
        build/asm/disc1/3384C.s.o(.text)
        build/src/func_80043240.c.o(.text)
        build/asm/disc1/33A4C.s.o(.text)
        build/src/func_800438C0.c.o(.text)
        build/src/func_800438E0.c.o(.text)
        build/asm/disc1/340EC.s.o(.text)
        build/src/func_8004D024.c.o(.text)
        build/asm/disc1/3D830.s.o(.text)
        build/src/func_8004D27C.c.o(.text)
        build/src/func_8004D288.c.o(.text)
        build/asm/disc1/3DA98.s.o(.text)
        build/src/func_8004DA9C.c.o(.text)
        build/asm/disc1/3E2A4.s.o(.text)
        build/src/func_8004E970.c.o(.text)
        build/asm/disc1/3F17C.s.o(.text)
        build/src/func_8004F448.c.o(.text)
        build/asm/disc1/3FC64.s.o(.text)
        build/src/func_8004F808.c.o(.text)
        build/asm/disc1/40038.s.o(.text)
        build/src/func_80050D18.c.o(.text)
        build/asm/disc1/41520.s.o(.text)
        build/src/func_80051084.c.o(.text)
        build/asm/disc1/41898.s.o(.text)
        build/src/func_80051244.c.o(.text)
        build/asm/disc1/41A58.s.o(.text)
        build/src/func_800514F8.c.o(.text)
        build/src/func_80051504.c.o(.text)
        build/asm/disc1/41D10.s.o(.text)
        build/src/func_80051834.c.o(.text)
        build/asm/disc1/4204C.s.o(.text)
        build/src/func_80051E48.c.o(.text)
        build/src/func_80051E58.c.o(.text)
        build/asm/disc1/42664.s.o(.text)
        build/src/func_80052514.c.o(.text)
        build/src/func_80052524.c.o(.text)
        build/asm/disc1/42D34.s.o(.text)
        build/src/func_8005257C.c.o(.text)
        build/asm/disc1/42D94.s.o(.text)
        build/src/func_800527B4.c.o(.text)
        build/src/func_800527C0.c.o(.text)
        build/asm/disc1/42FC8.s.o(.text)
        build/src/func_80052EB0.c.o(.text)
        build/asm/disc1/436C0.s.o(.text)
        build/src/func_80052F0C.c.o(.text)
        build/asm/disc1/43724.s.o(.text)
        build/src/func_80054288.c.o(.text)
        build/src/func_80054294.c.o(.text)
        build/asm/disc1/44AA0.s.o(.text)
        build/src/func_80057ECC.c.o(.text)
        build/asm/disc1/486D8.s.o(.text)
        build/src/func_8005B890.c.o(.text)
        build/src/func_8005B89C.c.o(.text)
        build/asm/disc1/4C0A8.s.o(.text)
        build/src/func_8005BC98.c.o(.text)
        build/src/func_8005BCA8.c.o(.text)
        build/src/func_8005BCB0.c.o(.text)
        build/asm/disc1/4C4BC.s.o(.text)
        build/src/func_8005BEDC.c.o(.text)
        build/asm/disc1/4C6E8.s.o(.text)
        build/src/func_8005C488.c.o(.text)
        build/asm/disc1/4CC98.s.o(.text)
        build/src/func_8005E114.c.o(.text)
        build/src/func_8005E120.c.o(.text)
        build/asm/disc1/4E92C.s.o(.text)
        build/src/func_8005E57C.c.o(.text)
        build/asm/disc1/4ED88.s.o(.text)
        build/src/func_8005E6E4.c.o(.text)
        build/asm/disc1/4EEF0.s.o(.text)
        build/src/func_8005E884.c.o(.text)
        build/src/func_8005E894.c.o(.text)
        build/asm/disc1/4F0A4.s.o(.text)
        build/src/func_8005EB58.c.o(.text)
        build/asm/disc1/4F364.s.o(.text)
        build/src/func_8005EEC8.c.o(.text)
        build/asm/disc1/4F6D4.s.o(.text)
        build/src/func_800614A0.c.o(.text)
        build/asm/disc1/51CAC.s.o(.text)
        build/src/func_800622B0.c.o(.text)
        build/asm/disc1/52ABC.s.o(.text)
        build/src/func_800629B0.c.o(.text)
        build/asm/disc1/531BC.s.o(.text)
        build/src/func_80062CB8.c.o(.text)
        build/src/func_80062CC4.c.o(.text)
        build/src/func_80062CD0.c.o(.text)
        build/asm/disc1/534E4.s.o(.text)
        build/src/func_80063198.c.o(.text)
        build/src/func_800631AC.c.o(.text)
        build/asm/disc1/539C0.s.o(.text)
        build/src/func_80064A48.c.o(.text)
        build/asm/disc1/55254.s.o(.text)
        build/src/func_80064C20.c.o(.text)
        build/asm/disc1/55430.s.o(.text)
        build/src/func_8006EBD4.c.o(.text)
        build/asm/disc1/5F3E4.s.o(.text)
        build/src/func_80073DE8.c.o(.text)
        build/src/func_80073DF8.c.o(.text)
        build/asm/disc1/64610.s.o(.text)
        build/src/func_80074330.c.o(.text)
        build/asm/disc1/64B54.s.o(.text)
        build/src/func_800744A4.c.o(.text)
        build/asm/disc1/64CC8.s.o(.text)
        build/src/func_8007474C.c.o(.text)
        build/asm/disc1/64F70.s.o(.text)
        build/src/func_80074A14.c.o(.text)
        build/src/func_80074A28.c.o(.text)
        build/asm/disc1/65238.s.o(.text)
        build/src/func_80074CB8.c.o(.text)
        build/asm/disc1/654C8.s.o(.text)
        build/src/func_8007633C.c.o(.text)
        build/asm/disc1/66B54.s.o(.text)
        build/src/func_80077A28.c.o(.text)
        build/asm/disc1/6824C.s.o(.text)
        build/src/func_80077B64.c.o(.text)
        build/asm/disc1/68378.s.o(.text)
        build/src/func_80077B84.c.o(.text)
        build/asm/disc1/68398.s.o(.text)
        build/src/func_80077BA4.c.o(.text)
        build/asm/disc1/683B8.s.o(.text)
        build/src/func_80077BC4.c.o(.text)
        build/asm/disc1/683D8.s.o(.text)
        build/src/func_80077BE4.c.o(.text)
        build/asm/disc1/683F8.s.o(.text)
        build/src/func_80077C04.c.o(.text)
        build/asm/disc1/68418.s.o(.text)
        build/src/func_80077C24.c.o(.text)
        build/asm/disc1/68438.s.o(.text)
        build/src/func_80077C44.c.o(.text)
        build/asm/disc1/68458.s.o(.text)
        build/src/func_80077C64.c.o(.text)
        build/asm/disc1/68478.s.o(.text)
        build/src/func_8007A324.c.o(.text)
        build/src/func_8007A334.c.o(.text)
        build/src/func_8007A344.c.o(.text)
        build/src/func_8007A354.c.o(.text)
        build/asm/disc1/6AB60.s.o(.text)
        build/src/func_8007A3EC.c.o(.text)
        build/asm/disc1/6AC00.s.o(.text)
        build/src/func_8007A4A8.c.o(.text)
        build/src/func_8007A4BC.c.o(.text)
        build/asm/disc1/6ACD0.s.o(.text)
        build/src/func_8007C130.c.o(.text)
        build/asm/disc1/6C93C.s.o(.text)
        build/src/func_8007DEA4.c.o(.text)
        build/src/func_8007DEB0.c.o(.text)
        build/asm/disc1/6E6C0.s.o(.text)
        build/src/func_8007F778.c.o(.text)
        build/asm/disc1/6FF88.s.o(.text)
        build/src/func_8007FBC0.c.o(.text)
        build/src/func_8007FBCC.c.o(.text)
        build/src/func_8007FBD8.c.o(.text)
        build/src/func_8007FBE4.c.o(.text)
        build/asm/disc1/703F0.s.o(.text)
        build/src/func_8007FC08.c.o(.text)
        build/src/func_8007FC18.c.o(.text)
        build/src/func_8007FC28.c.o(.text)
        build/src/func_8007FC34.c.o(.text)
        build/src/func_8007FC44.c.o(.text)
        build/src/func_8007FC54.c.o(.text)
        build/asm/disc1/70464.s.o(.text)
        build/src/func_8007FCAC.c.o(.text)
        build/asm/disc1/704BC.s.o(.text)
        build/src/func_80080930.c.o(.text)
        build/src/func_80080940.c.o(.text)
        build/asm/disc1/71150.s.o(.text)
        build/src/func_80080CC8.c.o(.text)
        build/asm/disc1/714DC.s.o(.text)
        build/src/func_80081254.c.o(.text)
        build/asm/disc1/71A68.s.o(.text)
        build/src/func_800822AC.c.o(.text)
        build/asm/disc1/72ABC.s.o(.text)
        build/src/func_80082CDC.c.o(.text)
        build/asm/disc1/734F0.s.o(.text)
        build/src/func_800835A4.c.o(.text)
        build/src/func_800835B0.c.o(.text)
        build/asm/disc1/73DC0.s.o(.text)
        build/src/func_80083E70.c.o(.text)
        build/asm/disc1/74684.s.o(.text)
        build/src/func_80083EE4.c.o(.text)
        build/asm/disc1/746F8.s.o(.text)
        build/src/func_800847A0.c.o(.text)
        build/asm/disc1/74FB0.s.o(.text)
        build/src/func_80085728.c.o(.text)
        build/asm/disc1/75F44.s.o(.text)
        build/src/func_800870E0.c.o(.text)
        build/asm/disc1/778F0.s.o(.text)
        build/src/func_80087198.c.o(.text)
        build/asm/disc1/779AC.s.o(.text)
        build/src/func_80087414.c.o(.text)
        build/asm/disc1/77C28.s.o(.text)
        build/src/func_8008AB1C.c.o(.text)
        build/asm/disc1/7B39C.s.o(.text)
        build/src/func_8008CA7C.c.o(.text)
        build/asm/disc1/7D284.s.o(.text)
        build/src/func_8008D7C0.c.o(.text)
        build/asm/disc1/7DFD0.s.o(.text)
        build/src/func_8008D820.c.o(.text)
        build/asm/disc1/7E044.s.o(.text)
        build/src/func_8008F694.c.o(.text)
        build/src/func_8008F6A8.c.o(.text)
        build/asm/disc1/7FEB0.s.o(.text)
        build/src/func_8008F868.c.o(.text)
        build/src/func_8008F880.c.o(.text)
        build/asm/disc1/80098.s.o(.text)
        build/src/func_8008FCB4.c.o(.text)
        build/asm/disc1/804BC.s.o(.text)
        build/src/func_800904A0.c.o(.text)
        build/src/func_800904AC.c.o(.text)
        build/src/func_800904B4.c.o(.text)
        build/src/func_800904BC.c.o(.text)
        build/asm/disc1/80CC4.s.o(.text)
        build/src/func_800906B4.c.o(.text)
        build/asm/disc1/80EE4.s.o(.text)
        build/src/func_80090A0C.c.o(.text)
        build/asm/disc1/81220.s.o(.text)
        build/src/func_80090C38.c.o(.text)
        build/src/func_80090C4C.c.o(.text)
        build/src/func_80090C60.c.o(.text)
        build/src/func_80090C74.c.o(.text)
        build/asm/disc1/81488.s.o(.text)
        build/src/func_80090F54.c.o(.text)
        build/asm/disc1/81768.s.o(.text)
        build/asm/disc1/data/818A0.rodata.s.o(.rodata)
        build/asm/disc1/B2AF8.s.o(.text)
        build/src/func_800C2B40.c.o(.text)
        build/src/func_800C2B50.c.o(.text)
        build/asm/disc1/B3368.s.o(.text)
        build/src/func_800C7DC4.c.o(.text)
        build/src/func_800C7DD4.c.o(.text)
        build/src/func_800C7DDC.c.o(.text)
        build/asm/disc1/B85E4.s.o(.text)
        build/src/func_800C8268.c.o(.text)
        build/asm/disc1/B8A70.s.o(.text)
        build/src/func_800C8BB4.c.o(.text)
        build/asm/disc1/B93C0.s.o(.text)
        build/src/func_800C8F08.c.o(.text)
        build/src/func_800C8F18.c.o(.text)
        build/src/func_800C8F20.c.o(.text)
        build/asm/disc1/B9728.s.o(.text)
        build/src/func_800C9260.c.o(.text)
        build/asm/disc1/B9A68.s.o(.text)
        build/src/func_800C9968.c.o(.text)
        build/asm/disc1/BA174.s.o(.text)
        build/src/func_800C9C00.c.o(.text)
        build/asm/disc1/BA410.s.o(.text)
        build/src/func_800C9EA0.c.o(.text)
        build/asm/disc1/BA6A8.s.o(.text)
        build/src/func_800CA4A8.c.o(.text)
        build/asm/disc1/BACB4.s.o(.text)
        build/src/func_800CA798.c.o(.text)
        build/asm/disc1/BAFA8.s.o(.text)
        build/src/func_800CACD4.c.o(.text)
        build/asm/disc1/BB4DC.s.o(.text)
        build/src/func_800CBB24.c.o(.text)
        build/asm/disc1/BC330.s.o(.text)
        build/src/func_800CBFA4.c.o(.text)
        build/asm/disc1/BC7B4.s.o(.text)
        build/src/func_800CCF80.c.o(.text)
        build/asm/disc1/BD790.s.o(.text)
        build/src/func_800CD2DC.c.o(.text)
        build/src/func_800CD2E4.c.o(.text)
        build/asm/disc1/BDAEC.s.o(.text)
        build/src/func_800CD59C.c.o(.text)
        build/src/func_800CD5A4.c.o(.text)
        build/asm/disc1/BDDB0.s.o(.text)
        build/src/func_800CD71C.c.o(.text)
        build/asm/disc1/BDF28.s.o(.text)
        build/src/func_800CD960.c.o(.text)
        build/asm/disc1/BE170.s.o(.text)
        build/src/func_800CDD04.c.o(.text)
        build/asm/disc1/BE50C.s.o(.text)
        build/src/func_800CDF40.c.o(.text)
        build/asm/disc1/BE74C.s.o(.text)
        build/src/func_800CE1DC.c.o(.text)
        build/asm/disc1/BE9EC.s.o(.text)
        build/src/func_800CE3AC.c.o(.text)
        build/asm/disc1/BEBB4.s.o(.text)
        build/src/func_800CE464.c.o(.text)
        build/asm/disc1/BEC70.s.o(.text)
        build/src/func_800D4850.c.o(.text)
        build/asm/disc1/C5060.s.o(.text)
        build/asm/disc1/2A0C.s.o(.data)
        build/src/func_80017E9C.c.o(.data)
        build/asm/disc1/86A4.s.o(.data)
        build/src/func_80017FDC.c.o(.data)
        build/src/func_80017FF0.c.o(.data)
        build/asm/disc1/8804.s.o(.data)
        build/src/func_80019050.c.o(.data)
        build/src/func_80019058.c.o(.data)
        build/asm/disc1/9860.s.o(.data)
        build/src/func_800190AC.c.o(.data)
        build/src/func_800190B4.c.o(.data)
        build/asm/disc1/98BC.s.o(.data)
        build/src/func_800192B8.c.o(.data)
        build/src/func_800192C8.c.o(.data)
        build/asm/disc1/9ADC.s.o(.data)
        build/src/func_80019BE4.c.o(.data)
        build/asm/disc1/A404.s.o(.data)
        build/src/func_80020EFC.c.o(.data)
        build/asm/disc1/11718.s.o(.data)
        build/src/func_80033A20.c.o(.data)
        build/asm/disc1/2422C.s.o(.data)
        build/src/func_800371A4.c.o(.data)
        build/asm/disc1/279B0.s.o(.data)
        build/src/func_80037454.c.o(.data)
        build/asm/disc1/27C6C.s.o(.data)
        build/src/func_800375B4.c.o(.data)
        build/src/func_800375C4.c.o(.data)
        build/src/func_800375D0.c.o(.data)
        build/asm/disc1/27DE0.s.o(.data)
        build/src/func_80037864.c.o(.data)
        build/asm/disc1/28070.s.o(.data)
        build/src/func_80038940.c.o(.data)
        build/asm/disc1/29154.s.o(.data)
        build/src/func_80038D0C.c.o(.data)
        build/asm/disc1/2951C.s.o(.data)
        build/src/func_8003D82C.c.o(.data)
        build/asm/disc1/2E034.s.o(.data)
        build/src/func_8003DFC8.c.o(.data)
        build/src/func_8003DFD0.c.o(.data)
        build/asm/disc1/2E7D8.s.o(.data)
        build/src/func_8003FFAC.c.o(.data)
        build/src/func_8003FFBC.c.o(.data)
        build/asm/disc1/307CC.s.o(.data)
        build/src/func_800428C4.c.o(.data)
        build/asm/disc1/330D4.s.o(.data)
        build/src/func_80042910.c.o(.data)
        build/asm/disc1/33128.s.o(.data)
        build/src/func_80042B28.c.o(.data)
        build/src/func_80042B38.c.o(.data)
        build/src/func_80042B50.c.o(.data)
        build/src/func_80042B6C.c.o(.data)
        build/src/func_80042BC8.c.o(.data)
        build/src/func_80042BD8.c.o(.data)
        build/src/func_80042BEC.c.o(.data)
        build/src/func_80042C00.c.o(.data)
        build/src/func_80042C14.c.o(.data)
        build/src/func_80042C28.c.o(.data)
        build/src/func_80042C3C.c.o(.data)
        build/src/func_80042C50.c.o(.data)
        build/src/func_80042C64.c.o(.data)
        build/asm/disc1/33478.s.o(.data)
        build/src/func_80042CB8.c.o(.data)
        build/asm/disc1/334C4.s.o(.data)
        build/src/func_80042ED0.c.o(.data)
        build/asm/disc1/336DC.s.o(.data)
        build/src/func_80042F20.c.o(.data)
        build/src/func_80042F38.c.o(.data)
        build/asm/disc1/33744.s.o(.data)
        build/src/func_80043038.c.o(.data)
        build/asm/disc1/3384C.s.o(.data)
        build/src/func_80043240.c.o(.data)
        build/asm/disc1/33A4C.s.o(.data)
        build/src/func_800438C0.c.o(.data)
        build/src/func_800438E0.c.o(.data)
        build/asm/disc1/340EC.s.o(.data)
        build/src/func_8004D024.c.o(.data)
        build/asm/disc1/3D830.s.o(.data)
        build/src/func_8004D27C.c.o(.data)
        build/src/func_8004D288.c.o(.data)
        build/asm/disc1/3DA98.s.o(.data)
        build/src/func_8004DA9C.c.o(.data)
        build/asm/disc1/3E2A4.s.o(.data)
        build/src/func_8004E970.c.o(.data)
        build/asm/disc1/3F17C.s.o(.data)
        build/src/func_8004F448.c.o(.data)
        build/asm/disc1/3FC64.s.o(.data)
        build/src/func_8004F808.c.o(.data)
        build/asm/disc1/40038.s.o(.data)
        build/src/func_80050D18.c.o(.data)
        build/asm/disc1/41520.s.o(.data)
        build/src/func_80051084.c.o(.data)
        build/asm/disc1/41898.s.o(.data)
        build/src/func_80051244.c.o(.data)
        build/asm/disc1/41A58.s.o(.data)
        build/src/func_800514F8.c.o(.data)
        build/src/func_80051504.c.o(.data)
        build/asm/disc1/41D10.s.o(.data)
        build/src/func_80051834.c.o(.data)
        build/asm/disc1/4204C.s.o(.data)
        build/src/func_80051E48.c.o(.data)
        build/src/func_80051E58.c.o(.data)
        build/asm/disc1/42664.s.o(.data)
        build/src/func_80052514.c.o(.data)
        build/src/func_80052524.c.o(.data)
        build/asm/disc1/42D34.s.o(.data)
        build/src/func_8005257C.c.o(.data)
        build/asm/disc1/42D94.s.o(.data)
        build/src/func_800527B4.c.o(.data)
        build/src/func_800527C0.c.o(.data)
        build/asm/disc1/42FC8.s.o(.data)
        build/src/func_80052EB0.c.o(.data)
        build/asm/disc1/436C0.s.o(.data)
        build/src/func_80052F0C.c.o(.data)
        build/asm/disc1/43724.s.o(.data)
        build/src/func_80054288.c.o(.data)
        build/src/func_80054294.c.o(.data)
        build/asm/disc1/44AA0.s.o(.data)
        build/src/func_80057ECC.c.o(.data)
        build/asm/disc1/486D8.s.o(.data)
        build/src/func_8005B890.c.o(.data)
        build/src/func_8005B89C.c.o(.data)
        build/asm/disc1/4C0A8.s.o(.data)
        build/src/func_8005BC98.c.o(.data)
        build/src/func_8005BCA8.c.o(.data)
        build/src/func_8005BCB0.c.o(.data)
        build/asm/disc1/4C4BC.s.o(.data)
        build/src/func_8005BEDC.c.o(.data)
        build/asm/disc1/4C6E8.s.o(.data)
        build/src/func_8005C488.c.o(.data)
        build/asm/disc1/4CC98.s.o(.data)
        build/src/func_8005E114.c.o(.data)
        build/src/func_8005E120.c.o(.data)
        build/asm/disc1/4E92C.s.o(.data)
        build/src/func_8005E57C.c.o(.data)
        build/asm/disc1/4ED88.s.o(.data)
        build/src/func_8005E6E4.c.o(.data)
        build/asm/disc1/4EEF0.s.o(.data)
        build/src/func_8005E884.c.o(.data)
        build/src/func_8005E894.c.o(.data)
        build/asm/disc1/4F0A4.s.o(.data)
        build/src/func_8005EB58.c.o(.data)
        build/asm/disc1/4F364.s.o(.data)
        build/src/func_8005EEC8.c.o(.data)
        build/asm/disc1/4F6D4.s.o(.data)
        build/src/func_800614A0.c.o(.data)
        build/asm/disc1/51CAC.s.o(.data)
        build/src/func_800622B0.c.o(.data)
        build/asm/disc1/52ABC.s.o(.data)
        build/src/func_800629B0.c.o(.data)
        build/asm/disc1/531BC.s.o(.data)
        build/src/func_80062CB8.c.o(.data)
        build/src/func_80062CC4.c.o(.data)
        build/src/func_80062CD0.c.o(.data)
        build/asm/disc1/534E4.s.o(.data)
        build/src/func_80063198.c.o(.data)
        build/src/func_800631AC.c.o(.data)
        build/asm/disc1/539C0.s.o(.data)
        build/src/func_80064A48.c.o(.data)
        build/asm/disc1/55254.s.o(.data)
        build/src/func_80064C20.c.o(.data)
        build/asm/disc1/55430.s.o(.data)
        build/src/func_8006EBD4.c.o(.data)
        build/asm/disc1/5F3E4.s.o(.data)
        build/src/func_80073DE8.c.o(.data)
        build/src/func_80073DF8.c.o(.data)
        build/asm/disc1/64610.s.o(.data)
        build/src/func_80074330.c.o(.data)
        build/asm/disc1/64B54.s.o(.data)
        build/src/func_800744A4.c.o(.data)
        build/asm/disc1/64CC8.s.o(.data)
        build/src/func_8007474C.c.o(.data)
        build/asm/disc1/64F70.s.o(.data)
        build/src/func_80074A14.c.o(.data)
        build/src/func_80074A28.c.o(.data)
        build/asm/disc1/65238.s.o(.data)
        build/src/func_80074CB8.c.o(.data)
        build/asm/disc1/654C8.s.o(.data)
        build/src/func_8007633C.c.o(.data)
        build/asm/disc1/66B54.s.o(.data)
        build/src/func_80077A28.c.o(.data)
        build/asm/disc1/6824C.s.o(.data)
        build/src/func_80077B64.c.o(.data)
        build/asm/disc1/68378.s.o(.data)
        build/src/func_80077B84.c.o(.data)
        build/asm/disc1/68398.s.o(.data)
        build/src/func_80077BA4.c.o(.data)
        build/asm/disc1/683B8.s.o(.data)
        build/src/func_80077BC4.c.o(.data)
        build/asm/disc1/683D8.s.o(.data)
        build/src/func_80077BE4.c.o(.data)
        build/asm/disc1/683F8.s.o(.data)
        build/src/func_80077C04.c.o(.data)
        build/asm/disc1/68418.s.o(.data)
        build/src/func_80077C24.c.o(.data)
        build/asm/disc1/68438.s.o(.data)
        build/src/func_80077C44.c.o(.data)
        build/asm/disc1/68458.s.o(.data)
        build/src/func_80077C64.c.o(.data)
        build/asm/disc1/68478.s.o(.data)
        build/src/func_8007A324.c.o(.data)
        build/src/func_8007A334.c.o(.data)
        build/src/func_8007A344.c.o(.data)
        build/src/func_8007A354.c.o(.data)
        build/asm/disc1/6AB60.s.o(.data)
        build/src/func_8007A3EC.c.o(.data)
        build/asm/disc1/6AC00.s.o(.data)
        build/src/func_8007A4A8.c.o(.data)
        build/src/func_8007A4BC.c.o(.data)
        build/asm/disc1/6ACD0.s.o(.data)
        build/src/func_8007C130.c.o(.data)
        build/asm/disc1/6C93C.s.o(.data)
        build/src/func_8007DEA4.c.o(.data)
        build/src/func_8007DEB0.c.o(.data)
        build/asm/disc1/6E6C0.s.o(.data)
        build/src/func_8007F778.c.o(.data)
        build/asm/disc1/6FF88.s.o(.data)
        build/src/func_8007FBC0.c.o(.data)
        build/src/func_8007FBCC.c.o(.data)
        build/src/func_8007FBD8.c.o(.data)
        build/src/func_8007FBE4.c.o(.data)
        build/asm/disc1/703F0.s.o(.data)
        build/src/func_8007FC08.c.o(.data)
        build/src/func_8007FC18.c.o(.data)
        build/src/func_8007FC28.c.o(.data)
        build/src/func_8007FC34.c.o(.data)
        build/src/func_8007FC44.c.o(.data)
        build/src/func_8007FC54.c.o(.data)
        build/asm/disc1/70464.s.o(.data)
        build/src/func_8007FCAC.c.o(.data)
        build/asm/disc1/704BC.s.o(.data)
        build/src/func_80080930.c.o(.data)
        build/src/func_80080940.c.o(.data)
        build/asm/disc1/71150.s.o(.data)
        build/src/func_80080CC8.c.o(.data)
        build/asm/disc1/714DC.s.o(.data)
        build/src/func_80081254.c.o(.data)
        build/asm/disc1/71A68.s.o(.data)
        build/src/func_800822AC.c.o(.data)
        build/asm/disc1/72ABC.s.o(.data)
        build/src/func_80082CDC.c.o(.data)
        build/asm/disc1/734F0.s.o(.data)
        build/src/func_800835A4.c.o(.data)
        build/src/func_800835B0.c.o(.data)
        build/asm/disc1/73DC0.s.o(.data)
        build/src/func_80083E70.c.o(.data)
        build/asm/disc1/74684.s.o(.data)
        build/src/func_80083EE4.c.o(.data)
        build/asm/disc1/746F8.s.o(.data)
        build/src/func_800847A0.c.o(.data)
        build/asm/disc1/74FB0.s.o(.data)
        build/src/func_80085728.c.o(.data)
        build/asm/disc1/75F44.s.o(.data)
        build/src/func_800870E0.c.o(.data)
        build/asm/disc1/778F0.s.o(.data)
        build/src/func_80087198.c.o(.data)
        build/asm/disc1/779AC.s.o(.data)
        build/src/func_80087414.c.o(.data)
        build/asm/disc1/77C28.s.o(.data)
        build/src/func_8008AB1C.c.o(.data)
        build/asm/disc1/7B39C.s.o(.data)
        build/src/func_8008CA7C.c.o(.data)
        build/asm/disc1/7D284.s.o(.data)
        build/src/func_8008D7C0.c.o(.data)
        build/asm/disc1/7DFD0.s.o(.data)
        build/src/func_8008D820.c.o(.data)
        build/asm/disc1/7E044.s.o(.data)
        build/src/func_8008F694.c.o(.data)
        build/src/func_8008F6A8.c.o(.data)
        build/asm/disc1/7FEB0.s.o(.data)
        build/src/func_8008F868.c.o(.data)
        build/src/func_8008F880.c.o(.data)
        build/asm/disc1/80098.s.o(.data)
        build/src/func_8008FCB4.c.o(.data)
        build/asm/disc1/804BC.s.o(.data)
        build/src/func_800904A0.c.o(.data)
        build/src/func_800904AC.c.o(.data)
        build/src/func_800904B4.c.o(.data)
        build/src/func_800904BC.c.o(.data)
        build/asm/disc1/80CC4.s.o(.data)
        build/src/func_800906B4.c.o(.data)
        build/asm/disc1/80EE4.s.o(.data)
        build/src/func_80090A0C.c.o(.data)
        build/asm/disc1/81220.s.o(.data)
        build/src/func_80090C38.c.o(.data)
        build/src/func_80090C4C.c.o(.data)
        build/src/func_80090C60.c.o(.data)
        build/src/func_80090C74.c.o(.data)
        build/asm/disc1/81488.s.o(.data)
        build/src/func_80090F54.c.o(.data)
        build/asm/disc1/81768.s.o(.data)
        build/asm/disc1/B2AF8.s.o(.data)
        build/src/func_800C2B40.c.o(.data)
        build/src/func_800C2B50.c.o(.data)
        build/asm/disc1/B3368.s.o(.data)
        build/src/func_800C7DC4.c.o(.data)
        build/src/func_800C7DD4.c.o(.data)
        build/src/func_800C7DDC.c.o(.data)
        build/asm/disc1/B85E4.s.o(.data)
        build/src/func_800C8268.c.o(.data)
        build/asm/disc1/B8A70.s.o(.data)
        build/src/func_800C8BB4.c.o(.data)
        build/asm/disc1/B93C0.s.o(.data)
        build/src/func_800C8F08.c.o(.data)
        build/src/func_800C8F18.c.o(.data)
        build/src/func_800C8F20.c.o(.data)
        build/asm/disc1/B9728.s.o(.data)
        build/src/func_800C9260.c.o(.data)
        build/asm/disc1/B9A68.s.o(.data)
        build/src/func_800C9968.c.o(.data)
        build/asm/disc1/BA174.s.o(.data)
        build/src/func_800C9C00.c.o(.data)
        build/asm/disc1/BA410.s.o(.data)
        build/src/func_800C9EA0.c.o(.data)
        build/asm/disc1/BA6A8.s.o(.data)
        build/src/func_800CA4A8.c.o(.data)
        build/asm/disc1/BACB4.s.o(.data)
        build/src/func_800CA798.c.o(.data)
        build/asm/disc1/BAFA8.s.o(.data)
        build/src/func_800CACD4.c.o(.data)
        build/asm/disc1/BB4DC.s.o(.data)
        build/src/func_800CBB24.c.o(.data)
        build/asm/disc1/BC330.s.o(.data)
        build/src/func_800CBFA4.c.o(.data)
        build/asm/disc1/BC7B4.s.o(.data)
        build/src/func_800CCF80.c.o(.data)
        build/asm/disc1/BD790.s.o(.data)
        build/src/func_800CD2DC.c.o(.data)
        build/src/func_800CD2E4.c.o(.data)
        build/asm/disc1/BDAEC.s.o(.data)
        build/src/func_800CD59C.c.o(.data)
        build/src/func_800CD5A4.c.o(.data)
        build/asm/disc1/BDDB0.s.o(.data)
        build/src/func_800CD71C.c.o(.data)
        build/asm/disc1/BDF28.s.o(.data)
        build/src/func_800CD960.c.o(.data)
        build/asm/disc1/BE170.s.o(.data)
        build/src/func_800CDD04.c.o(.data)
        build/asm/disc1/BE50C.s.o(.data)
        build/src/func_800CDF40.c.o(.data)
        build/asm/disc1/BE74C.s.o(.data)
        build/src/func_800CE1DC.c.o(.data)
        build/asm/disc1/BE9EC.s.o(.data)
        build/src/func_800CE3AC.c.o(.data)
        build/asm/disc1/BEBB4.s.o(.data)
        build/src/func_800CE464.c.o(.data)
        build/asm/disc1/BEC70.s.o(.data)
        build/src/func_800D4850.c.o(.data)
        build/asm/disc1/C5060.s.o(.data)
        build/asm/disc1/2A0C.s.o(.rodata)
        build/src/func_80017E9C.c.o(.rodata)
        build/asm/disc1/86A4.s.o(.rodata)
        build/src/func_80017FDC.c.o(.rodata)
        build/src/func_80017FF0.c.o(.rodata)
        build/asm/disc1/8804.s.o(.rodata)
        build/src/func_80019050.c.o(.rodata)
        build/src/func_80019058.c.o(.rodata)
        build/asm/disc1/9860.s.o(.rodata)
        build/src/func_800190AC.c.o(.rodata)
        build/src/func_800190B4.c.o(.rodata)
        build/asm/disc1/98BC.s.o(.rodata)
        build/src/func_800192B8.c.o(.rodata)
        build/src/func_800192C8.c.o(.rodata)
        build/asm/disc1/9ADC.s.o(.rodata)
        build/src/func_80019BE4.c.o(.rodata)
        build/asm/disc1/A404.s.o(.rodata)
        build/src/func_80020EFC.c.o(.rodata)
        build/asm/disc1/11718.s.o(.rodata)
        build/src/func_80033A20.c.o(.rodata)
        build/asm/disc1/2422C.s.o(.rodata)
        build/src/func_800371A4.c.o(.rodata)
        build/asm/disc1/279B0.s.o(.rodata)
        build/src/func_80037454.c.o(.rodata)
        build/asm/disc1/27C6C.s.o(.rodata)
        build/src/func_800375B4.c.o(.rodata)
        build/src/func_800375C4.c.o(.rodata)
        build/src/func_800375D0.c.o(.rodata)
        build/asm/disc1/27DE0.s.o(.rodata)
        build/src/func_80037864.c.o(.rodata)
        build/asm/disc1/28070.s.o(.rodata)
        build/src/func_80038940.c.o(.rodata)
        build/asm/disc1/29154.s.o(.rodata)
        build/src/func_80038D0C.c.o(.rodata)
        build/asm/disc1/2951C.s.o(.rodata)
        build/src/func_8003D82C.c.o(.rodata)
        build/asm/disc1/2E034.s.o(.rodata)
        build/src/func_8003DFC8.c.o(.rodata)
        build/src/func_8003DFD0.c.o(.rodata)
        build/asm/disc1/2E7D8.s.o(.rodata)
        build/src/func_8003FFAC.c.o(.rodata)
        build/src/func_8003FFBC.c.o(.rodata)
        build/asm/disc1/307CC.s.o(.rodata)
        build/src/func_800428C4.c.o(.rodata)
        build/asm/disc1/330D4.s.o(.rodata)
        build/src/func_80042910.c.o(.rodata)
        build/asm/disc1/33128.s.o(.rodata)
        build/src/func_80042B28.c.o(.rodata)
        build/src/func_80042B38.c.o(.rodata)
        build/src/func_80042B50.c.o(.rodata)
        build/src/func_80042B6C.c.o(.rodata)
        build/src/func_80042BC8.c.o(.rodata)
        build/src/func_80042BD8.c.o(.rodata)
        build/src/func_80042BEC.c.o(.rodata)
        build/src/func_80042C00.c.o(.rodata)
        build/src/func_80042C14.c.o(.rodata)
        build/src/func_80042C28.c.o(.rodata)
        build/src/func_80042C3C.c.o(.rodata)
        build/src/func_80042C50.c.o(.rodata)
        build/src/func_80042C64.c.o(.rodata)
        build/asm/disc1/33478.s.o(.rodata)
        build/src/func_80042CB8.c.o(.rodata)
        build/asm/disc1/334C4.s.o(.rodata)
        build/src/func_80042ED0.c.o(.rodata)
        build/asm/disc1/336DC.s.o(.rodata)
        build/src/func_80042F20.c.o(.rodata)
        build/src/func_80042F38.c.o(.rodata)
        build/asm/disc1/33744.s.o(.rodata)
        build/src/func_80043038.c.o(.rodata)
        build/asm/disc1/3384C.s.o(.rodata)
        build/src/func_80043240.c.o(.rodata)
        build/asm/disc1/33A4C.s.o(.rodata)
        build/src/func_800438C0.c.o(.rodata)
        build/src/func_800438E0.c.o(.rodata)
        build/asm/disc1/340EC.s.o(.rodata)
        build/src/func_8004D024.c.o(.rodata)
        build/asm/disc1/3D830.s.o(.rodata)
        build/src/func_8004D27C.c.o(.rodata)
        build/src/func_8004D288.c.o(.rodata)
        build/asm/disc1/3DA98.s.o(.rodata)
        build/src/func_8004DA9C.c.o(.rodata)
        build/asm/disc1/3E2A4.s.o(.rodata)
        build/src/func_8004E970.c.o(.rodata)
        build/asm/disc1/3F17C.s.o(.rodata)
        build/src/func_8004F448.c.o(.rodata)
        build/asm/disc1/3FC64.s.o(.rodata)
        build/src/func_8004F808.c.o(.rodata)
        build/asm/disc1/40038.s.o(.rodata)
        build/src/func_80050D18.c.o(.rodata)
        build/asm/disc1/41520.s.o(.rodata)
        build/src/func_80051084.c.o(.rodata)
        build/asm/disc1/41898.s.o(.rodata)
        build/src/func_80051244.c.o(.rodata)
        build/asm/disc1/41A58.s.o(.rodata)
        build/src/func_800514F8.c.o(.rodata)
        build/src/func_80051504.c.o(.rodata)
        build/asm/disc1/41D10.s.o(.rodata)
        build/src/func_80051834.c.o(.rodata)
        build/asm/disc1/4204C.s.o(.rodata)
        build/src/func_80051E48.c.o(.rodata)
        build/src/func_80051E58.c.o(.rodata)
        build/asm/disc1/42664.s.o(.rodata)
        build/src/func_80052514.c.o(.rodata)
        build/src/func_80052524.c.o(.rodata)
        build/asm/disc1/42D34.s.o(.rodata)
        build/src/func_8005257C.c.o(.rodata)
        build/asm/disc1/42D94.s.o(.rodata)
        build/src/func_800527B4.c.o(.rodata)
        build/src/func_800527C0.c.o(.rodata)
        build/asm/disc1/42FC8.s.o(.rodata)
        build/src/func_80052EB0.c.o(.rodata)
        build/asm/disc1/436C0.s.o(.rodata)
        build/src/func_80052F0C.c.o(.rodata)
        build/asm/disc1/43724.s.o(.rodata)
        build/src/func_80054288.c.o(.rodata)
        build/src/func_80054294.c.o(.rodata)
        build/asm/disc1/44AA0.s.o(.rodata)
        build/src/func_80057ECC.c.o(.rodata)
        build/asm/disc1/486D8.s.o(.rodata)
        build/src/func_8005B890.c.o(.rodata)
        build/src/func_8005B89C.c.o(.rodata)
        build/asm/disc1/4C0A8.s.o(.rodata)
        build/src/func_8005BC98.c.o(.rodata)
        build/src/func_8005BCA8.c.o(.rodata)
        build/src/func_8005BCB0.c.o(.rodata)
        build/asm/disc1/4C4BC.s.o(.rodata)
        build/src/func_8005BEDC.c.o(.rodata)
        build/asm/disc1/4C6E8.s.o(.rodata)
        build/src/func_8005C488.c.o(.rodata)
        build/asm/disc1/4CC98.s.o(.rodata)
        build/src/func_8005E114.c.o(.rodata)
        build/src/func_8005E120.c.o(.rodata)
        build/asm/disc1/4E92C.s.o(.rodata)
        build/src/func_8005E57C.c.o(.rodata)
        build/asm/disc1/4ED88.s.o(.rodata)
        build/src/func_8005E6E4.c.o(.rodata)
        build/asm/disc1/4EEF0.s.o(.rodata)
        build/src/func_8005E884.c.o(.rodata)
        build/src/func_8005E894.c.o(.rodata)
        build/asm/disc1/4F0A4.s.o(.rodata)
        build/src/func_8005EB58.c.o(.rodata)
        build/asm/disc1/4F364.s.o(.rodata)
        build/src/func_8005EEC8.c.o(.rodata)
        build/asm/disc1/4F6D4.s.o(.rodata)
        build/src/func_800614A0.c.o(.rodata)
        build/asm/disc1/51CAC.s.o(.rodata)
        build/src/func_800622B0.c.o(.rodata)
        build/asm/disc1/52ABC.s.o(.rodata)
        build/src/func_800629B0.c.o(.rodata)
        build/asm/disc1/531BC.s.o(.rodata)
        build/src/func_80062CB8.c.o(.rodata)
        build/src/func_80062CC4.c.o(.rodata)
        build/src/func_80062CD0.c.o(.rodata)
        build/asm/disc1/534E4.s.o(.rodata)
        build/src/func_80063198.c.o(.rodata)
        build/src/func_800631AC.c.o(.rodata)
        build/asm/disc1/539C0.s.o(.rodata)
        build/src/func_80064A48.c.o(.rodata)
        build/asm/disc1/55254.s.o(.rodata)
        build/src/func_80064C20.c.o(.rodata)
        build/asm/disc1/55430.s.o(.rodata)
        build/src/func_8006EBD4.c.o(.rodata)
        build/asm/disc1/5F3E4.s.o(.rodata)
        build/src/func_80073DE8.c.o(.rodata)
        build/src/func_80073DF8.c.o(.rodata)
        build/asm/disc1/64610.s.o(.rodata)
        build/src/func_80074330.c.o(.rodata)
        build/asm/disc1/64B54.s.o(.rodata)
        build/src/func_800744A4.c.o(.rodata)
        build/asm/disc1/64CC8.s.o(.rodata)
        build/src/func_8007474C.c.o(.rodata)
        build/asm/disc1/64F70.s.o(.rodata)
        build/src/func_80074A14.c.o(.rodata)
        build/src/func_80074A28.c.o(.rodata)
        build/asm/disc1/65238.s.o(.rodata)
        build/src/func_80074CB8.c.o(.rodata)
        build/asm/disc1/654C8.s.o(.rodata)
        build/src/func_8007633C.c.o(.rodata)
        build/asm/disc1/66B54.s.o(.rodata)
        build/src/func_80077A28.c.o(.rodata)
        build/asm/disc1/6824C.s.o(.rodata)
        build/src/func_80077B64.c.o(.rodata)
        build/asm/disc1/68378.s.o(.rodata)
        build/src/func_80077B84.c.o(.rodata)
        build/asm/disc1/68398.s.o(.rodata)
        build/src/func_80077BA4.c.o(.rodata)
        build/asm/disc1/683B8.s.o(.rodata)
        build/src/func_80077BC4.c.o(.rodata)
        build/asm/disc1/683D8.s.o(.rodata)
        build/src/func_80077BE4.c.o(.rodata)
        build/asm/disc1/683F8.s.o(.rodata)
        build/src/func_80077C04.c.o(.rodata)
        build/asm/disc1/68418.s.o(.rodata)
        build/src/func_80077C24.c.o(.rodata)
        build/asm/disc1/68438.s.o(.rodata)
        build/src/func_80077C44.c.o(.rodata)
        build/asm/disc1/68458.s.o(.rodata)
        build/src/func_80077C64.c.o(.rodata)
        build/asm/disc1/68478.s.o(.rodata)
        build/src/func_8007A324.c.o(.rodata)
        build/src/func_8007A334.c.o(.rodata)
        build/src/func_8007A344.c.o(.rodata)
        build/src/func_8007A354.c.o(.rodata)
        build/asm/disc1/6AB60.s.o(.rodata)
        build/src/func_8007A3EC.c.o(.rodata)
        build/asm/disc1/6AC00.s.o(.rodata)
        build/src/func_8007A4A8.c.o(.rodata)
        build/src/func_8007A4BC.c.o(.rodata)
        build/asm/disc1/6ACD0.s.o(.rodata)
        build/src/func_8007C130.c.o(.rodata)
        build/asm/disc1/6C93C.s.o(.rodata)
        build/src/func_8007DEA4.c.o(.rodata)
        build/src/func_8007DEB0.c.o(.rodata)
        build/asm/disc1/6E6C0.s.o(.rodata)
        build/src/func_8007F778.c.o(.rodata)
        build/asm/disc1/6FF88.s.o(.rodata)
        build/src/func_8007FBC0.c.o(.rodata)
        build/src/func_8007FBCC.c.o(.rodata)
        build/src/func_8007FBD8.c.o(.rodata)
        build/src/func_8007FBE4.c.o(.rodata)
        build/asm/disc1/703F0.s.o(.rodata)
        build/src/func_8007FC08.c.o(.rodata)
        build/src/func_8007FC18.c.o(.rodata)
        build/src/func_8007FC28.c.o(.rodata)
        build/src/func_8007FC34.c.o(.rodata)
        build/src/func_8007FC44.c.o(.rodata)
        build/src/func_8007FC54.c.o(.rodata)
        build/asm/disc1/70464.s.o(.rodata)
        build/src/func_8007FCAC.c.o(.rodata)
        build/asm/disc1/704BC.s.o(.rodata)
        build/src/func_80080930.c.o(.rodata)
        build/src/func_80080940.c.o(.rodata)
        build/asm/disc1/71150.s.o(.rodata)
        build/src/func_80080CC8.c.o(.rodata)
        build/asm/disc1/714DC.s.o(.rodata)
        build/src/func_80081254.c.o(.rodata)
        build/asm/disc1/71A68.s.o(.rodata)
        build/src/func_800822AC.c.o(.rodata)
        build/asm/disc1/72ABC.s.o(.rodata)
        build/src/func_80082CDC.c.o(.rodata)
        build/asm/disc1/734F0.s.o(.rodata)
        build/src/func_800835A4.c.o(.rodata)
        build/src/func_800835B0.c.o(.rodata)
        build/asm/disc1/73DC0.s.o(.rodata)
        build/src/func_80083E70.c.o(.rodata)
        build/asm/disc1/74684.s.o(.rodata)
        build/src/func_80083EE4.c.o(.rodata)
        build/asm/disc1/746F8.s.o(.rodata)
        build/src/func_800847A0.c.o(.rodata)
        build/asm/disc1/74FB0.s.o(.rodata)
        build/src/func_80085728.c.o(.rodata)
        build/asm/disc1/75F44.s.o(.rodata)
        build/src/func_800870E0.c.o(.rodata)
        build/asm/disc1/778F0.s.o(.rodata)
        build/src/func_80087198.c.o(.rodata)
        build/asm/disc1/779AC.s.o(.rodata)
        build/src/func_80087414.c.o(.rodata)
        build/asm/disc1/77C28.s.o(.rodata)
        build/src/func_8008AB1C.c.o(.rodata)
        build/asm/disc1/7B39C.s.o(.rodata)
        build/src/func_8008CA7C.c.o(.rodata)
        build/asm/disc1/7D284.s.o(.rodata)
        build/src/func_8008D7C0.c.o(.rodata)
        build/asm/disc1/7DFD0.s.o(.rodata)
        build/src/func_8008D820.c.o(.rodata)
        build/asm/disc1/7E044.s.o(.rodata)
        build/src/func_8008F694.c.o(.rodata)
        build/src/func_8008F6A8.c.o(.rodata)
        build/asm/disc1/7FEB0.s.o(.rodata)
        build/src/func_8008F868.c.o(.rodata)
        build/src/func_8008F880.c.o(.rodata)
        build/asm/disc1/80098.s.o(.rodata)
        build/src/func_8008FCB4.c.o(.rodata)
        build/asm/disc1/804BC.s.o(.rodata)
        build/src/func_800904A0.c.o(.rodata)
        build/src/func_800904AC.c.o(.rodata)
        build/src/func_800904B4.c.o(.rodata)
        build/src/func_800904BC.c.o(.rodata)
        build/asm/disc1/80CC4.s.o(.rodata)
        build/src/func_800906B4.c.o(.rodata)
        build/asm/disc1/80EE4.s.o(.rodata)
        build/src/func_80090A0C.c.o(.rodata)
        build/asm/disc1/81220.s.o(.rodata)
        build/src/func_80090C38.c.o(.rodata)
        build/src/func_80090C4C.c.o(.rodata)
        build/src/func_80090C60.c.o(.rodata)
        build/src/func_80090C74.c.o(.rodata)
        build/asm/disc1/81488.s.o(.rodata)
        build/src/func_80090F54.c.o(.rodata)
        build/asm/disc1/81768.s.o(.rodata)
        build/asm/disc1/B2AF8.s.o(.rodata)
        build/src/func_800C2B40.c.o(.rodata)
        build/src/func_800C2B50.c.o(.rodata)
        build/asm/disc1/B3368.s.o(.rodata)
        build/src/func_800C7DC4.c.o(.rodata)
        build/src/func_800C7DD4.c.o(.rodata)
        build/src/func_800C7DDC.c.o(.rodata)
        build/asm/disc1/B85E4.s.o(.rodata)
        build/src/func_800C8268.c.o(.rodata)
        build/asm/disc1/B8A70.s.o(.rodata)
        build/src/func_800C8BB4.c.o(.rodata)
        build/asm/disc1/B93C0.s.o(.rodata)
        build/src/func_800C8F08.c.o(.rodata)
        build/src/func_800C8F18.c.o(.rodata)
        build/src/func_800C8F20.c.o(.rodata)
        build/asm/disc1/B9728.s.o(.rodata)
        build/src/func_800C9260.c.o(.rodata)
        build/asm/disc1/B9A68.s.o(.rodata)
        build/src/func_800C9968.c.o(.rodata)
        build/asm/disc1/BA174.s.o(.rodata)
        build/src/func_800C9C00.c.o(.rodata)
        build/asm/disc1/BA410.s.o(.rodata)
        build/src/func_800C9EA0.c.o(.rodata)
        build/asm/disc1/BA6A8.s.o(.rodata)
        build/src/func_800CA4A8.c.o(.rodata)
        build/asm/disc1/BACB4.s.o(.rodata)
        build/src/func_800CA798.c.o(.rodata)
        build/asm/disc1/BAFA8.s.o(.rodata)
        build/src/func_800CACD4.c.o(.rodata)
        build/asm/disc1/BB4DC.s.o(.rodata)
        build/src/func_800CBB24.c.o(.rodata)
        build/asm/disc1/BC330.s.o(.rodata)
        build/src/func_800CBFA4.c.o(.rodata)
        build/asm/disc1/BC7B4.s.o(.rodata)
        build/src/func_800CCF80.c.o(.rodata)
        build/asm/disc1/BD790.s.o(.rodata)
        build/src/func_800CD2DC.c.o(.rodata)
        build/src/func_800CD2E4.c.o(.rodata)
        build/asm/disc1/BDAEC.s.o(.rodata)
        build/src/func_800CD59C.c.o(.rodata)
        build/src/func_800CD5A4.c.o(.rodata)
        build/asm/disc1/BDDB0.s.o(.rodata)
        build/src/func_800CD71C.c.o(.rodata)
        build/asm/disc1/BDF28.s.o(.rodata)
        build/src/func_800CD960.c.o(.rodata)
        build/asm/disc1/BE170.s.o(.rodata)
        build/src/func_800CDD04.c.o(.rodata)
        build/asm/disc1/BE50C.s.o(.rodata)
        build/src/func_800CDF40.c.o(.rodata)
        build/asm/disc1/BE74C.s.o(.rodata)
        build/src/func_800CE1DC.c.o(.rodata)
        build/asm/disc1/BE9EC.s.o(.rodata)
        build/src/func_800CE3AC.c.o(.rodata)
        build/asm/disc1/BEBB4.s.o(.rodata)
        build/src/func_800CE464.c.o(.rodata)
        build/asm/disc1/BEC70.s.o(.rodata)
        build/src/func_800D4850.c.o(.rodata)
        build/asm/disc1/C5060.s.o(.rodata)
        build/asm/disc1/2A0C.s.o(.bss)
        build/src/func_80017E9C.c.o(.bss)
        build/asm/disc1/86A4.s.o(.bss)
        build/src/func_80017FDC.c.o(.bss)
        build/src/func_80017FF0.c.o(.bss)
        build/asm/disc1/8804.s.o(.bss)
        build/src/func_80019050.c.o(.bss)
        build/src/func_80019058.c.o(.bss)
        build/asm/disc1/9860.s.o(.bss)
        build/src/func_800190AC.c.o(.bss)
        build/src/func_800190B4.c.o(.bss)
        build/asm/disc1/98BC.s.o(.bss)
        build/src/func_800192B8.c.o(.bss)
        build/src/func_800192C8.c.o(.bss)
        build/asm/disc1/9ADC.s.o(.bss)
        build/src/func_80019BE4.c.o(.bss)
        build/asm/disc1/A404.s.o(.bss)
        build/src/func_80020EFC.c.o(.bss)
        build/asm/disc1/11718.s.o(.bss)
        build/src/func_80033A20.c.o(.bss)
        build/asm/disc1/2422C.s.o(.bss)
        build/src/func_800371A4.c.o(.bss)
        build/asm/disc1/279B0.s.o(.bss)
        build/src/func_80037454.c.o(.bss)
        build/asm/disc1/27C6C.s.o(.bss)
        build/src/func_800375B4.c.o(.bss)
        build/src/func_800375C4.c.o(.bss)
        build/src/func_800375D0.c.o(.bss)
        build/asm/disc1/27DE0.s.o(.bss)
        build/src/func_80037864.c.o(.bss)
        build/asm/disc1/28070.s.o(.bss)
        build/src/func_80038940.c.o(.bss)
        build/asm/disc1/29154.s.o(.bss)
        build/src/func_80038D0C.c.o(.bss)
        build/asm/disc1/2951C.s.o(.bss)
        build/src/func_8003D82C.c.o(.bss)
        build/asm/disc1/2E034.s.o(.bss)
        build/src/func_8003DFC8.c.o(.bss)
        build/src/func_8003DFD0.c.o(.bss)
        build/asm/disc1/2E7D8.s.o(.bss)
        build/src/func_8003FFAC.c.o(.bss)
        build/src/func_8003FFBC.c.o(.bss)
        build/asm/disc1/307CC.s.o(.bss)
        build/src/func_800428C4.c.o(.bss)
        build/asm/disc1/330D4.s.o(.bss)
        build/src/func_80042910.c.o(.bss)
        build/asm/disc1/33128.s.o(.bss)
        build/src/func_80042B28.c.o(.bss)
        build/src/func_80042B38.c.o(.bss)
        build/src/func_80042B50.c.o(.bss)
        build/src/func_80042B6C.c.o(.bss)
        build/src/func_80042BC8.c.o(.bss)
        build/src/func_80042BD8.c.o(.bss)
        build/src/func_80042BEC.c.o(.bss)
        build/src/func_80042C00.c.o(.bss)
        build/src/func_80042C14.c.o(.bss)
        build/src/func_80042C28.c.o(.bss)
        build/src/func_80042C3C.c.o(.bss)
        build/src/func_80042C50.c.o(.bss)
        build/src/func_80042C64.c.o(.bss)
        build/asm/disc1/33478.s.o(.bss)
        build/src/func_80042CB8.c.o(.bss)
        build/asm/disc1/334C4.s.o(.bss)
        build/src/func_80042ED0.c.o(.bss)
        build/asm/disc1/336DC.s.o(.bss)
        build/src/func_80042F20.c.o(.bss)
        build/src/func_80042F38.c.o(.bss)
        build/asm/disc1/33744.s.o(.bss)
        build/src/func_80043038.c.o(.bss)
        build/asm/disc1/3384C.s.o(.bss)
        build/src/func_80043240.c.o(.bss)
        build/asm/disc1/33A4C.s.o(.bss)
        build/src/func_800438C0.c.o(.bss)
        build/src/func_800438E0.c.o(.bss)
        build/asm/disc1/340EC.s.o(.bss)
        build/src/func_8004D024.c.o(.bss)
        build/asm/disc1/3D830.s.o(.bss)
        build/src/func_8004D27C.c.o(.bss)
        build/src/func_8004D288.c.o(.bss)
        build/asm/disc1/3DA98.s.o(.bss)
        build/src/func_8004DA9C.c.o(.bss)
        build/asm/disc1/3E2A4.s.o(.bss)
        build/src/func_8004E970.c.o(.bss)
        build/asm/disc1/3F17C.s.o(.bss)
        build/src/func_8004F448.c.o(.bss)
        build/asm/disc1/3FC64.s.o(.bss)
        build/src/func_8004F808.c.o(.bss)
        build/asm/disc1/40038.s.o(.bss)
        build/src/func_80050D18.c.o(.bss)
        build/asm/disc1/41520.s.o(.bss)
        build/src/func_80051084.c.o(.bss)
        build/asm/disc1/41898.s.o(.bss)
        build/src/func_80051244.c.o(.bss)
        build/asm/disc1/41A58.s.o(.bss)
        build/src/func_800514F8.c.o(.bss)
        build/src/func_80051504.c.o(.bss)
        build/asm/disc1/41D10.s.o(.bss)
        build/src/func_80051834.c.o(.bss)
        build/asm/disc1/4204C.s.o(.bss)
        build/src/func_80051E48.c.o(.bss)
        build/src/func_80051E58.c.o(.bss)
        build/asm/disc1/42664.s.o(.bss)
        build/src/func_80052514.c.o(.bss)
        build/src/func_80052524.c.o(.bss)
        build/asm/disc1/42D34.s.o(.bss)
        build/src/func_8005257C.c.o(.bss)
        build/asm/disc1/42D94.s.o(.bss)
        build/src/func_800527B4.c.o(.bss)
        build/src/func_800527C0.c.o(.bss)
        build/asm/disc1/42FC8.s.o(.bss)
        build/src/func_80052EB0.c.o(.bss)
        build/asm/disc1/436C0.s.o(.bss)
        build/src/func_80052F0C.c.o(.bss)
        build/asm/disc1/43724.s.o(.bss)
        build/src/func_80054288.c.o(.bss)
        build/src/func_80054294.c.o(.bss)
        build/asm/disc1/44AA0.s.o(.bss)
        build/src/func_80057ECC.c.o(.bss)
        build/asm/disc1/486D8.s.o(.bss)
        build/src/func_8005B890.c.o(.bss)
        build/src/func_8005B89C.c.o(.bss)
        build/asm/disc1/4C0A8.s.o(.bss)
        build/src/func_8005BC98.c.o(.bss)
        build/src/func_8005BCA8.c.o(.bss)
        build/src/func_8005BCB0.c.o(.bss)
        build/asm/disc1/4C4BC.s.o(.bss)
        build/src/func_8005BEDC.c.o(.bss)
        build/asm/disc1/4C6E8.s.o(.bss)
        build/src/func_8005C488.c.o(.bss)
        build/asm/disc1/4CC98.s.o(.bss)
        build/src/func_8005E114.c.o(.bss)
        build/src/func_8005E120.c.o(.bss)
        build/asm/disc1/4E92C.s.o(.bss)
        build/src/func_8005E57C.c.o(.bss)
        build/asm/disc1/4ED88.s.o(.bss)
        build/src/func_8005E6E4.c.o(.bss)
        build/asm/disc1/4EEF0.s.o(.bss)
        build/src/func_8005E884.c.o(.bss)
        build/src/func_8005E894.c.o(.bss)
        build/asm/disc1/4F0A4.s.o(.bss)
        build/src/func_8005EB58.c.o(.bss)
        build/asm/disc1/4F364.s.o(.bss)
        build/src/func_8005EEC8.c.o(.bss)
        build/asm/disc1/4F6D4.s.o(.bss)
        build/src/func_800614A0.c.o(.bss)
        build/asm/disc1/51CAC.s.o(.bss)
        build/src/func_800622B0.c.o(.bss)
        build/asm/disc1/52ABC.s.o(.bss)
        build/src/func_800629B0.c.o(.bss)
        build/asm/disc1/531BC.s.o(.bss)
        build/src/func_80062CB8.c.o(.bss)
        build/src/func_80062CC4.c.o(.bss)
        build/src/func_80062CD0.c.o(.bss)
        build/asm/disc1/534E4.s.o(.bss)
        build/src/func_80063198.c.o(.bss)
        build/src/func_800631AC.c.o(.bss)
        build/asm/disc1/539C0.s.o(.bss)
        build/src/func_80064A48.c.o(.bss)
        build/asm/disc1/55254.s.o(.bss)
        build/src/func_80064C20.c.o(.bss)
        build/asm/disc1/55430.s.o(.bss)
        build/src/func_8006EBD4.c.o(.bss)
        build/asm/disc1/5F3E4.s.o(.bss)
        build/src/func_80073DE8.c.o(.bss)
        build/src/func_80073DF8.c.o(.bss)
        build/asm/disc1/64610.s.o(.bss)
        build/src/func_80074330.c.o(.bss)
        build/asm/disc1/64B54.s.o(.bss)
        build/src/func_800744A4.c.o(.bss)
        build/asm/disc1/64CC8.s.o(.bss)
        build/src/func_8007474C.c.o(.bss)
        build/asm/disc1/64F70.s.o(.bss)
        build/src/func_80074A14.c.o(.bss)
        build/src/func_80074A28.c.o(.bss)
        build/asm/disc1/65238.s.o(.bss)
        build/src/func_80074CB8.c.o(.bss)
        build/asm/disc1/654C8.s.o(.bss)
        build/src/func_8007633C.c.o(.bss)
        build/asm/disc1/66B54.s.o(.bss)
        build/src/func_80077A28.c.o(.bss)
        build/asm/disc1/6824C.s.o(.bss)
        build/src/func_80077B64.c.o(.bss)
        build/asm/disc1/68378.s.o(.bss)
        build/src/func_80077B84.c.o(.bss)
        build/asm/disc1/68398.s.o(.bss)
        build/src/func_80077BA4.c.o(.bss)
        build/asm/disc1/683B8.s.o(.bss)
        build/src/func_80077BC4.c.o(.bss)
        build/asm/disc1/683D8.s.o(.bss)
        build/src/func_80077BE4.c.o(.bss)
        build/asm/disc1/683F8.s.o(.bss)
        build/src/func_80077C04.c.o(.bss)
        build/asm/disc1/68418.s.o(.bss)
        build/src/func_80077C24.c.o(.bss)
        build/asm/disc1/68438.s.o(.bss)
        build/src/func_80077C44.c.o(.bss)
        build/asm/disc1/68458.s.o(.bss)
        build/src/func_80077C64.c.o(.bss)
        build/asm/disc1/68478.s.o(.bss)
        build/src/func_8007A324.c.o(.bss)
        build/src/func_8007A334.c.o(.bss)
        build/src/func_8007A344.c.o(.bss)
        build/src/func_8007A354.c.o(.bss)
        build/asm/disc1/6AB60.s.o(.bss)
        build/src/func_8007A3EC.c.o(.bss)
        build/asm/disc1/6AC00.s.o(.bss)
        build/src/func_8007A4A8.c.o(.bss)
        build/src/func_8007A4BC.c.o(.bss)
        build/asm/disc1/6ACD0.s.o(.bss)
        build/src/func_8007C130.c.o(.bss)
        build/asm/disc1/6C93C.s.o(.bss)
        build/src/func_8007DEA4.c.o(.bss)
        build/src/func_8007DEB0.c.o(.bss)
        build/asm/disc1/6E6C0.s.o(.bss)
        build/src/func_8007F778.c.o(.bss)
        build/asm/disc1/6FF88.s.o(.bss)
        build/src/func_8007FBC0.c.o(.bss)
        build/src/func_8007FBCC.c.o(.bss)
        build/src/func_8007FBD8.c.o(.bss)
        build/src/func_8007FBE4.c.o(.bss)
        build/asm/disc1/703F0.s.o(.bss)
        build/src/func_8007FC08.c.o(.bss)
        build/src/func_8007FC18.c.o(.bss)
        build/src/func_8007FC28.c.o(.bss)
        build/src/func_8007FC34.c.o(.bss)
        build/src/func_8007FC44.c.o(.bss)
        build/src/func_8007FC54.c.o(.bss)
        build/asm/disc1/70464.s.o(.bss)
        build/src/func_8007FCAC.c.o(.bss)
        build/asm/disc1/704BC.s.o(.bss)
        build/src/func_80080930.c.o(.bss)
        build/src/func_80080940.c.o(.bss)
        build/asm/disc1/71150.s.o(.bss)
        build/src/func_80080CC8.c.o(.bss)
        build/asm/disc1/714DC.s.o(.bss)
        build/src/func_80081254.c.o(.bss)
        build/asm/disc1/71A68.s.o(.bss)
        build/src/func_800822AC.c.o(.bss)
        build/asm/disc1/72ABC.s.o(.bss)
        build/src/func_80082CDC.c.o(.bss)
        build/asm/disc1/734F0.s.o(.bss)
        build/src/func_800835A4.c.o(.bss)
        build/src/func_800835B0.c.o(.bss)
        build/asm/disc1/73DC0.s.o(.bss)
        build/src/func_80083E70.c.o(.bss)
        build/asm/disc1/74684.s.o(.bss)
        build/src/func_80083EE4.c.o(.bss)
        build/asm/disc1/746F8.s.o(.bss)
        build/src/func_800847A0.c.o(.bss)
        build/asm/disc1/74FB0.s.o(.bss)
        build/src/func_80085728.c.o(.bss)
        build/asm/disc1/75F44.s.o(.bss)
        build/src/func_800870E0.c.o(.bss)
        build/asm/disc1/778F0.s.o(.bss)
        build/src/func_80087198.c.o(.bss)
        build/asm/disc1/779AC.s.o(.bss)
        build/src/func_80087414.c.o(.bss)
        build/asm/disc1/77C28.s.o(.bss)
        build/src/func_8008AB1C.c.o(.bss)
        build/asm/disc1/7B39C.s.o(.bss)
        build/src/func_8008CA7C.c.o(.bss)
        build/asm/disc1/7D284.s.o(.bss)
        build/src/func_8008D7C0.c.o(.bss)
        build/asm/disc1/7DFD0.s.o(.bss)
        build/src/func_8008D820.c.o(.bss)
        build/asm/disc1/7E044.s.o(.bss)
        build/src/func_8008F694.c.o(.bss)
        build/src/func_8008F6A8.c.o(.bss)
        build/asm/disc1/7FEB0.s.o(.bss)
        build/src/func_8008F868.c.o(.bss)
        build/src/func_8008F880.c.o(.bss)
        build/asm/disc1/80098.s.o(.bss)
        build/src/func_8008FCB4.c.o(.bss)
        build/asm/disc1/804BC.s.o(.bss)
        build/src/func_800904A0.c.o(.bss)
        build/src/func_800904AC.c.o(.bss)
        build/src/func_800904B4.c.o(.bss)
        build/src/func_800904BC.c.o(.bss)
        build/asm/disc1/80CC4.s.o(.bss)
        build/src/func_800906B4.c.o(.bss)
        build/asm/disc1/80EE4.s.o(.bss)
        build/src/func_80090A0C.c.o(.bss)
        build/asm/disc1/81220.s.o(.bss)
        build/src/func_80090C38.c.o(.bss)
        build/src/func_80090C4C.c.o(.bss)
        build/src/func_80090C60.c.o(.bss)
        build/src/func_80090C74.c.o(.bss)
        build/asm/disc1/81488.s.o(.bss)
        build/src/func_80090F54.c.o(.bss)
        build/asm/disc1/81768.s.o(.bss)
        build/asm/disc1/B2AF8.s.o(.bss)
        build/src/func_800C2B40.c.o(.bss)
        build/src/func_800C2B50.c.o(.bss)
        build/asm/disc1/B3368.s.o(.bss)
        build/src/func_800C7DC4.c.o(.bss)
        build/src/func_800C7DD4.c.o(.bss)
        build/src/func_800C7DDC.c.o(.bss)
        build/asm/disc1/B85E4.s.o(.bss)
        build/src/func_800C8268.c.o(.bss)
        build/asm/disc1/B8A70.s.o(.bss)
        build/src/func_800C8BB4.c.o(.bss)
        build/asm/disc1/B93C0.s.o(.bss)
        build/src/func_800C8F08.c.o(.bss)
        build/src/func_800C8F18.c.o(.bss)
        build/src/func_800C8F20.c.o(.bss)
        build/asm/disc1/B9728.s.o(.bss)
        build/src/func_800C9260.c.o(.bss)
        build/asm/disc1/B9A68.s.o(.bss)
        build/src/func_800C9968.c.o(.bss)
        build/asm/disc1/BA174.s.o(.bss)
        build/src/func_800C9C00.c.o(.bss)
        build/asm/disc1/BA410.s.o(.bss)
        build/src/func_800C9EA0.c.o(.bss)
        build/asm/disc1/BA6A8.s.o(.bss)
        build/src/func_800CA4A8.c.o(.bss)
        build/asm/disc1/BACB4.s.o(.bss)
        build/src/func_800CA798.c.o(.bss)
        build/asm/disc1/BAFA8.s.o(.bss)
        build/src/func_800CACD4.c.o(.bss)
        build/asm/disc1/BB4DC.s.o(.bss)
        build/src/func_800CBB24.c.o(.bss)
        build/asm/disc1/BC330.s.o(.bss)
        build/src/func_800CBFA4.c.o(.bss)
        build/asm/disc1/BC7B4.s.o(.bss)
        build/src/func_800CCF80.c.o(.bss)
        build/asm/disc1/BD790.s.o(.bss)
        build/src/func_800CD2DC.c.o(.bss)
        build/src/func_800CD2E4.c.o(.bss)
        build/asm/disc1/BDAEC.s.o(.bss)
        build/src/func_800CD59C.c.o(.bss)
        build/src/func_800CD5A4.c.o(.bss)
        build/asm/disc1/BDDB0.s.o(.bss)
        build/src/func_800CD71C.c.o(.bss)
        build/asm/disc1/BDF28.s.o(.bss)
        build/src/func_800CD960.c.o(.bss)
        build/asm/disc1/BE170.s.o(.bss)
        build/src/func_800CDD04.c.o(.bss)
        build/asm/disc1/BE50C.s.o(.bss)
        build/src/func_800CDF40.c.o(.bss)
        build/asm/disc1/BE74C.s.o(.bss)
        build/src/func_800CE1DC.c.o(.bss)
        build/asm/disc1/BE9EC.s.o(.bss)
        build/src/func_800CE3AC.c.o(.bss)
        build/asm/disc1/BEBB4.s.o(.bss)
        build/src/func_800CE464.c.o(.bss)
        build/asm/disc1/BEC70.s.o(.bss)
        build/src/func_800D4850.c.o(.bss)
        build/asm/disc1/C5060.s.o(.bss)

    }

    /DISCARD/ : { *(*); }
}
LDEOF

# Probe link to collect remaining undefs (expected first time).
set +e
run "$LD" -EL -m elf32ltsmip -nostdlib --no-check-sections \
    -T "$ROM_ORDER_LD" -T "$ABS_LD" \
    -o build/disc1_probe.elf 2>build/link_probe.err
probe_ec=$?
set -e
if [[ "$probe_ec" -ne 0 ]]; then
    grep -oE 'undefined reference to `D_[0-9A-Fa-f]+' build/link_probe.err \
        | sed 's/undefined reference to `//' | sort -u \
        | while read -r s; do
            addr="${s#D_}"
            echo "${s} = 0x${addr};"
          done >>"$ABS_LD"
    info "appended $(grep -c ' = 0x' "$ABS_LD" || true) absolute symbol lines (incl. auto)"
fi

step "Link (ROM-order script with C leaf)"
# LDFLAGS documented here — not guessed silently.
# -EL -m elf32ltsmip : little-endian MIPS ELF32
# -nostdlib          : no crt/libs
# --no-check-sections: allow splat-style AT() layouts
set +e
run "$LD" -EL -m elf32ltsmip -nostdlib --no-check-sections \
    -T "$ROM_ORDER_LD" \
    -T "$ABS_LD" \
    -Map build/disc1.map \
    -o build/disc1.elf 2>build/link.err
link_ec=$?
set -e
if [[ "$link_ec" -ne 0 ]]; then
    info "link FAILED (exit $link_ec)"
    head -40 build/link.err >&2 || true
    echo
    echo "Rebuild: LINK FAILED"
    echo "Matching: NOT achieved"
    exit 1
fi
info "OK build/disc1.elf"
run "$READELF" -l build/disc1.elf | sed 's/^/  /' || true

step "Pack PS-X EXE candidate"
run "$OBJCOPY" -O binary -j .header build/disc1.elf build/disc1.header.bin
run "$OBJCOPY" -O binary -j .main   build/disc1.elf build/disc1.main.bin
# Pack: 0x800 header + first 0x1EE000 of main (truncate gas/ld align padding)
# Host python3 (pe-mipsel may lack python).
set +e
python3 - <<'PY'
from pathlib import Path
import hashlib, sys
hdr = Path("build/disc1.header.bin").read_bytes()[:0x800].ljust(0x800, b"\x00")
main = Path("build/disc1.main.bin").read_bytes()
body = main[:0x1EE000].ljust(0x1EE000, b"\x00")
cand = hdr + body
Path("build/disc1.candidate.exe").write_bytes(cand)
orig = Path("build/extracted/disc1/SLUS_006.62").read_bytes()
print(f"  header: {len(hdr)} bytes (header match={hdr==orig[:0x800]})")
print(f"  main raw: {len(main)} (0x{len(main):X}); packed body: 0x1EE000")
print(f"  candidate: {len(cand)} (0x{len(cand):X})")
print(f"  orig SHA-1: {hashlib.sha1(orig).hexdigest()}")
print(f"  cand SHA-1: {hashlib.sha1(cand).hexdigest()}")
# Probe C leaf spans
leaf17e9c = slice(0x869C, 0x86A4)
leaf17fdc = slice(0x87DC, 0x87F0)
leaf17ff0 = slice(0x87F0, 0x8804)
leaf19050 = slice(0x9850, 0x9858)
leaf19058 = slice(0x9858, 0x9860)
leaf190ac = slice(0x98AC, 0x98B4)
leaf190b4 = slice(0x98B4, 0x98BC)
leaf192b8 = slice(0x9AB8, 0x9AC8)
leaf192c8 = slice(0x9AC8, 0x9ADC)
leaf38d0c = slice(0x2950C, 0x2951C)
leaf3d82c = slice(0x2E02C, 0x2E034)
leaf3dfc8 = slice(0x2E7C8, 0x2E7D0)
leaf3ffac = slice(0x307AC, 0x307BC)
leaf3ffbc = slice(0x307BC, 0x307CC)
leaf4da9c = slice(0x3E29C, 0x3E2A4)
leaf50d18 = slice(0x41518, 0x41520)
leaf51834 = slice(0x42034, 0x4204C)
leaf51e48 = slice(0x42648, 0x42658)
leaf52514 = slice(0x42D14, 0x42D24)
leaf52524 = slice(0x42D24, 0x42D34)
leaf5257c = slice(0x42D7C, 0x42D94)
leaf527c0 = slice(0x42FC0, 0x42FC8)
leaf5bca8 = slice(0x4C4A8, 0x4C4B0)
leaf5e884 = slice(0x4F084, 0x4F094)
leaf6ebd4 = slice(0x5F3D4, 0x5F3E4)
leaf73de8 = slice(0x645E8, 0x645F8)
leaf73df8 = slice(0x645F8, 0x64610)
leaf74a14 = slice(0x65214, 0x65228)
leaf74a28 = slice(0x65228, 0x65238)
leaf74cb8 = slice(0x654B8, 0x654C8)
leaf7633c = slice(0x66B3C, 0x66B54)
leaf7a324 = slice(0x6AB24, 0x6AB34)
leaf7a334 = slice(0x6AB34, 0x6AB44)
leaf7a344 = slice(0x6AB44, 0x6AB54)
leaf7a354 = slice(0x6AB54, 0x6AB60)
leaf7a3ec = slice(0x6ABEC, 0x6AC00)
leaf7a4a8 = slice(0x6ACA8, 0x6ACBC)
leaf7a4bc = slice(0x6ACBC, 0x6ACD0)
leaf7c130 = slice(0x6C930, 0x6C93C)
leaf7dea4 = slice(0x6E6A4, 0x6E6B0)
leaf7deb0 = slice(0x6E6B0, 0x6E6C0)
leaf7f778 = slice(0x6FF78, 0x6FF88)
leaf7fbcc = slice(0x703CC, 0x703D8)
leaf7fbd8 = slice(0x703D8, 0x703E4)
leaf7fbe4 = slice(0x703E4, 0x703F0)
leaf7fc08 = slice(0x70408, 0x70418)
leaf7fc18 = slice(0x70418, 0x70428)
leaf7fc28 = slice(0x70428, 0x70434)
leaf7fc34 = slice(0x70434, 0x70444)
leaf7fc44 = slice(0x70444, 0x70454)
leaf7fc54 = slice(0x70454, 0x70464)
leaf7fcac = slice(0x704AC, 0x704BC)
leaf80930 = slice(0x71130, 0x71140)
leaf80940 = slice(0x71140, 0x71150)
leaf80cc8 = slice(0x714C8, 0x714DC)
leaf81254 = slice(0x71A54, 0x71A68)
leaf822ac = slice(0x72AAC, 0x72ABC)
leaf82cdc = slice(0x734DC, 0x734F0)
leaf870e0 = slice(0x778E0, 0x778F0)
leaf87198 = slice(0x77998, 0x779AC)
leaf87414 = slice(0x77C14, 0x77C28)
leaf8ca7c = slice(0x7D27C, 0x7D284)
leaf8d7c0 = slice(0x7DFC0, 0x7DFD0)
leaf8f6a8 = slice(0x7FEA8, 0x7FEB0)
leaf8f = slice(0x7FE94, 0x7FEA8)
leaf8f868 = slice(0x80068, 0x80080)
leaf8f880 = slice(0x80080, 0x80098)
leaf8fcb4 = slice(0x804B4, 0x804BC)
leaf904a0 = slice(0x80CA0, 0x80CAC)
leaf904ac = slice(0x80CAC, 0x80CB4)
leaf904b4 = slice(0x80CB4, 0x80CBC)
leaf904bc = slice(0x80CBC, 0x80CC4)
leaf906b4 = slice(0x80EB4, 0x80EE4)
leaf0 = slice(0x8120C, 0x81220)
leaf1 = slice(0x81438, 0x8144C)
leaf2 = slice(0x8144C, 0x81460)
leaf3 = slice(0x81460, 0x81474)
leaf4 = slice(0x81474, 0x81488)
leaf5 = slice(0x81754, 0x81768)
print(f"  probe file 0x869C (17E9C): cand={cand[leaf17e9c].hex()} orig={orig[leaf17e9c].hex()}")
print(f"  probe file 0x87DC (17FDC): cand={cand[leaf17fdc].hex()} orig={orig[leaf17fdc].hex()}")
print(f"  probe file 0x87F0 (17FF0): cand={cand[leaf17ff0].hex()} orig={orig[leaf17ff0].hex()}")
print(f"  probe file 0x9850 (19050): cand={cand[leaf19050].hex()} orig={orig[leaf19050].hex()}")
print(f"  probe file 0x9858 (19058): cand={cand[leaf19058].hex()} orig={orig[leaf19058].hex()}")
print(f"  probe file 0x98AC (190AC): cand={cand[leaf190ac].hex()} orig={orig[leaf190ac].hex()}")
print(f"  probe file 0x98B4 (190B4): cand={cand[leaf190b4].hex()} orig={orig[leaf190b4].hex()}")
print(f"  probe file 0x9AB8 (192B8): cand={cand[leaf192b8].hex()} orig={orig[leaf192b8].hex()}")
print(f"  probe file 0x9AC8 (192C8): cand={cand[leaf192c8].hex()} orig={orig[leaf192c8].hex()}")
print(f"  probe file 0x2950C (38D0C): cand={cand[leaf38d0c].hex()} orig={orig[leaf38d0c].hex()}")
print(f"  probe file 0x2E02C (3D82C): cand={cand[leaf3d82c].hex()} orig={orig[leaf3d82c].hex()}")
print(f"  probe file 0x2E7C8 (3DFC8): cand={cand[leaf3dfc8].hex()} orig={orig[leaf3dfc8].hex()}")
print(f"  probe file 0x307AC (3FFAC): cand={cand[leaf3ffac].hex()} orig={orig[leaf3ffac].hex()}")
print(f"  probe file 0x307BC (3FFBC): cand={cand[leaf3ffbc].hex()} orig={orig[leaf3ffbc].hex()}")
leaf428c4 = slice(0x330C4, 0x330D4)
leaf42910 = slice(0x33110, 0x33128)
leaf42b28 = slice(0x33328, 0x33338)
leaf42b38 = slice(0x33338, 0x33350)
leaf42b50 = slice(0x33350, 0x3336C)
leaf42bc8 = slice(0x333C8, 0x333D8)
leaf42bd8 = slice(0x333D8, 0x333EC)
leaf42bec = slice(0x333EC, 0x33400)
leaf42c00 = slice(0x33400, 0x33414)
leaf42c14 = slice(0x33414, 0x33428)
leaf42c28 = slice(0x33428, 0x3343C)
leaf42c3c = slice(0x3343C, 0x33450)
leaf42c50 = slice(0x33450, 0x33464)
leaf42c64 = slice(0x33464, 0x33478)
print(f"  probe file 0x330C4 (428C4): cand={cand[leaf428c4].hex()} orig={orig[leaf428c4].hex()}")
print(f"  probe file 0x33110 (42910): cand={cand[leaf42910].hex()} orig={orig[leaf42910].hex()}")
print(f"  probe file 0x33328 (42B28): cand={cand[leaf42b28].hex()} orig={orig[leaf42b28].hex()}")
print(f"  probe file 0x33338 (42B38): cand={cand[leaf42b38].hex()} orig={orig[leaf42b38].hex()}")
print(f"  probe file 0x33350 (42B50): cand={cand[leaf42b50].hex()} orig={orig[leaf42b50].hex()}")
print(f"  probe file 0x333C8 (42BC8): cand={cand[leaf42bc8].hex()} orig={orig[leaf42bc8].hex()}")
print(f"  probe file 0x333D8 (42BD8): cand={cand[leaf42bd8].hex()} orig={orig[leaf42bd8].hex()}")
print(f"  probe file 0x333EC (42BEC): cand={cand[leaf42bec].hex()} orig={orig[leaf42bec].hex()}")
print(f"  probe file 0x33400 (42C00): cand={cand[leaf42c00].hex()} orig={orig[leaf42c00].hex()}")
print(f"  probe file 0x33414 (42C14): cand={cand[leaf42c14].hex()} orig={orig[leaf42c14].hex()}")
print(f"  probe file 0x33428 (42C28): cand={cand[leaf42c28].hex()} orig={orig[leaf42c28].hex()}")
print(f"  probe file 0x3343C (42C3C): cand={cand[leaf42c3c].hex()} orig={orig[leaf42c3c].hex()}")
print(f"  probe file 0x33450 (42C50): cand={cand[leaf42c50].hex()} orig={orig[leaf42c50].hex()}")
print(f"  probe file 0x33464 (42C64): cand={cand[leaf42c64].hex()} orig={orig[leaf42c64].hex()}")
print(f"  probe file 0x3E29C (4DA9C): cand={cand[leaf4da9c].hex()} orig={orig[leaf4da9c].hex()}")
print(f"  probe file 0x41518 (50D18): cand={cand[leaf50d18].hex()} orig={orig[leaf50d18].hex()}")
print(f"  probe file 0x42034 (51834): cand={cand[leaf51834].hex()} orig={orig[leaf51834].hex()}")
print(f"  probe file 0x42648 (51E48): cand={cand[leaf51e48].hex()} orig={orig[leaf51e48].hex()}")
print(f"  probe file 0x42D14 (52514): cand={cand[leaf52514].hex()} orig={orig[leaf52514].hex()}")
print(f"  probe file 0x42D24 (52524): cand={cand[leaf52524].hex()} orig={orig[leaf52524].hex()}")
print(f"  probe file 0x42D7C (5257C): cand={cand[leaf5257c].hex()} orig={orig[leaf5257c].hex()}")
print(f"  probe file 0x42FC0 (527C0): cand={cand[leaf527c0].hex()} orig={orig[leaf527c0].hex()}")
print(f"  probe file 0x4C4A8 (5BCA8): cand={cand[leaf5bca8].hex()} orig={orig[leaf5bca8].hex()}")
print(f"  probe file 0x4F084 (5E884): cand={cand[leaf5e884].hex()} orig={orig[leaf5e884].hex()}")
print(f"  probe file 0x5F3D4 (6EBD4): cand={cand[leaf6ebd4].hex()} orig={orig[leaf6ebd4].hex()}")
print(f"  probe file 0x645E8 (73DE8): cand={cand[leaf73de8].hex()} orig={orig[leaf73de8].hex()}")
print(f"  probe file 0x645F8 (73DF8): cand={cand[leaf73df8].hex()} orig={orig[leaf73df8].hex()}")
print(f"  probe file 0x65214 (74A14): cand={cand[leaf74a14].hex()} orig={orig[leaf74a14].hex()}")
print(f"  probe file 0x65228 (74A28): cand={cand[leaf74a28].hex()} orig={orig[leaf74a28].hex()}")
print(f"  probe file 0x654B8 (74CB8): cand={cand[leaf74cb8].hex()} orig={orig[leaf74cb8].hex()}")
print(f"  probe file 0x66B3C (7633C): cand={cand[leaf7633c].hex()} orig={orig[leaf7633c].hex()}")
print(f"  probe file 0x6AB24 (7A324): cand={cand[leaf7a324].hex()} orig={orig[leaf7a324].hex()}")
print(f"  probe file 0x6AB34 (7A334): cand={cand[leaf7a334].hex()} orig={orig[leaf7a334].hex()}")
print(f"  probe file 0x6AB44 (7A344): cand={cand[leaf7a344].hex()} orig={orig[leaf7a344].hex()}")
print(f"  probe file 0x6AB54 (7A354): cand={cand[leaf7a354].hex()} orig={orig[leaf7a354].hex()}")
print(f"  probe file 0x6ABEC (7A3EC): cand={cand[leaf7a3ec].hex()} orig={orig[leaf7a3ec].hex()}")
print(f"  probe file 0x6ACA8 (7A4A8): cand={cand[leaf7a4a8].hex()} orig={orig[leaf7a4a8].hex()}")
print(f"  probe file 0x6ACBC (7A4BC): cand={cand[leaf7a4bc].hex()} orig={orig[leaf7a4bc].hex()}")
print(f"  probe file 0x6C930 (7C130): cand={cand[leaf7c130].hex()} orig={orig[leaf7c130].hex()}")
print(f"  probe file 0x6E6A4 (7DEA4): cand={cand[leaf7dea4].hex()} orig={orig[leaf7dea4].hex()}")
print(f"  probe file 0x6E6B0 (7DEB0): cand={cand[leaf7deb0].hex()} orig={orig[leaf7deb0].hex()}")
print(f"  probe file 0x6FF78 (7F778): cand={cand[leaf7f778].hex()} orig={orig[leaf7f778].hex()}")
print(f"  probe file 0x703CC (7FBCC): cand={cand[leaf7fbcc].hex()} orig={orig[leaf7fbcc].hex()}")
print(f"  probe file 0x703D8 (7FBD8): cand={cand[leaf7fbd8].hex()} orig={orig[leaf7fbd8].hex()}")
print(f"  probe file 0x703E4 (7FBE4): cand={cand[leaf7fbe4].hex()} orig={orig[leaf7fbe4].hex()}")
print(f"  probe file 0x70408 (7FC08): cand={cand[leaf7fc08].hex()} orig={orig[leaf7fc08].hex()}")
print(f"  probe file 0x70418 (7FC18): cand={cand[leaf7fc18].hex()} orig={orig[leaf7fc18].hex()}")
print(f"  probe file 0x70428 (7FC28): cand={cand[leaf7fc28].hex()} orig={orig[leaf7fc28].hex()}")
print(f"  probe file 0x70434 (7FC34): cand={cand[leaf7fc34].hex()} orig={orig[leaf7fc34].hex()}")
print(f"  probe file 0x70444 (7FC44): cand={cand[leaf7fc44].hex()} orig={orig[leaf7fc44].hex()}")
print(f"  probe file 0x70454 (7FC54): cand={cand[leaf7fc54].hex()} orig={orig[leaf7fc54].hex()}")
print(f"  probe file 0x704AC (7FCAC): cand={cand[leaf7fcac].hex()} orig={orig[leaf7fcac].hex()}")
print(f"  probe file 0x71130 (80930): cand={cand[leaf80930].hex()} orig={orig[leaf80930].hex()}")
print(f"  probe file 0x71140 (80940): cand={cand[leaf80940].hex()} orig={orig[leaf80940].hex()}")
print(f"  probe file 0x714C8 (80CC8): cand={cand[leaf80cc8].hex()} orig={orig[leaf80cc8].hex()}")
print(f"  probe file 0x71A54 (81254): cand={cand[leaf81254].hex()} orig={orig[leaf81254].hex()}")
print(f"  probe file 0x72AAC (822AC): cand={cand[leaf822ac].hex()} orig={orig[leaf822ac].hex()}")
print(f"  probe file 0x734DC (82CDC): cand={cand[leaf82cdc].hex()} orig={orig[leaf82cdc].hex()}")
print(f"  probe file 0x778E0 (870E0): cand={cand[leaf870e0].hex()} orig={orig[leaf870e0].hex()}")
print(f"  probe file 0x77998 (87198): cand={cand[leaf87198].hex()} orig={orig[leaf87198].hex()}")
print(f"  probe file 0x77C14 (87414): cand={cand[leaf87414].hex()} orig={orig[leaf87414].hex()}")
print(f"  probe file 0x7D27C (8CA7C): cand={cand[leaf8ca7c].hex()} orig={orig[leaf8ca7c].hex()}")
print(f"  probe file 0x7DFC0 (8D7C0): cand={cand[leaf8d7c0].hex()} orig={orig[leaf8d7c0].hex()}")
print(f"  probe file 0x7FEA8 (8F6A8): cand={cand[leaf8f6a8].hex()} orig={orig[leaf8f6a8].hex()}")
print(f"  probe file 0x7FE94 (8F694): cand={cand[leaf8f].hex()} orig={orig[leaf8f].hex()}")
print(f"  probe file 0x80068 (8F868): cand={cand[leaf8f868].hex()} orig={orig[leaf8f868].hex()}")
print(f"  probe file 0x80080 (8F880): cand={cand[leaf8f880].hex()} orig={orig[leaf8f880].hex()}")
print(f"  probe file 0x804B4 (8FCB4): cand={cand[leaf8fcb4].hex()} orig={orig[leaf8fcb4].hex()}")
print(f"  probe file 0x80CA0 (904A0): cand={cand[leaf904a0].hex()} orig={orig[leaf904a0].hex()}")
print(f"  probe file 0x80CAC (904AC): cand={cand[leaf904ac].hex()} orig={orig[leaf904ac].hex()}")
print(f"  probe file 0x80CB4 (904B4): cand={cand[leaf904b4].hex()} orig={orig[leaf904b4].hex()}")
print(f"  probe file 0x80CBC (904BC): cand={cand[leaf904bc].hex()} orig={orig[leaf904bc].hex()}")
print(f"  probe file 0x80EB4 (906B4): cand={cand[leaf906b4].hex()} orig={orig[leaf906b4].hex()}")
print(f"  probe file 0x8120C (90A0C): cand={cand[leaf0].hex()} orig={orig[leaf0].hex()}")
print(f"  probe file 0x81438 (90C38): cand={cand[leaf1].hex()} orig={orig[leaf1].hex()}")
print(f"  probe file 0x8144C (90C4C): cand={cand[leaf2].hex()} orig={orig[leaf2].hex()}")
print(f"  probe file 0x81460 (90C60): cand={cand[leaf3].hex()} orig={orig[leaf3].hex()}")
print(f"  probe file 0x81474 (90C74): cand={cand[leaf4].hex()} orig={orig[leaf4].hex()}")
print(f"  probe file 0x81754 (90F54): cand={cand[leaf5].hex()} orig={orig[leaf5].hex()}")
leaf6 = slice(0xB3340, 0xB3350)
leafc2b50 = slice(0xB3350, 0xB3368)
print(f"  probe file 0xB3340 (C2B40): cand={cand[leaf6].hex()} orig={orig[leaf6].hex()}")
print(f"  probe file 0xB3350 (C2B50): cand={cand[leafc2b50].hex()} orig={orig[leafc2b50].hex()}")
leafc7dc4 = slice(0xB85C4, 0xB85D4)
print(f"  probe file 0xB85C4 (C7DC4): cand={cand[leafc7dc4].hex()} orig={orig[leafc7dc4].hex()}")
leafc7dd4 = slice(0xB85D4, 0xB85DC)
print(f"  probe file 0xB85D4 (C7DD4): cand={cand[leafc7dd4].hex()} orig={orig[leafc7dd4].hex()}")
leafc7ddc = slice(0xB85DC, 0xB85E4)
print(f"  probe file 0xB85DC (C7DDC): cand={cand[leafc7ddc].hex()} orig={orig[leafc7ddc].hex()}")
leaf7 = slice(0xB8A68, 0xB8A70)
print(f"  probe file 0xB8A68 (C8268): cand={cand[leaf7].hex()} orig={orig[leaf7].hex()}")
leafc8f08 = slice(0xB9708, 0xB9718)
print(f"  probe file 0xB9708 (C8F08): cand={cand[leafc8f08].hex()} orig={orig[leafc8f08].hex()}")
leafc8f18 = slice(0xB9718, 0xB9720)
print(f"  probe file 0xB9718 (C8F18): cand={cand[leafc8f18].hex()} orig={orig[leafc8f18].hex()}")
leafc8f20 = slice(0xB9720, 0xB9728)
print(f"  probe file 0xB9720 (C8F20): cand={cand[leafc8f20].hex()} orig={orig[leafc8f20].hex()}")
leaf8 = slice(0xB9A60, 0xB9A68)
print(f"  probe file 0xB9A60 (C9260): cand={cand[leaf8].hex()} orig={orig[leaf8].hex()}")
leaf9 = slice(0xBA6A0, 0xBA6A8)
print(f"  probe file 0xBA6A0 (C9EA0): cand={cand[leaf9].hex()} orig={orig[leaf9].hex()}")
leafc9c00 = slice(0xBA400, 0xBA410)
print(f"  probe file 0xBA400 (C9C00): cand={cand[leafc9c00].hex()} orig={orig[leafc9c00].hex()}")
leafca798 = slice(0xBAF98, 0xBAFA8)
print(f"  probe file 0xBAF98 (CA798): cand={cand[leafca798].hex()} orig={orig[leafca798].hex()}")
leaf10 = slice(0xBB4D4, 0xBB4DC)
print(f"  probe file 0xBB4D4 (CACD4): cand={cand[leaf10].hex()} orig={orig[leaf10].hex()}")
leafcbfa4 = slice(0xBC7A4, 0xBC7B4)
print(f"  probe file 0xBC7A4 (CBFA4): cand={cand[leafcbfa4].hex()} orig={orig[leafcbfa4].hex()}")
leafccf80 = slice(0xBD780, 0xBD790)
print(f"  probe file 0xBD780 (CCF80): cand={cand[leafccf80].hex()} orig={orig[leafccf80].hex()}")
leaf11 = slice(0xBDADC, 0xBDAE4)
print(f"  probe file 0xBDADC (CD2DC): cand={cand[leaf11].hex()} orig={orig[leaf11].hex()}")
leaf12 = slice(0xBDAE4, 0xBDAEC)
print(f"  probe file 0xBDAE4 (CD2E4): cand={cand[leaf12].hex()} orig={orig[leaf12].hex()}")
leaf13 = slice(0xBDD9C, 0xBDDA4)
print(f"  probe file 0xBDD9C (CD59C): cand={cand[leaf13].hex()} orig={orig[leaf13].hex()}")
leafcd960 = slice(0xBE160, 0xBE170)
print(f"  probe file 0xBE160 (CD960): cand={cand[leafcd960].hex()} orig={orig[leafcd960].hex()}")
leaf14 = slice(0xBE504, 0xBE50C)
print(f"  probe file 0xBE504 (CDD04): cand={cand[leaf14].hex()} orig={orig[leaf14].hex()}")
leafce1dc = slice(0xBE9DC, 0xBE9EC)
print(f"  probe file 0xBE9DC (CE1DC): cand={cand[leafce1dc].hex()} orig={orig[leafce1dc].hex()}")
leaf15 = slice(0xBEBAC, 0xBEBB4)
print(f"  probe file 0xBEBAC (CE3AC): cand={cand[leaf15].hex()} orig={orig[leaf15].hex()}")
leafd4850 = slice(0xC5050, 0xC5060)
print(f"  probe file 0xC5050 (D4850): cand={cand[leafd4850].hex()} orig={orig[leafd4850].hex()}")
if cand == orig:
    print("  RESULT: EXACT MATCH")
    sys.exit(0)
mism = sum(1 for a, b in zip(cand, orig) if a != b)
print(f"  RESULT: NON-MATCH ({mism}/{len(orig)} bytes differ, {100*mism/len(orig):.4f}%)")
for i, (a, b) in enumerate(zip(cand, orig)):
    if a != b:
        print(f"  first mismatch @ file 0x{i:X}: cand=0x{a:02X} orig=0x{b:02X}")
        break
print(f"  probe file 0x800 (jtbl start): cand={cand[0x800:0x808].hex()} orig={orig[0x800:0x808].hex()}")
print(f"  probe file 0x2A0C (text start): cand={cand[0x2A0C:0x2A10].hex()} orig={orig[0x2A0C:0x2A10].hex()}")
print(f"  probe file 0xB2AF8 (tail start): cand={cand[0xB2AF8:0xB2AFC].hex()} orig={orig[0xB2AF8:0xB2AFC].hex()}")
sys.exit(1)
PY
cmp_ec=$?
set -e

echo
echo "=== Summary ==="
echo "Assemble: OK (asm units + 35 gp carves)"
echo "Compile:  OK (207 C leaves (incl. gp batches + era + 5EF + 5EG + 5EH) with Phase 4J flags; func_80051E48 -fno-delayed-branch)"
echo "Pad trim: OK (incl. C .text pad strip for 0x14/0x18/0x30/0xC/0x8/0x10 bodies)"
echo "Link:     OK (ROM-order ld script + absolute symbol workarounds)"
echo "Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)"
if [[ "$cmp_ec" -eq 0 ]]; then
    echo "Compare:  EXACT SHA-1 MATCH"
    echo "Matching claim: YES (207 C leaves (incl. gp batches + era + 5EF + 5EG + 5EH) + remaining asm)"
    echo "Artifacts (git-ignored): build/asm/**/*.o build/src/*.o build/disc1.elf build/disc1.candidate.exe"
    exit 0
else
    echo "Compare:  NON-MATCH (see details above)"
    echo "Matching claim: NO"
    echo
    echo "If non-match persists after pad trim, investigate:"
    echo "  - C object padding / wrong trim target (expect body 0x14)"
    echo "  - relocatable .word/.jal / missing in-image dlabels (absolute fallbacks)"
    echo "  - splat stock linkers/disc1.ld section order (C layout vs ROM order)"
    echo
    echo "Artifacts (git-ignored): build/asm/**/*.o build/src/*.o build/disc1.elf build/disc1.candidate.exe"
    exit 1
fi
