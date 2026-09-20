#!/usr/bin/env bash
# setup_m2c.sh — fetch the m2c MIPS->C decompiler used as a *triage aid*.
#
# m2c (https://github.com/matt-kempster/m2c) translates a MIPS assembly
# function into candidate C. It is NOT an authority: its output is a
# first-draft hypothesis that must still be driven to a byte-exact match by
# tools/analysis/try_leaf.py and finally by scripts/build_us.sh. It exists to
# remove the blank-page cost from the ~1600 remaining retail functions.
#
# Everything is installed under tools/era/ (git-ignored, like the rest of the
# era toolchain) so no third-party source enters the repository. The pinned
# commit keeps runs reproducible.
#
# Usage: bash scripts/setup_m2c.sh [--check]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
M2C_COMMIT=708d2d2cb2698f091a92492b328f73b24209f72d
SRC_DIR="$ROOT/tools/era/m2c"
VENV_DIR="$ROOT/tools/era/m2c-venv"
STAMP="$SRC_DIR/.m2c-commit"

if [[ "${1:-}" == "--check" ]]; then
    if [[ -x "$VENV_DIR/bin/python" && -f "$STAMP" ]] \
       && [[ "$(cat "$STAMP")" == "$M2C_COMMIT" ]]; then
        echo "m2c: OK ($M2C_COMMIT)"
        exit 0
    fi
    echo "m2c: MISSING (run scripts/setup_m2c.sh)" >&2
    exit 1
fi

if [[ -x "$VENV_DIR/bin/python" && -f "$STAMP" ]] \
   && [[ "$(cat "$STAMP")" == "$M2C_COMMIT" ]] \
   && [[ -f "$SRC_DIR/m2c.py" ]]; then
    echo "m2c already present at $M2C_COMMIT"
    exit 0
fi

echo "==> cloning m2c @ $M2C_COMMIT"
rm -rf "$SRC_DIR"
mkdir -p "$(dirname "$SRC_DIR")"
git clone --quiet https://github.com/matt-kempster/m2c.git "$SRC_DIR"
git -C "$SRC_DIR" checkout --quiet "$M2C_COMMIT"
printf '%s\n' "$M2C_COMMIT" > "$STAMP"

echo "==> creating venv at $VENV_DIR"
if [[ ! -x "$VENV_DIR/bin/python" ]]; then
    python3 -m venv "$VENV_DIR"
fi
"$VENV_DIR/bin/pip" install --quiet --upgrade pip >/dev/null 2>&1 || true
"$VENV_DIR/bin/pip" install --quiet pycparser

echo "==> smoke test"
"$VENV_DIR/bin/python" "$SRC_DIR/m2c.py" --help >/dev/null
echo "m2c ready: $VENV_DIR/bin/python $SRC_DIR/m2c.py"
