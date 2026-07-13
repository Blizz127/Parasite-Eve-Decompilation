#!/usr/bin/env bash
# scripts/setup_era.sh — fetch the era-accurate PS1 compiler toolchain used for
# functions that GCC 14.2 cannot match (Psy-Q ccpsx = GCC 2.7.2 fingerprint).
#
# Installs into tools/era/ (git-ignored):
#   tools/era/gcc-2.7.2-psx/{cpp,cc1,gcc,...}   from decompals/old-gcc (0.17)
#   tools/era/maspsx/                            from mkst/maspsx (assembler-macro layer)
#
# Idempotent: skips downloads that are already present. Requires network + curl + git.
# These are open-source community rebuilds / tools — NOT proprietary Psy-Q SDK files.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ERA="$ROOT/tools/era"
GCC_URL="https://github.com/decompals/old-gcc/releases/download/0.17/gcc-2.7.2-psx.tar.gz"
GCC_DIR="$ERA/gcc-2.7.2-psx"
MASPSX_REPO="https://github.com/mkst/maspsx"

mkdir -p "$ERA"

if [[ -x "$GCC_DIR/cc1" && -x "$GCC_DIR/cpp" ]]; then
    echo "OK  era gcc present: $GCC_DIR"
else
    echo "Fetching gcc-2.7.2-psx ..."
    mkdir -p "$GCC_DIR"
    tmp="$(mktemp)"
    curl -fsSL -o "$tmp" "$GCC_URL"
    tar xzf "$tmp" -C "$GCC_DIR"
    rm -f "$tmp"
    [[ -x "$GCC_DIR/cc1" ]] || { echo "ERROR: cc1 missing after extract" >&2; exit 1; }
    echo "OK  installed $GCC_DIR"
fi

if [[ -f "$ERA/maspsx/maspsx.py" ]]; then
    echo "OK  maspsx present: $ERA/maspsx"
else
    echo "Cloning maspsx ..."
    git clone -q --depth 1 "$MASPSX_REPO" "$ERA/maspsx"
    [[ -f "$ERA/maspsx/maspsx.py" ]] || { echo "ERROR: maspsx.py missing" >&2; exit 1; }
    echo "OK  cloned $ERA/maspsx"
fi

# Smoke test: era cc1 must reproduce the lui;ori const fingerprint GCC 14.2 can't.
echo "Smoke test: era const-synthesis fingerprint ..."
t="$(mktemp -d)"; printf 'int f(void){return 0x7F7F7F;}\n' >"$t/x.c"
"$GCC_DIR/cpp" "$t/x.c" >"$t/x.i" 2>/dev/null
"$GCC_DIR/cc1" -quiet -O2 -G0 "$t/x.i" -o "$t/x.s" 2>/dev/null
if grep -q 'ori' "$t/x.s"; then echo "OK  era emits lui;ori (matches retail)"; else
    echo "ERROR: era cc1 did not emit ori — wrong compiler?" >&2; exit 1; fi
rm -rf "$t"
echo "Era toolchain ready. build_us.sh will use it for ERA_LEAVES."
