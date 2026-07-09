#!/usr/bin/env bash
# Phase 5G: Disc 1 rebuild with six C leaves
# (90C38, 90C4C, 90C60, 90C74, 90F54, C2B40).
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

# Phase 5G file-span sizes (config subsegment edges; exclusive end).
# 2A0C:     0x2A0C  → 0x81438 = 0x7EA2C
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
# B3350:    0xB3350 → 0x1EE800 = 0x13B4B0
# 818A0.rodata: 0x818A0 → 0xB2AF8 = 0x31258
SIZE_2A0C=0x7EA2C
SIZE_C_LEAF=0x14
SIZE_C_C2B40=0x10
SIZE_81488=0x2CC
SIZE_81768=0x138
SIZE_800_RODATA=0x220C
SIZE_B2AF8=0x848
SIZE_B3350=0x13B4B0
SIZE_818A0_RODATA=0x31258

# Object paths (ROM-order units; splat ld is still C-layout and unused for link).
OBJECTS=(
    "build/asm/disc1/header.s.o"
    "build/asm/disc1/data/800.rodata.s.o"
    "build/asm/disc1/2A0C.s.o"
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
    "build/asm/disc1/B3350.s.o"
)
SOURCES=(
    "asm/disc1/header.s"
    "asm/disc1/data/800.rodata.s"
    "asm/disc1/2A0C.s"
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
    "asm/disc1/B3350.s"
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
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/data/800.rodata.s.o   asm/disc1/data/800.rodata.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/data/818A0.rodata.s.o asm/disc1/data/818A0.rodata.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2A0C.s.o              asm/disc1/2A0C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/81488.s.o             asm/disc1/81488.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/81768.s.o             asm/disc1/81768.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B2AF8.s.o             asm/disc1/B2AF8.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B3350.s.o             asm/disc1/B3350.s

# --- compile C leaves ---
step "Compile C leaves (five 90Cxx/90F54 + func_800C2B40)"
# shellcheck disable=SC2086
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C38.c.o src/func_80090C38.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C4C.c.o src/func_80090C4C.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C60.c.o src/func_80090C60.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090C74.c.o src/func_80090C74.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_80090F54.c.o src/func_80090F54.c
run "$CC" $CFLAGS_LEAF -c -o build/src/func_800C2B40.c.o src/func_800C2B40.c

for o in "${OBJECTS[@]}"; do
    [[ -f "$o" ]] || die "build failed: missing $o"
    info "OK $o ($(wc -c <"$o") bytes)"
done

# Phase 4I / 5B–5G: GNU as / GCC pad section sizes to sh_addralign with trailing
# zeros (.text align 16 → object may be 0x20 for a 0x14-byte function body).
# Original PE1 file spans are exact; strip proven zero-only tail pad and lower
# align to 4 so ld does not re-insert gaps or shift ROM layout.
step "Trim ELF section alignment padding (Phase 4I/5B–5G)"
TRIM="$ROOT/tools/trim_elf_section_pad.py"
[[ -f "$TRIM" ]] || die "missing $TRIM"
python3 "$TRIM" build/asm/disc1/data/800.rodata.s.o .rodata "$SIZE_800_RODATA"
python3 "$TRIM" build/asm/disc1/2A0C.s.o .text "$SIZE_2A0C"
# Critical: each C object .text is typically 0x20 (align-16); real body is 0x14 or 0x10.
python3 "$TRIM" build/src/func_80090C38.c.o .text "$SIZE_C_LEAF"
python3 "$TRIM" build/src/func_80090C4C.c.o .text "$SIZE_C_LEAF"
python3 "$TRIM" build/src/func_80090C60.c.o .text "$SIZE_C_LEAF"
python3 "$TRIM" build/src/func_80090C74.c.o .text "$SIZE_C_LEAF"
python3 "$TRIM" build/asm/disc1/81488.s.o .text "$SIZE_81488"
python3 "$TRIM" build/src/func_80090F54.c.o .text "$SIZE_C_LEAF"
python3 "$TRIM" build/asm/disc1/81768.s.o .text "$SIZE_81768"
python3 "$TRIM" build/asm/disc1/B2AF8.s.o .text "$SIZE_B2AF8"
python3 "$TRIM" build/src/func_800C2B40.c.o .text "$SIZE_C_C2B40"
python3 "$TRIM" build/asm/disc1/B3350.s.o .text "$SIZE_B3350"
# 818A0.rodata and header already exact; still force align 4 if needed
python3 "$TRIM" build/asm/disc1/data/818A0.rodata.s.o .rodata "$SIZE_818A0_RODATA" || true
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
} >"$ABS_LD"

