# Parasite Eve Native PC Port — Phase 6A

**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**Current milestone:** First native PE-driven clear frame via translated boot functions.

**Status:** NATIVE BLACK REACHED — translated PE boot chain presents the first native clear frame; matching build remains exact.

## Architecture

- **Build system:** CMake 3.16+
- **Backend:** Headless software framebuffer (no SDL2, no OpenGL, no OpenAL)
- **Output:** PPM (Portable Pixmap), deterministic byte-identical results
- **Game entry:** `port_main.c` → `func_8006E9A0_port.c` (PE_PORT adapted)
- **Memory strategy:** Typed host globals + arena buffer proxies
- **Disc strategy:** Bootstrap-disc mode (real-disc deferred)

## Building

```bash
cd pc_port
mkdir build && cd build
cmake ..
make
```

Produces:
- `parasite-eve-port` — native executable
- `pe-native-tests` — test suite

## Running

```bash
# Headless (recommended for CI/validation)
./parasite-eve-port --headless --bootstrap-disc \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Windowed (requires X11 display)
DISPLAY=:10.0 ./parasite-eve-port --bootstrap-disc \
  --screenshot /tmp/pe-black.ppm

# Strict mode — no bootstrap stubs allowed
./parasite-eve-port --headless --bootstrap-disc --strict-stubs
```

## Testing

```bash
cd pc_port/build
ctest          # or: ./pe-native-tests
```

## CLI arguments

| Flag | Description |
|------|-------------|
| `--headless` | Run without window |
| `--bootstrap-disc` | Use bootstrap disc adapter (Phase 6A only) |
| `--strict-stubs` | Trap on any unsupported stub |
| `--max-frames N` | Stop after N frames |
| `--screenshot PATH` | Output PPM screenshot |
| `--trace PATH` | Write boot trace |
| `--disc1 PATH` | Disc 1 image path (for real-disc mode) |
| `--assets PATH` | Asset directory path |

## What is real translated C

- `func_8006E9A0` — HOST_ADAPTED (register pins removed, arena globals proxied)
  - Calls VSync, SetDispMask, PutDispEnv, ClearImage, DrawSync through the PS1 SDK replacement layer
  - The ClearImage call is the one that produces the black frame

## What are bootstrap stubs

7 bootstrap stubs are invoked before the black frame:
- func_8006E834, func_8005E588, func_80066B60, ClearOTagR, func_80068E24, func_80070E54, func_80038D1C

Each is explicitly logged and classified. None are silent empty functions.

## Next steps after black

1. Compile more matched C functions natively (func_8005E588, func_80066B60, etc.)
2. Implement real disc I/O (libcd replacement)
3. Wire func_8001220C (main) through the full boot chain
4. Add SDL2 window for real display output
5. Implement GTE/MDEC for logo rendering
6. Audio, input, save/load

## Constraints

- PCSX-Redux is the retail oracle only — never used as runtime
- Matching decomp remains separate and SHA-exact
- No PS1 emulator in the native executable
- This is NOT a playable game yet
