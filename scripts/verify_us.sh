#!/usr/bin/env bash
# Phase 4E: minimal verification harness for Disc 1 split artifacts.
#
# Checks that the local study state is sane:
#   - repo root + config present
#   - extracted SLUS_006.62 present and SHA-1 matches Phase 1 record
#   - split prerequisites (via scripts/split_us.sh --check)
#   - expected generated split outputs exist and are git-ignored
#   - config subsegment boundaries match the parked Phase 3 map
#   - pinned splat is available (and reports its version)
#
# Status layers reported at the end (honest, separate):
#   - split verification (this script, gates 1–7)
#   - asm-only rebuild status (optional presence of build/disc1.candidate.exe)
#   - C/matching status (always NOT IMPLEMENTED until harness proves it)
#
# Exit codes:
#   0  split artifacts / prerequisites OK (does NOT mean matching works)
#   1  a real problem was found
#   2  usage error
#
# Usage:
#   scripts/verify_us.sh
set -euo pipefail

if [[ $# -gt 0 ]]; then
    echo "Usage: $0" >&2
    echo "Phase 4E/4H verification — no flags yet." >&2
    exit 2
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# --- constants (must match docs/disc_info.md + configs/USA/disc1.yaml) ---
CONFIG="$ROOT/configs/USA/disc1.yaml"
EXE="$ROOT/build/extracted/disc1/SLUS_006.62"
EXPECTED_SHA1="452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
EXPECTED_SPLAT_PIN="0.41.0"

# Current production subsegments (file offsets). Phase 5DB: one-hundred-three C leaves.
EXPECTED_SUBSEGMENTS=(
    '[0x27C54, c, func_80037454]'
    '[0x27DB4, c, func_800375B4]'
    '[0x27DD0, c, func_800375D0]'
    '[0x29140, c, func_80038940]'
    '[0x116FC, c, func_80020EFC]'
    '[0x3DA88, c, func_8004D288]'
    '[0x4C498, c, func_8005BC98]'
    '[0x4CC88, c, func_8005C488]'
    '[0x436B0, c, func_80052EB0]'
    '[0x4F094, c, func_8005E894]'
    '[0x24220, c, func_80033A20]'
    '[0x28064, c, func_80037864]'
    '[0x336D0, c, func_80042ED0]'
    '[0x340E0, c, func_800438E0]'
    '[0x3DA7C, c, func_8004D27C]'
    '[0x3F170, c, func_8004E970]'
    '[0x41CF8, c, func_800514F8]'
    '[0x42658, c, func_80051E58]'
    '[0x42FB4, c, func_800527B4]'
    '[0x44A88, c, func_80054288]'
    '[0x44A94, c, func_80054294]'
    '[0x486CC, c, func_80057ECC]'
    '[0x4C09C, c, func_8005B89C]'
    '[0x4C4B0, c, func_8005BCB0]'
    '[0x4C6DC, c, func_8005BEDC]'
    '[0x4E920, c, func_8005E120]'
    '[0x51CA0, c, func_800614A0]'
    '[0x531B0, c, func_800629B0]'
    '[0x534C4, c, func_80062CC4]'
    '[0x55248, c, func_80064A48]'
    '[0x279A4, c, func_800371A4]'
    '[0x27DC4, c, func_800375C4]'
    '[0x334B8, c, func_80042CB8]'
    '[0x33738, c, func_80042F38]'
    '[0x33A40, c, func_80043240]'
    '[0x3D824, c, func_8004D024]'
    '[0x41D04, c, func_80051504]'
    '[0x4C090, c, func_8005B890]'
    '[0x4E914, c, func_8005E114]'
    '[0x4ED7C, c, func_8005E57C]'
    '[0x4EEE4, c, func_8005E6E4]'
    '[0x4F358, c, func_8005EB58]'
    '[0x4F6C8, c, func_8005EEC8]'
    '[0x52AB0, c, func_800622B0]'
    '[0x534B8, c, func_80062CB8]'
    '[0x800, rodata]'
    '[0x2A0C, asm]'
    '[0x869C, c, func_80017E9C]'
    '[0x86A4, asm]'
    '[0x9850, c, func_80019050]'
    '[0x9858, c, func_80019058]'
    '[0x9860, asm]'
    '[0x98AC, c, func_800190AC]'
    '[0x98B4, c, func_800190B4]'
    '[0x98BC, asm]'
    '[0x2950C, c, func_80038D0C]'
    '[0x2951C, asm]'
    '[0x2E02C, c, func_8003D82C]'
    '[0x2E034, asm]'
    '[0x2E7C8, c, func_8003DFC8]'
    '[0x2E7D0, asm]'
    '[0x307BC, c, func_8003FFBC]'
    '[0x307CC, asm]'
    '[0x330C4, c, func_800428C4]'
    '[0x330D4, asm]'
    '[0x33328, c, func_80042B28]'
    '[0x33338, asm]'
    '[0x333C8, c, func_80042BC8]'
    '[0x333D8, asm]'
    '[0x3E29C, c, func_8004DA9C]'
    '[0x3E2A4, asm]'
    '[0x41518, c, func_80050D18]'
    '[0x41520, asm]'
    '[0x42034, c, func_80051834]'
    '[0x4204C, asm]'
    '[0x42648, c, func_80051E48]'
    '[0x42D14, c, func_80052514]'
    '[0x42D24, c, func_80052524]'
    '[0x42D34, asm]'
    '[0x42D7C, c, func_8005257C]'
    '[0x42D94, asm]'
    '[0x42FC0, c, func_800527C0]'
    '[0x42FC8, asm]'
    '[0x4C4A8, c, func_8005BCA8]'
    '[0x4F084, c, func_8005E884]'
    '[0x53998, c, func_80063198]'
    '[0x539AC, c, func_800631AC]'
    '[0x539C0, asm]'
    '[0x55420, c, func_80064C20]'
    '[0x55430, asm]'
    '[0x5F3D4, c, func_8006EBD4]'
    '[0x5F3E4, asm]'
    '[0x645E8, c, func_80073DE8]'
    '[0x645F8, c, func_80073DF8]'
    '[0x64610, asm]'
    '[0x64B30, c, func_80074330]'
    '[0x64B54, asm]'
    '[0x64CA4, c, func_800744A4]'
    '[0x64CC8, asm]'
    '[0x64F4C, c, func_8007474C]'
    '[0x64F70, asm]'
    '[0x65228, c, func_80074A28]'
    '[0x65238, asm]'
    '[0x654B8, c, func_80074CB8]'
    '[0x654C8, asm]'
    '[0x66B3C, c, func_8007633C]'
    '[0x66B54, asm]'
    '[0x68228, c, func_80077A28]'
    '[0x6824C, asm]'
    '[0x68364, c, func_80077B64]'
    '[0x68378, asm]'
    '[0x68384, c, func_80077B84]'
    '[0x68398, asm]'
    '[0x683A4, c, func_80077BA4]'
    '[0x683B8, asm]'
    '[0x683C4, c, func_80077BC4]'
    '[0x683D8, asm]'
    '[0x683E4, c, func_80077BE4]'
    '[0x683F8, asm]'
    '[0x68404, c, func_80077C04]'
    '[0x68418, asm]'
    '[0x68424, c, func_80077C24]'
    '[0x68438, asm]'
    '[0x68444, c, func_80077C44]'
    '[0x68458, asm]'
    '[0x68464, c, func_80077C64]'
    '[0x68478, asm]'
    '[0x6AB24, c, func_8007A324]'
    '[0x6AB34, c, func_8007A334]'
    '[0x6AB44, c, func_8007A344]'
    '[0x6AB54, c, func_8007A354]'
    '[0x6AB60, asm]'
    '[0x6E6B0, c, func_8007DEB0]'
    '[0x6E6C0, asm]'
    '[0x6FF78, c, func_8007F778]'
    '[0x6FF88, asm]'
    '[0x70408, c, func_8007FC08]'
    '[0x70418, c, func_8007FC18]'
    '[0x70428, c, func_8007FC28]'
    '[0x70434, c, func_8007FC34]'
    '[0x70444, c, func_8007FC44]'
    '[0x70454, c, func_8007FC54]'
    '[0x70464, asm]'
    '[0x704AC, c, func_8007FCAC]'
    '[0x704BC, asm]'
    '[0x71140, c, func_80080940]'
    '[0x71150, asm]'
    '[0x72AAC, c, func_800822AC]'
    '[0x72ABC, asm]'
    '[0x73DA4, c, func_800835A4]'
    '[0x73DB0, c, func_800835B0]'
    '[0x73DC0, asm]'
    '[0x74670, c, func_80083E70]'
    '[0x74684, asm]'
    '[0x746E4, c, func_80083EE4]'
    '[0x746F8, asm]'
    '[0x74FA0, c, func_800847A0]'
    '[0x74FB0, asm]'
    '[0x778E0, c, func_800870E0]'
    '[0x778F0, asm]'
    '[0x7D27C, c, func_8008CA7C]'
    '[0x7D284, asm]'
    '[0x7DFC0, c, func_8008D7C0]'
    '[0x7DFD0, asm]'
    '[0x7E020, c, func_8008D820]'
    '[0x7E044, asm]'
    '[0x7FE94, c, func_8008F694]'
    '[0x7FEA8, c, func_8008F6A8]'
    '[0x7FEB0, asm]'
    '[0x80068, c, func_8008F868]'
    '[0x80080, c, func_8008F880]'
    '[0x80098, asm]'
    '[0x804B4, c, func_8008FCB4]'
    '[0x804BC, asm]'
    '[0x80CA0, c, func_800904A0]'
    '[0x80CAC, c, func_800904AC]'
    '[0x80CB4, c, func_800904B4]'
    '[0x80CBC, c, func_800904BC]'
    '[0x80CC4, asm]'
    '[0x80EB4, c, func_800906B4]'
    '[0x80EE4, asm]'
    '[0x8120C, c, func_80090A0C]'
    '[0x81220, asm]'
    '[0x81438, c, func_80090C38]'
    '[0x8144C, c, func_80090C4C]'
    '[0x81460, c, func_80090C60]'
    '[0x81474, c, func_80090C74]'
    '[0x81488, asm]'
    '[0x81754, c, func_80090F54]'
    '[0x81768, asm]'
    '[0x818A0, rodata]'
    '[0xB2AF8, asm]'
    '[0xB3340, c, func_800C2B40]'
    '[0xB3350, c, func_800C2B50]'
    '[0xB3368, asm]'
    '[0xB8A68, c, func_800C8268]'
    '[0xB8A70, asm]'
    '[0xB93B4, c, func_800C8BB4]'
    '[0xB93C0, asm]'
    '[0xB9A60, c, func_800C9260]'
    '[0xB9A68, asm]'
    '[0xBA168, c, func_800C9968]'
    '[0xBA174, asm]'
    '[0xBA6A0, c, func_800C9EA0]'
    '[0xBA6A8, asm]'
    '[0xBACA8, c, func_800CA4A8]'
    '[0xBACB4, asm]'
    '[0xBB4D4, c, func_800CACD4]'
    '[0xBB4DC, asm]'
    '[0xBC324, c, func_800CBB24]'
    '[0xBC330, asm]'
    '[0xBDADC, c, func_800CD2DC]'
    '[0xBDAE4, c, func_800CD2E4]'
    '[0xBDAEC, asm]'
    '[0xBDD9C, c, func_800CD59C]'
    '[0xBDDA4, c, func_800CD5A4]'
    '[0xBDDB0, asm]'
    '[0xBDF1C, c, func_800CD71C]'
    '[0xBDF28, asm]'
    '[0xBE504, c, func_800CDD04]'
    '[0xBE50C, asm]'
    '[0xBE740, c, func_800CDF40]'
    '[0xBE74C, asm]'
    '[0xBEBAC, c, func_800CE3AC]'
    '[0xBEBB4, asm]'
    '[0xBEC64, c, func_800CE464]'
    '[0xBEC70, asm]'
)

# Files a successful current-config split must produce (relative to ROOT).
EXPECTED_ARTIFACTS=(
    "asm/disc1/header.s"
    "asm/disc1/2A0C.s"
    "asm/disc1/86A4.s"
    "asm/disc1/9860.s"
    "asm/disc1/98BC.s"
    "asm/disc1/2951C.s"
    "asm/disc1/2E034.s"
    "asm/disc1/2E7D0.s"
    "asm/disc1/307CC.s"
    "asm/disc1/330D4.s"
    "asm/disc1/33338.s"
    "asm/disc1/333D8.s"
    "asm/disc1/3E2A4.s"
    "asm/disc1/41520.s"
    "asm/disc1/4204C.s"
    "asm/disc1/42658.s"
    "asm/disc1/42D34.s"
    "asm/disc1/42D94.s"
    "asm/disc1/42FC8.s"
    "asm/disc1/4C4B0.s"
    "asm/disc1/4F094.s"
    "asm/disc1/539C0.s"
    "asm/disc1/55430.s"
    "asm/disc1/5F3E4.s"
    "asm/disc1/64610.s"
    "asm/disc1/64B54.s"
    "asm/disc1/64CC8.s"
    "asm/disc1/64F70.s"
    "asm/disc1/65238.s"
    "asm/disc1/654C8.s"
    "asm/disc1/66B54.s"
    "asm/disc1/6824C.s"
    "asm/disc1/68378.s"
    "asm/disc1/68398.s"
    "asm/disc1/683B8.s"
    "asm/disc1/683D8.s"
    "asm/disc1/683F8.s"
    "asm/disc1/68418.s"
    "asm/disc1/68438.s"
    "asm/disc1/68458.s"
    "asm/disc1/68478.s"
    "asm/disc1/6AB60.s"
    "asm/disc1/6E6C0.s"
    "asm/disc1/6FF88.s"
    "asm/disc1/70464.s"
    "asm/disc1/704BC.s"
    "asm/disc1/71150.s"
    "asm/disc1/72ABC.s"
    "asm/disc1/73DC0.s"
    "asm/disc1/74684.s"
    "asm/disc1/746F8.s"
    "asm/disc1/74FB0.s"
    "asm/disc1/778F0.s"
    "asm/disc1/7D284.s"
    "asm/disc1/7DFD0.s"
    "asm/disc1/7E044.s"
    "asm/disc1/7FEB0.s"
    "asm/disc1/80098.s"
    "asm/disc1/804BC.s"
    "asm/disc1/80CC4.s"
    "asm/disc1/80EE4.s"
    "asm/disc1/81220.s"
    "asm/disc1/81488.s"
    "asm/disc1/81768.s"
    "asm/disc1/B2AF8.s"
    "asm/disc1/B3368.s"
    "asm/disc1/B8A70.s"
    "asm/disc1/B93C0.s"
    "asm/disc1/B9A68.s"
    "asm/disc1/BA174.s"
    "asm/disc1/BA6A8.s"
    "asm/disc1/BACB4.s"
    "asm/disc1/BB4DC.s"
    "asm/disc1/BC330.s"
    "asm/disc1/BDAEC.s"
    "asm/disc1/BDDB0.s"
    "asm/disc1/BDF28.s"
    "asm/disc1/BE50C.s"
    "asm/disc1/BE74C.s"
    "asm/disc1/BEBB4.s"
    "asm/disc1/BEC70.s"
    "asm/disc1/data/800.rodata.s"
    "asm/disc1/data/818A0.rodata.s"
    "src/func_80017E9C.c"
    "src/func_80019050.c"
    "src/func_80019058.c"
    "src/func_800190AC.c"
    "src/func_800190B4.c"
    "src/func_80038D0C.c"
    "src/func_8003D82C.c"
    "src/func_8003DFC8.c"
    "src/func_8003FFBC.c"
    "src/func_800428C4.c"
    "src/func_80042B28.c"
    "src/func_80042BC8.c"
    "src/func_8004DA9C.c"
    "src/func_80050D18.c"
    "src/func_80051834.c"
    "src/func_80051E48.c"
    "src/func_80052514.c"
    "src/func_80052524.c"
    "src/func_8005257C.c"
    "src/func_800527C0.c"
    "src/func_8005BCA8.c"
    "src/func_8005E884.c"
    "src/func_80063198.c"
    "src/func_800631AC.c"
    "src/func_80064C20.c"
    "src/func_8006EBD4.c"
    "src/func_80073DE8.c"
    "src/func_80073DF8.c"
    "src/func_80074330.c"
    "src/func_800744A4.c"
    "src/func_8007474C.c"
    "src/func_80074A28.c"
    "src/func_80074CB8.c"
    "src/func_8007633C.c"
    "src/func_80077A28.c"
    "src/func_80077B64.c"
    "src/func_80077B84.c"
    "src/func_80077BA4.c"
    "src/func_80077BC4.c"
    "src/func_80077BE4.c"
    "src/func_80077C04.c"
    "src/func_80077C24.c"
    "src/func_80077C44.c"
    "src/func_80077C64.c"
    "src/func_8007A324.c"
    "src/func_8007A334.c"
    "src/func_8007A344.c"
    "src/func_8007A354.c"
    "src/func_8007DEB0.c"
    "src/func_8007F778.c"
    "src/func_8007FC08.c"
    "src/func_8007FC18.c"
    "src/func_8007FC28.c"
    "src/func_8007FC34.c"
    "src/func_8007FC44.c"
    "src/func_8007FC54.c"
    "src/func_8007FCAC.c"
    "src/func_80080940.c"
    "src/func_800822AC.c"
    "src/func_800835A4.c"
    "src/func_800835B0.c"
    "src/func_80083E70.c"
    "src/func_80083EE4.c"
    "src/func_800847A0.c"
    "src/func_800870E0.c"
    "src/func_8008CA7C.c"
    "src/func_8008D7C0.c"
    "src/func_8008D820.c"
    "src/func_8008F694.c"
    "src/func_8008F6A8.c"
    "src/func_8008F868.c"
    "src/func_8008F880.c"
    "src/func_8008FCB4.c"
    "src/func_800904A0.c"
    "src/func_800904AC.c"
    "src/func_800904B4.c"
    "src/func_800904BC.c"
    "src/func_800906B4.c"
    "src/func_80090A0C.c"
    "src/func_80090C38.c"
    "src/func_80090C4C.c"
    "src/func_80090C60.c"
    "src/func_80090C74.c"
    "src/func_80090F54.c"
    "src/func_800C2B40.c"
    "src/func_800C2B50.c"
    "src/func_800C8268.c"
    "src/func_800C8BB4.c"
    "src/func_800C9260.c"
    "src/func_800C9968.c"
    "src/func_800C9EA0.c"
    "src/func_800CA4A8.c"
    "src/func_800CACD4.c"
    "src/func_800CBB24.c"
    "src/func_800CD2DC.c"
    "src/func_800CD2E4.c"
    "src/func_800CD59C.c"
    "src/func_800CD5A4.c"
    "src/func_800CD71C.c"
    "src/func_800CDD04.c"
    "src/func_800CDF40.c"
    "src/func_800CE3AC.c"
    "src/func_800CE464.c"
)

# Paths that must remain git-ignored (same set as split_us.sh OUTPUT_PATHS).
IGNORED_PATHS=(
    "asm/disc1"
    "linkers/disc1.ld"
    "assets/disc1"
    ".splache"
    "undefined_funcs_auto.txt"
    "undefined_syms_auto.txt"
    "include/gte_macros.inc"
    "include/include_asm.h"
    "include/labels.inc"
    "include/macro.inc"
)

failures=0
# All status lines go to stdout so the report stays ordered when captured.
pass() { echo "  OK  $*"; }
fail() {
    echo "  FAIL $*"
    failures=$((failures + 1))
}
hint() { echo "       $*"; }

echo "=== Phase 4E verification (split artifacts only) ==="
echo "root: $ROOT"
echo

# 1. Root guard.
echo "[1/7] Repo root"
if [[ ! -f "$ROOT/CLAUDE.md" || ! -d "$ROOT/configs/USA" ]]; then
    fail "not the Parasite-Eve-Decompilation root ($ROOT)"
else
    pass "CLAUDE.md + configs/USA present"
fi
if ! git -C "$ROOT" rev-parse --show-toplevel >/dev/null 2>&1; then
    fail "not inside a git repository"
else
    pass "git repository"
fi
echo

# 2. Config present + Phase 3 boundary markers.
echo "[2/7] Config boundaries (configs/USA/disc1.yaml)"
if [[ ! -f "$CONFIG" ]]; then
    fail "missing $CONFIG"
else
    pass "config present"
    for marker in "${EXPECTED_SUBSEGMENTS[@]}"; do
        if grep -Fq -- "$marker" "$CONFIG"; then
            pass "subsegment $marker"
        else
            fail "missing expected subsegment marker: $marker"
        fi
    done
fi
echo

# 3. Extracted EXE + SHA-1.
echo "[3/7] Extracted EXE + SHA-1"
if [[ ! -f "$EXE" ]]; then
    fail "missing $EXE"
    hint "Run: scripts/extract_us.sh 1  (needs disc image under rom/image/)"
else
    actual_sha1="$(sha1sum "$EXE" | cut -d' ' -f1)"
    if [[ "$actual_sha1" != "$EXPECTED_SHA1" ]]; then
        fail "SHA-1 mismatch for $EXE"
        hint "expected: $EXPECTED_SHA1 (docs/disc_info.md)"
        hint "actual:   $actual_sha1"
    else
        pass "SLUS_006.62 SHA-1 $actual_sha1"
    fi
fi
echo

# 4. split_us.sh --check (prerequisites: config, EXE hash, splat, gitignore).
echo "[4/7] scripts/split_us.sh --check"
if [[ ! -x "$ROOT/scripts/split_us.sh" ]]; then
    fail "scripts/split_us.sh missing or not executable"
else
    # Capture output so a nested failure is still one gate in this summary.
    if split_check_out="$("$ROOT/scripts/split_us.sh" --check 2>&1)"; then
        pass "split_us.sh --check passed"
        while IFS= read -r line; do
            [[ -n "$line" ]] && hint "$line"
        done <<<"$split_check_out"
    else
        fail "split_us.sh --check failed"
        while IFS= read -r line; do
            [[ -n "$line" ]] && hint "$line"
        done <<<"$split_check_out"
    fi
fi
echo

# 5. Pinned splat version.
echo "[5/7] splat version (pin $EXPECTED_SPLAT_PIN)"
SPLAT=""
if [[ -x "$ROOT/.venv/bin/splat" ]]; then
    SPLAT="$ROOT/.venv/bin/splat"
elif command -v splat >/dev/null 2>&1; then
    SPLAT="$(command -v splat)"
fi
if [[ -z "$SPLAT" ]]; then
    fail "splat not found (.venv/bin/splat missing and not on PATH)"
    hint "Run: scripts/setup_env.sh"
else
    # Prefer pip-reported package version (stable); fall back to --version.
    splat_ver=""
    if [[ -x "$ROOT/.venv/bin/pip" ]]; then
        splat_ver="$("$ROOT/.venv/bin/pip" show splat64 2>/dev/null | awk '/^Version:/{print $2}')"
    fi
    if [[ -z "$splat_ver" ]]; then
        splat_ver="$("$SPLAT" --version 2>/dev/null | head -n1 | grep -oE '[0-9]+\.[0-9]+(\.[0-9]+)?' | head -n1 || true)"
    fi
    if [[ -z "$splat_ver" ]]; then
        fail "could not determine splat version from $SPLAT"
    elif [[ "$splat_ver" != "$EXPECTED_SPLAT_PIN" ]]; then
        fail "splat version $splat_ver (expected pinned $EXPECTED_SPLAT_PIN) at $SPLAT"
    else
        pass "splat64 $splat_ver ($SPLAT)"
    fi
fi
echo

# 6. Expected generated artifacts present.
echo "[6/7] Expected split artifacts present"
missing_artifacts=0
for path in "${EXPECTED_ARTIFACTS[@]}"; do
    if [[ -f "$ROOT/$path" ]]; then
        pass "$path"
    else
        fail "missing $path"
        missing_artifacts=1
    fi
done
if [[ "$missing_artifacts" -ne 0 ]]; then
    hint "Run: scripts/split_us.sh  (after extract + setup_env)"
fi
echo

# 7. Output paths git-ignored.
echo "[7/7] Generated paths git-ignored"
for path in "${IGNORED_PATHS[@]}"; do
    if git -C "$ROOT" check-ignore -q "$path"; then
        pass "git-ignored: $path"
    else
        fail "not git-ignored: $path"
    fi
done
echo

# --- summary ---
echo "=== Summary ==="
if [[ "$failures" -eq 0 ]]; then
    echo "Split verification (Phase 4E): OK."
else
    echo "Split verification (Phase 4E): FAILED ($failures check(s) failed)."
fi

# Rebuild status (Phase 4H/5B) — report only; do not fail this script on non-match.
# Matching is a separate claim; scripts/build_us.sh owns the rebuild exit code.
echo "Rebuild status (scripts/build_us.sh):"
if [[ -x "$ROOT/scripts/build_us.sh" ]]; then
    echo "  scripts/build_us.sh: present"
    if [[ -f "$ROOT/build/disc1.candidate.exe" ]]; then
        cand_sha1="$(sha1sum "$ROOT/build/disc1.candidate.exe" | cut -d' ' -f1)"
        echo "  candidate: build/disc1.candidate.exe SHA-1 $cand_sha1"
        if [[ -f "$EXE" ]]; then
            if [[ "$cand_sha1" == "$EXPECTED_SHA1" ]]; then
                echo "  compare: EXACT MATCH to original"
            else
                echo "  compare: NON-MATCH (expected SHA-1 $EXPECTED_SHA1)"
                echo "  matching claim: NO — run scripts/build_us.sh for details"
            fi
        fi
    else
        echo "  candidate: not present (run scripts/build_us.sh)"
        echo "  matching claim: NO"
    fi
else
    echo "  scripts/build_us.sh: not present"
    echo "  matching claim: NO"
fi

if [[ -f "$ROOT/src/func_8008D820.c" && -f "$ROOT/src/func_80074330.c" && -f "$ROOT/src/func_80077A28.c" && -f "$ROOT/src/func_800CE464.c" && ! -f "$ROOT/src/func_8007FBF0.c" ]]; then
    echo "C conversion: Phase 5DE — 148 leaves (+ gp-relative setter batch)"
    echo "  sources: src/func_8008D820.c (+ one-hundred-two prior leaves)"
elif [[ -f "$ROOT/src/func_800CE464.c" && -f "$ROOT/src/func_800847A0.c" && -f "$ROOT/src/func_80077B64.c" && -f "$ROOT/src/func_80063198.c" && -f "$ROOT/src/func_80073DF8.c" && ! -f "$ROOT/src/func_8007FBF0.c" ]]; then
    echo "C conversion: Phase 5CW — ninety-eight leaves (+ store/setter batch through func_800CE464)"
    echo "  sources: src/func_800CE464.c (+ ninety-seven prior leaves)"
elif [[ -f "$ROOT/src/func_80073DF8.c" && -f "$ROOT/src/func_800C2B50.c" && -f "$ROOT/src/func_8008D7C0.c" && -f "$ROOT/src/func_8007633C.c" && -f "$ROOT/src/func_8005257C.c" && ! -f "$ROOT/src/func_8007FBF0.c" ]]; then
    echo "C conversion: Phase 5BX — seventy-three leaves (+ func_80073DF8 / prior through func_800CE3AC)"
    echo "  sources: src/func_80073DF8.c (+ seventy-two prior leaves)"
elif [[ -f "$ROOT/src/func_800C2B50.c" && -f "$ROOT/src/func_8008D7C0.c" && -f "$ROOT/src/func_8007633C.c" && -f "$ROOT/src/func_8005257C.c" && -f "$ROOT/src/func_8006EBD4.c" && ! -f "$ROOT/src/func_8007FBF0.c" ]]; then
    echo "C conversion: Phase 5BW — seventy-two leaves (+ func_8005257C / func_8007633C / func_8008D7C0 / func_800C2B50 / prior through func_800CE3AC)"
    echo "  sources: src/func_8005257C.c src/func_8007633C.c src/func_8008D7C0.c src/func_800C2B50.c (+ sixty-eight prior leaves)"
elif [[ -f "$ROOT/src/func_8006EBD4.c" && -f "$ROOT/src/func_8005E884.c" && -f "$ROOT/src/func_80038D0C.c" && -f "$ROOT/src/func_80042BC8.c" && ! -f "$ROOT/src/func_8007FBF0.c" ]]; then
    echo "C conversion: Phase 5BS — sixty-eight leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_80038D0C / func_8003D82C / func_8003DFC8 / func_8003FFBC / func_800428C4 / func_80042B28 / func_80042BC8 / func_8004DA9C / func_80050D18 / func_80051834 / func_80051E48 / func_80052514 / func_80052524 / func_800527C0 / func_8005BCA8 / func_8005E884 / func_8006EBD4 / func_80073DE8 / func_80074A28 / func_80074CB8 / func_8007A324 / func_8007A334 / func_8007A344 / func_8007A354 / func_8007DEB0 / func_8007F778 / func_8007FC08 / func_8007FC18 / func_8007FC28 / func_8007FC34 / func_8007FC44 / func_8007FC54 / func_8007FCAC / func_80080940 / func_800822AC / func_800870E0 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_80038D0C.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8003FFBC.c src/func_800428C4.c src/func_80042B28.c src/func_80042BC8.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051834.c src/func_80051E48.c src/func_80052514.c src/func_80052524.c src/func_800527C0.c src/func_8005BCA8.c src/func_8005E884.c src/func_8006EBD4.c src/func_80073DE8.c src/func_80074A28.c src/func_80074CB8.c src/func_8007A324.c src/func_8007A334.c src/func_8007A344.c src/func_8007A354.c src/func_8007DEB0.c src/func_8007F778.c src/func_8007FC08.c src/func_8007FC18.c src/func_8007FC28.c src/func_8007FC34.c src/func_8007FC44.c src/func_8007FC54.c src/func_8007FCAC.c src/func_80080940.c src/func_800822AC.c src/func_800870E0.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80042BC8.c" && -f "$ROOT/src/func_80074CB8.c" && -f "$ROOT/src/func_8007A354.c" && -f "$ROOT/src/func_8007A344.c" && -f "$ROOT/src/func_8007A334.c" && -f "$ROOT/src/func_8007A324.c" && -f "$ROOT/src/func_8007FC28.c" && -f "$ROOT/src/func_8007FC54.c" && -f "$ROOT/src/func_8007FC44.c" && -f "$ROOT/src/func_8007FC34.c" && -f "$ROOT/src/func_8007FC18.c" && -f "$ROOT/src/func_8007FC08.c" && -f "$ROOT/src/func_800870E0.c" && -f "$ROOT/src/func_800822AC.c" && -f "$ROOT/src/func_80080940.c" && -f "$ROOT/src/func_8007FCAC.c" && -f "$ROOT/src/func_8007F778.c" && -f "$ROOT/src/func_8007DEB0.c" && -f "$ROOT/src/func_80074A28.c" && -f "$ROOT/src/func_80042B28.c" && -f "$ROOT/src/func_80051834.c" && -f "$ROOT/src/func_80073DE8.c" && -f "$ROOT/src/func_8003FFBC.c" && -f "$ROOT/src/func_80052524.c" && -f "$ROOT/src/func_80052514.c" && -f "$ROOT/src/func_80051E48.c" && -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5BP — sixty-five leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_8003FFBC / func_800428C4 / func_80042B28 / func_80042BC8 / func_8004DA9C / func_80050D18 / func_80051834 / func_80051E48 / func_80052514 / func_80052524 / func_800527C0 / func_8005BCA8 / func_80073DE8 / func_80074A28 / func_80074CB8 / func_8007A324 / func_8007A334 / func_8007A344 / func_8007A354 / func_8007DEB0 / func_8007F778 / func_8007FC08 / func_8007FC18 / func_8007FC28 / func_8007FC34 / func_8007FC44 / func_8007FC54 / func_8007FCAC / func_80080940 / func_800822AC / func_800870E0 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8003FFBC.c src/func_800428C4.c src/func_80042B28.c src/func_80042BC8.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051834.c src/func_80051E48.c src/func_80052514.c src/func_80052524.c src/func_800527C0.c src/func_8005BCA8.c src/func_80073DE8.c src/func_80074A28.c src/func_80074CB8.c src/func_8007A324.c src/func_8007A334.c src/func_8007A344.c src/func_8007A354.c src/func_8007DEB0.c src/func_8007F778.c src/func_8007FC08.c src/func_8007FC18.c src/func_8007FC28.c src/func_8007FC34.c src/func_8007FC44.c src/func_8007FC54.c src/func_8007FCAC.c src/func_80080940.c src/func_800822AC.c src/func_800870E0.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8007FC28.c" && ! -f "$ROOT/src/func_8007A354.c" && -f "$ROOT/src/func_8007FC54.c" && -f "$ROOT/src/func_8007FC44.c" && -f "$ROOT/src/func_8007FC34.c" && -f "$ROOT/src/func_8007FC18.c" && -f "$ROOT/src/func_8007FC08.c" && -f "$ROOT/src/func_800870E0.c" && -f "$ROOT/src/func_800822AC.c" && -f "$ROOT/src/func_80080940.c" && -f "$ROOT/src/func_8007FCAC.c" && -f "$ROOT/src/func_8007F778.c" && -f "$ROOT/src/func_8007DEB0.c" && -f "$ROOT/src/func_80074A28.c" && -f "$ROOT/src/func_80042B28.c" && -f "$ROOT/src/func_80051834.c" && -f "$ROOT/src/func_80073DE8.c" && -f "$ROOT/src/func_8003FFBC.c" && -f "$ROOT/src/func_80052524.c" && -f "$ROOT/src/func_80052514.c" && -f "$ROOT/src/func_80051E48.c" && -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5BJ — fifty-nine leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_8003FFBC / func_800428C4 / func_80042B28 / func_8004DA9C / func_80050D18 / func_80051834 / func_80051E48 / func_80052514 / func_80052524 / func_800527C0 / func_8005BCA8 / func_80073DE8 / func_80074A28 / func_8007DEB0 / func_8007F778 / func_8007FC08 / func_8007FC18 / func_8007FC28 / func_8007FC34 / func_8007FC44 / func_8007FC54 / func_8007FCAC / func_80080940 / func_800822AC / func_800870E0 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8003FFBC.c src/func_800428C4.c src/func_80042B28.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051834.c src/func_80051E48.c src/func_80052514.c src/func_80052524.c src/func_800527C0.c src/func_8005BCA8.c src/func_80073DE8.c src/func_80074A28.c src/func_8007DEB0.c src/func_8007F778.c src/func_8007FC08.c src/func_8007FC18.c src/func_8007FC28.c src/func_8007FC34.c src/func_8007FC44.c src/func_8007FC54.c src/func_8007FCAC.c src/func_80080940.c src/func_800822AC.c src/func_800870E0.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8007FC54.c" && ! -f "$ROOT/src/func_8007FC28.c" && -f "$ROOT/src/func_8007FC44.c" && -f "$ROOT/src/func_8007FC34.c" && -f "$ROOT/src/func_8007FC18.c" && -f "$ROOT/src/func_8007FC08.c" && -f "$ROOT/src/func_800870E0.c" && -f "$ROOT/src/func_800822AC.c" && -f "$ROOT/src/func_80080940.c" && -f "$ROOT/src/func_8007FCAC.c" && -f "$ROOT/src/func_8007F778.c" && -f "$ROOT/src/func_8007DEB0.c" && -f "$ROOT/src/func_80074A28.c" && -f "$ROOT/src/func_80042B28.c" && -f "$ROOT/src/func_80051834.c" && -f "$ROOT/src/func_80073DE8.c" && -f "$ROOT/src/func_8003FFBC.c" && -f "$ROOT/src/func_80052524.c" && -f "$ROOT/src/func_80052514.c" && -f "$ROOT/src/func_80051E48.c" && -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5BI — fifty-eight leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_8003FFBC / func_800428C4 / func_80042B28 / func_8004DA9C / func_80050D18 / func_80051834 / func_80051E48 / func_80052514 / func_80052524 / func_800527C0 / func_8005BCA8 / func_80073DE8 / func_80074A28 / func_8007DEB0 / func_8007F778 / func_8007FC08 / func_8007FC18 / func_8007FC34 / func_8007FC44 / func_8007FC54 / func_8007FCAC / func_80080940 / func_800822AC / func_800870E0 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8003FFBC.c src/func_800428C4.c src/func_80042B28.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051834.c src/func_80051E48.c src/func_80052514.c src/func_80052524.c src/func_800527C0.c src/func_8005BCA8.c src/func_80073DE8.c src/func_80074A28.c src/func_8007DEB0.c src/func_8007F778.c src/func_8007FC08.c src/func_8007FC18.c src/func_8007FC34.c src/func_8007FC44.c src/func_8007FC54.c src/func_8007FCAC.c src/func_80080940.c src/func_800822AC.c src/func_800870E0.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8007FC18.c" && ! -f "$ROOT/src/func_8007FC54.c" && -f "$ROOT/src/func_8007FC08.c" && -f "$ROOT/src/func_800870E0.c" && -f "$ROOT/src/func_800822AC.c" && -f "$ROOT/src/func_80080940.c" && -f "$ROOT/src/func_8007FCAC.c" && -f "$ROOT/src/func_8007F778.c" && -f "$ROOT/src/func_8007DEB0.c" && -f "$ROOT/src/func_80074A28.c" && -f "$ROOT/src/func_80042B28.c" && -f "$ROOT/src/func_80051834.c" && -f "$ROOT/src/func_80073DE8.c" && -f "$ROOT/src/func_8003FFBC.c" && -f "$ROOT/src/func_80052524.c" && -f "$ROOT/src/func_80052514.c" && -f "$ROOT/src/func_80051E48.c" && -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5BF — fifty-five leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_8003FFBC / func_800428C4 / func_80042B28 / func_8004DA9C / func_80050D18 / func_80051834 / func_80051E48 / func_80052514 / func_80052524 / func_800527C0 / func_8005BCA8 / func_80073DE8 / func_80074A28 / func_8007DEB0 / func_8007F778 / func_8007FC08 / func_8007FC18 / func_8007FCAC / func_80080940 / func_800822AC / func_800870E0 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8003FFBC.c src/func_800428C4.c src/func_80042B28.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051834.c src/func_80051E48.c src/func_80052514.c src/func_80052524.c src/func_800527C0.c src/func_8005BCA8.c src/func_80073DE8.c src/func_80074A28.c src/func_8007DEB0.c src/func_8007F778.c src/func_8007FC08.c src/func_8007FC18.c src/func_8007FCAC.c src/func_80080940.c src/func_800822AC.c src/func_800870E0.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8007FC08.c" && -f "$ROOT/src/func_800870E0.c" && -f "$ROOT/src/func_800822AC.c" && -f "$ROOT/src/func_80080940.c" && -f "$ROOT/src/func_8007FCAC.c" && -f "$ROOT/src/func_8007F778.c" && -f "$ROOT/src/func_8007DEB0.c" && -f "$ROOT/src/func_80074A28.c" && -f "$ROOT/src/func_80042B28.c" && -f "$ROOT/src/func_80051834.c" && -f "$ROOT/src/func_80073DE8.c" && -f "$ROOT/src/func_8003FFBC.c" && -f "$ROOT/src/func_80052524.c" && -f "$ROOT/src/func_80052514.c" && -f "$ROOT/src/func_80051E48.c" && -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5BE — fifty-four leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_8003FFBC / func_800428C4 / func_80042B28 / func_8004DA9C / func_80050D18 / func_80051834 / func_80051E48 / func_80052514 / func_80052524 / func_800527C0 / func_8005BCA8 / func_80073DE8 / func_80074A28 / func_8007DEB0 / func_8007F778 / func_8007FC08 / func_8007FCAC / func_80080940 / func_800822AC / func_800870E0 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8003FFBC.c src/func_800428C4.c src/func_80042B28.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051834.c src/func_80051E48.c src/func_80052514.c src/func_80052524.c src/func_800527C0.c src/func_8005BCA8.c src/func_80073DE8.c src/func_80074A28.c src/func_8007DEB0.c src/func_8007F778.c src/func_8007FC08.c src/func_8007FCAC.c src/func_80080940.c src/func_800822AC.c src/func_800870E0.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80051E48.c" && -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AQ — forty leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_800428C4 / func_8004DA9C / func_80050D18 / func_80051E48 / func_800527C0 / func_8005BCA8 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_800428C4.c src/func_8004DA9C.c src/func_80050D18.c src/func_80051E48.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_800428C4.c" && -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AP — thirty-nine leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_800428C4 / func_8004DA9C / func_80050D18 / func_800527C0 / func_8005BCA8 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_800428C4.c src/func_8004DA9C.c src/func_80050D18.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8004DA9C.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AO — thirty-eight leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_8004DA9C / func_80050D18 / func_800527C0 / func_8005BCA8 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_8004DA9C.c src/func_80050D18.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_800190AC.c" && -f "$ROOT/src/func_800190B4.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AN — thirty-seven leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_800190AC / func_800190B4 / func_8003D82C / func_8003DFC8 / func_80050D18 / func_800527C0 / func_8005BCA8 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_800190AC.c src/func_800190B4.c src/func_8003D82C.c src/func_8003DFC8.c src/func_80050D18.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_80019058.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AM — thirty-five leaves (+ func_80017E9C / func_80019050 / func_80019058 / func_8003D82C / func_8003DFC8 / func_80050D18 / func_800527C0 / func_8005BCA8 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_80019058.c src/func_8003D82C.c src/func_8003DFC8.c src/func_80050D18.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80017E9C.c" && -f "$ROOT/src/func_80019050.c" && -f "$ROOT/src/func_8003D82C.c" && -f "$ROOT/src/func_8008CA7C.c" && -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AL — thirty-four leaves (+ func_80017E9C / func_80019050 / func_8003D82C / func_8003DFC8 / func_80050D18 / func_800527C0 / func_8005BCA8 / func_8008CA7C mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_80017E9C.c src/func_80019050.c src/func_8003D82C.c src/func_8003DFC8.c src/func_80050D18.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008CA7C.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8005BCA8.c" && -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AH — thirty leaves (+ func_8003DFC8 / func_80050D18 / func_800527C0 / func_8005BCA8 mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_8003DFC8.c src/func_80050D18.c src/func_800527C0.c src/func_8005BCA8.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_800527C0.c" && -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AG — twenty-nine leaves (+ func_8003DFC8 / func_80050D18 / func_800527C0 mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_8003DFC8.c src/func_80050D18.c src/func_800527C0.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AF — twenty-eight leaves (+ func_8003DFC8 / func_80050D18 mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_8003DFC8.c src/func_80050D18.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AE — twenty-seven leaves (+ func_8003DFC8 mid-2A0C; func_8008F6A8; tail through func_800CE3AC)"
    echo "  sources: src/func_8003DFC8.c src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AC — twenty-six leaves (+ func_8008F6A8 at 7FEA8 boundary; tail cluster through func_800CE3AC)"
    echo "  sources: src/func_8008F694.c src/func_8008F6A8.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AB — twenty-five leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 / func_800CACD4 / func_800CD2DC / func_800CD2E4 / func_800CD59C / func_800CDD04 / func_800CE3AC in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c src/func_800CE3AC.c"
elif [[ -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5AA — twenty-four leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 / func_800CACD4 / func_800CD2DC / func_800CD2E4 / func_800CD59C / func_800CDD04 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c src/func_800CDD04.c"
elif [[ -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5Z — twenty-three leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 / func_800CACD4 / func_800CD2DC / func_800CD2E4 / func_800CD59C in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c src/func_800CD59C.c"
elif [[ -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5Y — twenty-two leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 / func_800CACD4 / func_800CD2DC / func_800CD2E4 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c src/func_800CD2E4.c"
elif [[ -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5X — twenty-one leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 / func_800CACD4 / func_800CD2DC in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c src/func_800CD2DC.c"
elif [[ -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5W — twenty leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 / func_800CACD4 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c src/func_800CACD4.c"
elif [[ -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5V — nineteen leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 / func_800C9EA0 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c src/func_800C9EA0.c"
elif [[ -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5U — eighteen leaves (+ func_800C2B40 / func_800C8268 / func_800C9260 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c src/func_800C9260.c"
elif [[ -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5T — seventeen leaves (+ func_800C2B40 / func_800C8268 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c src/func_800C8268.c"
elif [[ -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5S — sixteen leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_800906B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5R — fifteen leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_800904BC.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5Q — fourteen leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_800904B4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5P — thirteen leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_800904AC.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5O — twelve leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_800904A0.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5N — eleven leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_8008FCB4.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5M — ten leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_8008F880.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
elif [[ -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5L — nine leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_8008F868.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5K — eight leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_8008F694.c src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5J — seven leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_80090A0C.c src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
    echo "C conversion: Phase 5G — six leaves (+ func_800C2B40 in tail)"
    echo "  sources: src/func_80090C{38,4C,60,74}.c src/func_80090F54.c src/func_800C2B40.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_80090C38.c" && -f "$ROOT/src/func_80090C4C.c" && -f "$ROOT/src/func_80090C60.c" && -f "$ROOT/src/func_80090C74.c" && -f "$ROOT/src/func_80090F54.c" ]]; then
    echo "C conversion: Phase 5F — five leaves (90C38, 90C4C, 90C60, 90C74, 90F54)"
    echo "  sources: src/func_80090C{38,4C,60,74}.c src/func_80090F54.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_80090C38.c" && -f "$ROOT/src/func_80090C4C.c" && -f "$ROOT/src/func_80090C60.c" && -f "$ROOT/src/func_80090F54.c" ]]; then
    echo "C conversion: Phase 5E — four leaves (90C38, 90C4C, 90C60, 90F54)"
    echo "  sources: src/func_80090C{38,4C,60}.c src/func_80090F54.c"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
elif [[ -f "$ROOT/src/func_80090C38.c" ]]; then
    echo "C conversion: earlier Phase 5 leaves present (see src/)"
    echo "  oracle: scripts/build_us.sh (exit 0 = exact SHA-1)"
else
    echo "C conversion: NOT IMPLEMENTED (missing src/func_80090C38.c)"
fi
echo

if [[ "$failures" -ne 0 ]]; then
    exit 1
fi
exit 0
