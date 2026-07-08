#!/usr/bin/env bash
# Phase 4H: asm-only Disc 1 rebuild attempt (no C).
#
# Assembles splat-generated .s → .o with MIPS LE binutils, links, packs a
# PS-X EXE-sized candidate, and compares SHA-1 to the original.
#
# Toolchain (documented Phase 4G):
#   Distrobox container: pe-mipsel  (Debian trixie + binutils-mipsel-linux-gnu 2.44)
#   Tools: mipsel-linux-gnu-{as,ld,objcopy,objdump,readelf}
#   Assembler flags: -EL -mips1 -mabi=32 -I include/
#
# Exit codes:
#   0  exact SHA-1 match against original SLUS_006.62 (not yet achieved)
#   1  real failure, or rebuild produced a non-matching candidate
#   2  usage / missing prerequisites
#
# Usage:
#   scripts/build_us.sh              # assemble + link + pack + compare
#   scripts/build_us.sh --assemble-only
#   scripts/build_us.sh --help
#
# Does NOT claim matching unless compare succeeds. Does NOT convert C.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

MODE="full"
if [[ $# -gt 0 ]]; then
    case "$1" in
        --assemble-only) MODE="assemble" ;;
        --help|-h)
            sed -n '2,25p' "$0" | sed 's/^# \?//'
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

# Object paths must match splat-generated linkers/disc1.ld (when used).
OBJECTS=(
    "build/asm/disc1/header.s.o"
    "build/asm/disc1/2A0C.s.o"
    "build/asm/disc1/B2AF8.s.o"
    "build/asm/disc1/data/800.rodata.s.o"
    "build/asm/disc1/data/818A0.rodata.s.o"
)
SOURCES=(
    "asm/disc1/header.s"
    "asm/disc1/2A0C.s"
    "asm/disc1/B2AF8.s"
    "asm/disc1/data/800.rodata.s"
    "asm/disc1/data/818A0.rodata.s"
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
info "OK all five split sources present"
[[ -f "$ROOT/linkers/disc1.ld" ]] || die "missing linkers/disc1.ld — run scripts/split_us.sh"
info "OK linkers/disc1.ld present"

# --- toolchain discovery ---
step "Toolchain"
AS="" LD="" OBJCOPY="" READELF=""
if command -v mipsel-linux-gnu-as >/dev/null 2>&1; then
    AS="$(command -v mipsel-linux-gnu-as)"
    LD="$(command -v mipsel-linux-gnu-ld)"
    OBJCOPY="$(command -v mipsel-linux-gnu-objcopy)"
    READELF="$(command -v mipsel-linux-gnu-readelf)"
    TOOL_NOTE="host PATH"
elif command -v distrobox >/dev/null 2>&1 && distrobox list 2>/dev/null | grep -q 'pe-mipsel'; then
    # Run later steps via distrobox enter
    RUNNER=(distrobox enter pe-mipsel --)
    AS="mipsel-linux-gnu-as"
    LD="mipsel-linux-gnu-ld"
    OBJCOPY="mipsel-linux-gnu-objcopy"
    READELF="mipsel-linux-gnu-readelf"
    TOOL_NOTE="distrobox pe-mipsel"
else
    die "mipsel-linux-gnu-as not on PATH and Distrobox pe-mipsel not found.
Install/use Phase 4G path: distrobox create -i docker.io/library/debian:trixie -n pe-mipsel
  then: distrobox enter pe-mipsel -- sudo apt-get install -y binutils-mipsel-linux-gnu
Or put mipsel-linux-gnu-{as,ld,objcopy,readelf} on PATH."
fi

# If tools are on host PATH, no runner prefix.
if [[ -z "${RUNNER+x}" ]]; then
    RUNNER=()
fi

run() { "${RUNNER[@]}" "$@"; }

AS_VER="$(run "$AS" --version 2>/dev/null | head -n1 || true)"
LD_VER="$(run "$LD" --version 2>/dev/null | head -n1 || true)"
info "using: $TOOL_NOTE"
info "as: $AS ($AS_VER)"
info "ld: $LD ($LD_VER)"
info "assembler flags: $ASFLAGS_DEFAULT -I $ROOT/include"

# --- assemble ---
step "Assemble (5 units → build/asm/disc1/**/*.s.o)"
mkdir -p build/asm/disc1/data
# shellcheck disable=SC2086
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/header.s.o            asm/disc1/header.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/data/800.rodata.s.o   asm/disc1/data/800.rodata.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/data/818A0.rodata.s.o asm/disc1/data/818A0.rodata.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/2A0C.s.o              asm/disc1/2A0C.s
run "$AS" $ASFLAGS_DEFAULT -I "$ROOT/include" -o build/asm/disc1/B2AF8.s.o             asm/disc1/B2AF8.s
for o in "${OBJECTS[@]}"; do
    [[ -f "$o" ]] || die "assemble failed: missing $o"
    info "OK $o ($(wc -c <"$o") bytes)"
done

if [[ "$MODE" == "assemble" ]]; then
    echo
    echo "Assemble-only complete. Objects under build/asm/ (git-ignored)."
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

# First link attempt may reveal in-image labels never emitted as dlabels.
# Capture undefined D_* and provide absolute fallbacks (honest workaround).
ROM_ORDER_LD="build/disc1_romorder.ld"
cat >"$ROM_ORDER_LD" <<'LDEOF'
/* Phase 4H experimental ROM-order link script.
 * splat's linkers/disc1.ld places all .text then all .rodata (C layout).
 * PE1 image order is interleaved: prefix rodata, main text, mid rodata, tail text.
 * This script places inputs in file order for PS-X EXE packing experiments.
 * NOT a matching claim.
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
        build/asm/disc1/data/818A0.rodata.s.o(.rodata)
        build/asm/disc1/B2AF8.s.o(.text)
        build/asm/disc1/2A0C.s.o(.data)
        build/asm/disc1/B2AF8.s.o(.data)
        build/asm/disc1/2A0C.s.o(.rodata)
        build/asm/disc1/B2AF8.s.o(.rodata)
        build/asm/disc1/2A0C.s.o(.bss)
        build/asm/disc1/B2AF8.s.o(.bss)
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

step "Link (ROM-order experimental script)"
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
    echo "Asm-only rebuild: LINK FAILED"
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
echo "Assemble: OK (5/5 objects)"
echo "Link:     OK (ROM-order experimental ld script + absolute symbol workarounds)"
echo "Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)"
if [[ "$cmp_ec" -eq 0 ]]; then
    echo "Compare:  EXACT SHA-1 MATCH"
    echo "Matching claim: YES (asm-only rebuild)"
    exit 0
else
    echo "Compare:  NON-MATCH (see details above)"
    echo "Matching claim: NO"
    echo
    echo "Known non-match causes (Phase 4H):"
    echo "  - gas/ld section alignment can expand .text/.rodata vs original sizes"
    echo "  - relocatable .word/.jal need link; layout must match original VRAM map"
    echo "  - ~134 in-image D_* labels missing from split (absolute fallbacks used)"
    echo "  - splat linkers/disc1.ld uses C section order (all text then rodata); ROM-order script is experimental"
    echo
    echo "Artifacts (git-ignored): build/asm/**/*.o build/disc1.elf build/disc1.candidate.exe"
    exit 1
fi
