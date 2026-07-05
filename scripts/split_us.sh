#!/usr/bin/env bash
# Run splat against configs/USA/disc1.yaml to produce the first disassembly
# of SLUS_006.62 locally. All output (asm/disc1/, linkers/disc1.ld,
# assets/disc1/) is git-ignored and must NEVER be committed.
#
# The canonical target is disc 1's SLUS_006.62 only: disc 2's SLUS_006.68 is
# byte-identical (see configs/USA/disc2.yaml and docs/disc_info.md).
#
# Prerequisites, checked below, in order:
#   1. configs/USA/disc1.yaml exists (committed; Phase 2).
#   2. build/extracted/disc1/SLUS_006.62 exists — produced locally by
#      `scripts/extract_us.sh 1` from a user-supplied disc image.
#   3. The extracted EXE's SHA-1 matches the recorded value; a mismatch
#      means the wrong or corrupt input and the split must not run.
#   4. `splat` is on PATH (install: pip install -U 'splat64[mips]';
#      version pinning is an open decision tracked in ACTIVE_HANDOFF.md).
#
# Running a split makes NO matching or rebuild claims — it only generates a
# first-pass disassembly to study. See docs/splitting.md.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONFIG="$ROOT/configs/USA/disc1.yaml"
EXE="$ROOT/build/extracted/disc1/SLUS_006.62"
EXPECTED_SHA1="452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

if [[ ! -f "$CONFIG" ]]; then
    echo "ERROR: missing $CONFIG" >&2
    exit 1
fi

if [[ ! -f "$EXE" ]]; then
    echo "ERROR: extracted executable not found: $EXE" >&2
    echo "Run 'scripts/extract_us.sh 1' first (needs your disc 1 image under rom/image/)." >&2
    exit 1
fi

actual_sha1="$(sha1sum "$EXE" | cut -d' ' -f1)"
if [[ "$actual_sha1" != "$EXPECTED_SHA1" ]]; then
    echo "ERROR: SHA-1 mismatch for $EXE" >&2
    echo "  expected: $EXPECTED_SHA1 (docs/disc_info.md)" >&2
    echo "  actual:   $actual_sha1" >&2
    echo "Refusing to split an unverified input. Re-run scripts/extract_us.sh 1." >&2
    exit 1
fi

if ! command -v splat >/dev/null; then
    echo "ERROR: 'splat' not found on PATH." >&2
    echo "Install with: pip install -U 'splat64[mips]'" >&2
    echo "(Exact version pinning is an open decision — see docs/ai_context/ACTIVE_HANDOFF.md.)" >&2
    exit 1
fi

echo "Input verified: $EXE (SHA-1 $actual_sha1)"
echo "Running: splat split $CONFIG"
splat split "$CONFIG"

echo
echo "Split complete. Generated locally (ALL git-ignored — never commit):"
echo "  asm/disc1/        first-pass disassembly"
echo "  linkers/disc1.ld  linker script"
echo "  assets/disc1/     extracted data segments (if any)"
echo "This is a study artifact only — no matching or rebuild claims."