# ROM-order link: PE1 image is interleaved (prefix rodata, main text, mid
# rodata, tail text). splat's linkers/disc1.ld uses C layout (all .text then
# all .rodata) and is not used for the production pack.
ROM_ORDER_LD="build/disc1_romorder.ld"
cat >"$ROM_ORDER_LD" <<'LDEOF'
/* Phase 5G ROM-order link script (six C leaves).
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
        build/asm/disc1/B3350.s.o(.text)
        build/asm/disc1/2A0C.s.o(.data)
        build/src/func_80090C38.c.o(.data)
        build/src/func_80090C4C.c.o(.data)
        build/src/func_80090C60.c.o(.data)
        build/src/func_80090C74.c.o(.data)
        build/asm/disc1/81488.s.o(.data)
        build/src/func_80090F54.c.o(.data)
        build/asm/disc1/81768.s.o(.data)
        build/asm/disc1/B2AF8.s.o(.data)
        build/src/func_800C2B40.c.o(.data)
        build/asm/disc1/B3350.s.o(.data)
        build/asm/disc1/2A0C.s.o(.rodata)
        build/src/func_80090C38.c.o(.rodata)
        build/src/func_80090C4C.c.o(.rodata)
        build/src/func_80090C60.c.o(.rodata)
        build/src/func_80090C74.c.o(.rodata)
        build/asm/disc1/81488.s.o(.rodata)
        build/src/func_80090F54.c.o(.rodata)
        build/asm/disc1/81768.s.o(.rodata)
        build/asm/disc1/B2AF8.s.o(.rodata)
        build/src/func_800C2B40.c.o(.rodata)
        build/asm/disc1/B3350.s.o(.rodata)
        build/asm/disc1/2A0C.s.o(.bss)
        build/src/func_80090C38.c.o(.bss)
        build/src/func_80090C4C.c.o(.bss)
        build/src/func_80090C60.c.o(.bss)
        build/src/func_80090C74.c.o(.bss)
        build/asm/disc1/81488.s.o(.bss)
        build/src/func_80090F54.c.o(.bss)
        build/asm/disc1/81768.s.o(.bss)
        build/asm/disc1/B2AF8.s.o(.bss)
        build/src/func_800C2B40.c.o(.bss)
        build/asm/disc1/B3350.s.o(.bss)
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
# Probe C leaf spans (size 0x14 each)
leaf1 = slice(0x81438, 0x8144C)
leaf2 = slice(0x8144C, 0x81460)
leaf3 = slice(0x81460, 0x81474)
leaf4 = slice(0x81474, 0x81488)
leaf5 = slice(0x81754, 0x81768)
print(f"  probe file 0x81438 (90C38): cand={cand[leaf1].hex()} orig={orig[leaf1].hex()}")
print(f"  probe file 0x8144C (90C4C): cand={cand[leaf2].hex()} orig={orig[leaf2].hex()}")
print(f"  probe file 0x81460 (90C60): cand={cand[leaf3].hex()} orig={orig[leaf3].hex()}")
print(f"  probe file 0x81474 (90C74): cand={cand[leaf4].hex()} orig={orig[leaf4].hex()}")
print(f"  probe file 0x81754 (90F54): cand={cand[leaf5].hex()} orig={orig[leaf5].hex()}")
leaf6 = slice(0xB3340, 0xB3350)
print(f"  probe file 0xB3340 (C2B40): cand={cand[leaf6].hex()} orig={orig[leaf6].hex()}")
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
echo "Assemble: OK (8 asm units)"
echo "Compile:  OK (six C leaves with Phase 4J flags)"
echo "Pad trim: OK (incl. C .text pad strip for 0x14/0x10 bodies)"
echo "Link:     OK (ROM-order ld script + absolute symbol workarounds)"
echo "Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)"
if [[ "$cmp_ec" -eq 0 ]]; then
    echo "Compare:  EXACT SHA-1 MATCH"
    echo "Matching claim: YES (six C leaves + remaining asm)"
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
