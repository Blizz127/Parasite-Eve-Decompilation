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

# Current production subsegments (file offsets). Phase 5AF: twenty-eight C leaves.
EXPECTED_SUBSEGMENTS=(
    '[0x800, rodata]'
    '[0x2A0C, asm]'
    '[0x2E7C8, c, func_8003DFC8]'
    '[0x2E7D0, asm]'
    '[0x41518, c, func_80050D18]'
    '[0x41520, asm]'
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
    '[0xB3350, asm]'
    '[0xB8A68, c, func_800C8268]'
    '[0xB8A70, asm]'
    '[0xB9A60, c, func_800C9260]'
    '[0xB9A68, asm]'
    '[0xBA6A0, c, func_800C9EA0]'
    '[0xBA6A8, asm]'
    '[0xBB4D4, c, func_800CACD4]'
    '[0xBB4DC, asm]'
    '[0xBDADC, c, func_800CD2DC]'
    '[0xBDAE4, c, func_800CD2E4]'
    '[0xBDAEC, asm]'
    '[0xBDD9C, c, func_800CD59C]'
    '[0xBDDA4, asm]'
    '[0xBE504, c, func_800CDD04]'
    '[0xBE50C, asm]'
    '[0xBEBAC, c, func_800CE3AC]'
    '[0xBEBB4, asm]'
)

# Files a successful current-config split must produce (relative to ROOT).
EXPECTED_ARTIFACTS=(
    "asm/disc1/header.s"
    "asm/disc1/2A0C.s"
    "asm/disc1/2E7D0.s"
    "asm/disc1/41520.s"
    "asm/disc1/7FEB0.s"
    "asm/disc1/80098.s"
    "asm/disc1/804BC.s"
    "asm/disc1/80CC4.s"
    "asm/disc1/80EE4.s"
    "asm/disc1/81220.s"
    "asm/disc1/81488.s"
    "asm/disc1/81768.s"
    "asm/disc1/B2AF8.s"
    "asm/disc1/B3350.s"
    "asm/disc1/B8A70.s"
    "asm/disc1/B9A68.s"
    "asm/disc1/BA6A8.s"
    "asm/disc1/BB4DC.s"
    "asm/disc1/BDAEC.s"
    "asm/disc1/BDDA4.s"
    "asm/disc1/BE50C.s"
    "asm/disc1/BEBB4.s"
    "asm/disc1/data/800.rodata.s"
    "asm/disc1/data/818A0.rodata.s"
    "src/func_8003DFC8.c"
    "src/func_80050D18.c"
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
    "src/func_800C8268.c"
    "src/func_800C9260.c"
    "src/func_800C9EA0.c"
    "src/func_800CACD4.c"
    "src/func_800CD2DC.c"
    "src/func_800CD2E4.c"
    "src/func_800CD59C.c"
    "src/func_800CDD04.c"
    "src/func_800CE3AC.c"
    "linkers/disc1.ld"
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

if [[ -f "$ROOT/src/func_80050D18.c" && -f "$ROOT/src/func_8003DFC8.c" && -f "$ROOT/src/func_8008F6A8.c" && -f "$ROOT/src/func_800CE3AC.c" && -f "$ROOT/src/func_800CDD04.c" && -f "$ROOT/src/func_800CD59C.c" && -f "$ROOT/src/func_800CD2E4.c" && -f "$ROOT/src/func_800CD2DC.c" && -f "$ROOT/src/func_800CACD4.c" && -f "$ROOT/src/func_800C9EA0.c" && -f "$ROOT/src/func_800C9260.c" && -f "$ROOT/src/func_800C8268.c" && -f "$ROOT/src/func_800906B4.c" && -f "$ROOT/src/func_800904BC.c" && -f "$ROOT/src/func_800904B4.c" && -f "$ROOT/src/func_800904AC.c" && -f "$ROOT/src/func_800904A0.c" && -f "$ROOT/src/func_8008FCB4.c" && -f "$ROOT/src/func_8008F880.c" && -f "$ROOT/src/func_8008F868.c" && -f "$ROOT/src/func_8008F694.c" && -f "$ROOT/src/func_80090A0C.c" && -f "$ROOT/src/func_800C2B40.c" && -f "$ROOT/src/func_80090C74.c" ]]; then
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
