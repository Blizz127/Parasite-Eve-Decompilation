#!/usr/bin/env bash
# Package the Parasite Eve native PC port as a private Banshee runtime.
#
# Produces a tar.zst with one top-level `runtime/` directory (the launcher's
# installer unwraps a single top-level dir):
#
#   runtime/parasite-eve-port   launch wrapper (executable; wires --disc-image)
#   runtime/bin/parasite-eve-port   native linux-x64 binary (Release)
#   runtime/data/disc1.bin      operator's own Disc 1 raw image (MODE2/2352)
#   runtime/data/disc2.bin      operator's own Disc 2 raw image (+ disc1/2.cue)
#   (win-x64: bin/parasite-eve-port.exe and a parasite-eve-port.cmd wrapper)
#   runtime/build-info.json     build identity + inventory hashes (schema 1)
#
# The package embeds operator-owned disc data, so it is PRIVATE: upload only
# to the private R2 bucket and distribute only via a signed channel URL.
# Nothing here writes game data into git — the default output directory is
# git-ignored and the script refuses a non-ignored output.
#
# Usage:
#   PE_BUILD_ID=PE-INV17-7821e7c3 PE_SOURCE_COMMIT=7821e7c3 \
#   PE_DISC="/path/to/Parasite Eve (USA) (Disc 1).bin" \
#   ./pc_port/package_runtime.sh
#
# Env (all optional):
#   PE_BUILD_ID       safe identifier, e.g. PE-INV17-<short-sha> (default derived)
#   PE_SOURCE_COMMIT  safe identifier for the source commit (default: short HEAD)
#   PE_DISC           Disc 1 raw image (default: auto-find under rom/image/)
#   PE_BINARY         native binary (default: pc_port/build-dist/parasite-eve-port)
#   PE_PACKAGE_THREADS compression workers (default: 4)
#   PE_PACKAGE_LEVEL zstd compression level 1..19 (default: 19)
#   PE_OUTPUT_DIR     package output (default: pc_port/build-dist/dist; must be git-ignored)
set -euo pipefail

COMPRESSION_LEVEL="${PE_PACKAGE_LEVEL:-19}"
if [[ ! "$COMPRESSION_LEVEL" =~ ^([1-9]|1[0-9])$ ]]; then
    echo "ERROR: PE_PACKAGE_LEVEL must be an integer from 1 to 19" >&2
    exit 1
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

SOURCE_COMMIT="${PE_SOURCE_COMMIT:-$(git rev-parse --short HEAD)}"
BUILD_ID="${PE_BUILD_ID:-PE-INV17-${SOURCE_COMMIT}}"
PLATFORM="${PE_PLATFORM:-linux-x64}"          # linux-x64 | win-x64
case "$PLATFORM" in
    linux-x64) BIN_NAME="parasite-eve-port";     EXEC_PATH="parasite-eve-port";     DEFAULT_BINARY="$ROOT/pc_port/build-dist/parasite-eve-port" ;;
    win-x64)   BIN_NAME="parasite-eve-port.exe"; EXEC_PATH="parasite-eve-port.cmd"; DEFAULT_BINARY="$ROOT/pc_port/build-win/parasite-eve-port.exe" ;;
    *) echo "ERROR: unknown PE_PLATFORM=$PLATFORM (linux-x64|win-x64)" >&2; exit 1 ;;
esac
BINARY="${PE_BINARY:-$DEFAULT_BINARY}"
OUT_DIR="${PE_OUTPUT_DIR:-$ROOT/pc_port/build-dist/dist}"
APP_ID="banshee.parasite-eve"

if [[ ! -f "$BINARY" ]]; then
    echo "ERROR: native binary not found: $BINARY" >&2
    echo "       Build it first: cmake -S pc_port -B pc_port/build-dist -DCMAKE_BUILD_TYPE=Release && cmake --build pc_port/build-dist -j8" >&2
    exit 1
fi

DISC="${PE_DISC:-}"
if [[ -z "$DISC" ]]; then
    DISC="$(find "$ROOT/rom/image" -iname "*disc 1*.bin" -print -quit 2>/dev/null || true)"
fi
if [[ -z "$DISC" || ! -f "$DISC" ]]; then
    echo "ERROR: Disc 1 image not found. Set PE_DISC=/path/to/disc1.bin" >&2
    exit 1
fi

DISC_SIZE="$(stat -c%s "$DISC")"
if (( DISC_SIZE % 2352 != 0 )); then
    echo "ERROR: disc image size $DISC_SIZE is not a multiple of 2352 (not a raw MODE2 image): $DISC" >&2
    exit 1
fi
DISC2="${PE_DISC2:-}"
if [[ -z "$DISC2" ]]; then
    DISC2="$(find "$ROOT/rom/image" -iname "*disc 2*.bin" -print -quit 2>/dev/null || true)"
fi
if [[ -z "$DISC2" || ! -f "$DISC2" ]]; then
    echo "ERROR: Disc 2 image not found. Set PE_DISC2=/path/to/disc2.bin" >&2
    exit 1
