#!/usr/bin/env bash
# Launch PCSX-Redux against canonical PE Disc 1 + SCPH-1001 + BTL83 capture.
# Does not poke guest RAM. Optional Day-1 Theater memcard is a retail save.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
APP_DIR="${PE_PCSX_DIR:-/var/home/blizz/Applications/pcsx-redux}"
APP="${PE_PCSX_APP:-$APP_DIR/PCSX-Redux-HEAD-x86_64.AppImage}"
PORTABLE="${PE_PCSX_PORTABLE:-$APP_DIR/portable-pe-btl83}"
DISC="${PE_DISC1:-/var/home/blizz/Projects/Parasite-Eve-Decompilation/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin}"
BIOS="${PE_BIOS:-$APP_DIR/bios/scph1001.bin}"
LUA="${PE_BTL83_LUA:-$ROOT/docs/evidence/pe-btl83-retail-battle-transition/capture.lua}"
OUT="${PE_BTL83_OUT:-$APP_DIR/captures/pe-btl83}"
GDB_PORT="${PE_GDB_PORT:-3334}"

mkdir -p "$PORTABLE" "$OUT"

if [[ ! -x "$APP" && ! -f "$APP" ]]; then
  echo "ERROR: PCSX-Redux AppImage missing: $APP" >&2
  exit 2
fi
if [[ ! -f "$DISC" ]]; then
  echo "ERROR: Disc 1 BIN missing: $DISC" >&2
  exit 2
fi
if [[ ! -f "$BIOS" ]]; then
  echo "ERROR: BIOS missing: $BIOS" >&2
  exit 2
fi
if [[ ! -f "$LUA" ]]; then
  echo "ERROR: capture.lua missing: $LUA" >&2
  exit 2
fi

echo "pcsx_app=$APP"
echo "pcsx_build=$(cat "$APP_DIR/BUILD_ID" 2>/dev/null || echo unknown)"
echo "disc=$DISC"
echo "bios=$BIOS"
echo "lua=$LUA"
echo "out=$OUT"
echo "portable=$PORTABLE"

export PE_BTL83_OUT="$OUT"
export PE_BTL83_NO_PAD="${PE_BTL83_NO_PAD:-0}"
export PE_BTL83_AUTO_LOAD="${PE_BTL83_AUTO_LOAD:-1}"

args=(
  -portable "$PORTABLE"
  -interpreter
  -debugger
  -gdb
  -gdb-port "$GDB_PORT"
  -softgpu
  -noupdate
  -fastboot
  -run
  -stdout
  -lua_stdout
  -no-gui-log
  -bios "$BIOS"
  -iso "$DISC"
  -dofile "$LUA"
)
if [[ "${PE_PCSX_CLI:-0}" == 1 ]]; then
  args+=(-cli)
fi

exec "$APP" "${args[@]}" "$@"