fi
DISC2_SIZE="$(stat -c%s "$DISC2")"
if (( DISC2_SIZE % 2352 != 0 )); then
    echo "ERROR: disc 2 image size $DISC2_SIZE is not a multiple of 2352: $DISC2" >&2
    exit 1
fi

if ! git check-ignore -q "$OUT_DIR" 2>/dev/null; then
    echo "ERROR: output dir is not git-ignored (game data must never land in git): $OUT_DIR" >&2
    echo "       Override with a git-ignored path via PE_OUTPUT_DIR=" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"
WORK="$(mktemp -d "$OUT_DIR/.pe-runtime-build.XXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
STAGE="$WORK/stage/runtime"
mkdir -p "$STAGE/bin" "$STAGE/data"

echo "==> Staging runtime (build $BUILD_ID, commit $SOURCE_COMMIT, platform $PLATFORM)"
install -m 0755 "$BINARY" "$STAGE/bin/$BIN_NAME"
cp --reflink=auto "$DISC" "$STAGE/data/disc1.bin"
cp --reflink=auto "$DISC2" "$STAGE/data/disc2.bin"
# Cue sidecars with the bundled file names (single MODE2/2352 data track).
for n in 1 2; do
    printf 'FILE "disc%s.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n' "$n" > "$STAGE/data/disc$n.cue"
done

if [[ "$PLATFORM" == "linux-x64" ]]; then
cat > "$STAGE/parasite-eve-port" <<'WRAPPER'
#!/usr/bin/env bash
# Launch wrapper: run the bundled native binary against the bundled
# operator-owned disc image, forwarding any extra arguments.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$HERE/bin/parasite-eve-port" --disc-image "$HERE/data/disc1.bin" "$@"
WRAPPER
chmod 0755 "$STAGE/parasite-eve-port"
else
cat > "$STAGE/parasite-eve-port.cmd" <<'WRAPPER'
@echo off
rem Launch wrapper: run the bundled native binary against the bundled
rem operator-owned disc image, forwarding any extra arguments.
setlocal
set "HERE=%~dp0"
start "" /D "%HERE%" "%HERE%bin\parasite-eve-port.exe" --disc-image "%HERE%data\disc1.bin" %*
endlocal
WRAPPER
fi

BIN_SHA="$(sha256sum "$STAGE/bin/$BIN_NAME" | awk '{print $1}')"
DATA_SHA="$(sha256sum "$STAGE/data/disc1.bin" | awk '{print $1}')"
DATA2_SHA="$(sha256sum "$STAGE/data/disc2.bin" | awk '{print $1}')"

python3 - "$STAGE/build-info.json" <<PY
import json, subprocess, sys
info = {
    "schemaVersion": 1,
    "appId": "$APP_ID",
    "buildId": "$BUILD_ID",
    "sourceCommit": "$SOURCE_COMMIT",
    "sourceDirty": bool(subprocess.check_output(["git", "status", "--porcelain"], text=True).strip()),
    "platform": "$PLATFORM",
    "executablePath": "$EXEC_PATH",
    "binaryPath": "bin/$BIN_NAME",
    "binarySha256": "$BIN_SHA",
    "discImage": "data/disc1.bin",
    "discSize": $DISC_SIZE,
    "discSha256": "$DATA_SHA",
    "discs": [
        {"path": "data/disc1.bin", "cue": "data/disc1.cue", "size": $DISC_SIZE, "sha256": "$DATA_SHA"},
        {"path": "data/disc2.bin", "cue": "data/disc2.cue", "size": $DISC2_SIZE, "sha256": "$DATA2_SHA"},
    ],
}
with open(sys.argv[1], "w") as f:
    json.dump(info, f, indent=2)
    f.write("\n")
PY

PKG_NAME="parasite-eve-${BUILD_ID}-${PLATFORM}.tar.zst"
PKG_PATH="$OUT_DIR/$PKG_NAME"
echo "==> Compressing $PKG_NAME (this takes a while for a full disc image)"
tar -C "$WORK/stage" -c runtime | zstd -T"${PE_PACKAGE_THREADS:-4}" -"$COMPRESSION_LEVEL" -o "$PKG_PATH"

PKG_SIZE="$(stat -c%s "$PKG_PATH")"
PKG_SHA="$(sha256sum "$PKG_PATH" | awk '{print $1}')"
echo "$PKG_SHA  $PKG_NAME" > "$PKG_PATH.sha256"

cat <<EOF

Package ready:
  path:   $PKG_PATH
  size:   $PKG_SIZE bytes
  sha256: $PKG_SHA
  build:  $BUILD_ID (commit $SOURCE_COMMIT, platform $PLATFORM)
  binary: $BIN_SHA
  disc1:  $DATA_SHA ($DISC_SIZE bytes)
  disc2:  $DATA2_SHA ($DISC2_SIZE bytes)

Next: upload to the private R2 bucket and publish the channel document.
EOF
