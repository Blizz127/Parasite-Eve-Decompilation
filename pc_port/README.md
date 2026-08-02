# Parasite Eve Native PC Port — Phase 6D-R

**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**Current milestone:** Translated Boot Rung verified — six real PS1 functions route
the retail main path through native arena layout, display init, and subsystem init
to produce a deterministic black framebuffer.

**Status:** BOOT RUNG VERIFIED — 6 translated Boot Rung functions proven by 39
native tests; 3 deterministic headless runs byte-identical; strict mode correctly
rejects unresolved providers; matching build remains exact at SHA `452fb033`.

## Architecture

- **Build system:** CMake 3.16+
- **Backend:** Headless software framebuffer + X11 window via dlopen
- **Output:** PPM (Portable Pixmap), deterministic byte-identical results
- **Game entry:** `port_main.c` → `func_8001220C_port.c` → full PE boot chain
- **Memory strategy:** Typed host globals + 4× 0x8000 arena buffer proxies
- **Disc strategy:** Bootstrap-disc mode (real-disc deferred to Phase 6E)

## Translated Boot Rung functions (Phase 6D/6D-R)

| Function | Matching size | Words | Purpose |
|----------|--------------|-------|---------|
| `func_8003E610` | 0x70 | 28 | Display/graphics bring-up (10 straight-line calls) |
| `func_8003E680` | 0xD4 | 53 | Subsystem-init dispatcher (5 globals, 2000 polls, callback) |
| `func_8006A5BC` | 0x90 | 36 | Boot init with two VSync-polled wait loops |
| `func_8006A64C` | 0x28 | **10** | Boot memory-layout wrapper |
| `func_8006A674` | 0x260 | 152 | Boot state initializer with five counting loops |
| `func_8006A8D4` | 0x110 | **68** | Memory-region layout (19 pointer assignments) |

## Building

```bash
cd pc_port
mkdir build && cd build
cmake ..
make
```

Produces:
- `parasite-eve-port` — native executable
- `pe-native-tests` — test suite (39 tests, all pass)

## Running

```bash
# Headless deterministic (CI/validation)
./parasite-eve-port --headless --bootstrap-disc \
  --stop-after-event first-clear \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Windowed (requires X11 display)
DISPLAY=:10.0 ./parasite-eve-port --bootstrap-disc \
  --stop-after-event first-clear --hold-ms 5000 --debug-overlay

# Strict mode — stops at first unresolved provider
./parasite-eve-port --headless --strict-stubs \
  --stop-after-event first-clear
# Expected: exit 1, names func_8007F72C (CdReady)
```

## Testing

```bash
cd pc_port/build
./pe-native-tests   # 39 tests (12 baseline + 27 Boot Rung)
```

## Deterministic framebuffer

| Property | Value |
|----------|-------|
| SHA-256 (headless, 3 runs) | `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` |
| Windowed SHA matches headless | ✅ |
| main iterations | 1 (consistent across all runs) |
| Unsupported traps | 0 |
| Bootstrap stubs invoked | 51 |

## Next steps

1. **Phase 6E:** Replace bootstrap-disc with real Disc 1 + PE.IMG reads
2. Identify the first boot asset (likely MDEC logo data)
3. Wire MDEC decoding and display
4. Audio, input, save/load (later phases)

## Constraints

- PCSX-Redux is the retail oracle only — never used as runtime
- Matching decomp remains separate and SHA-exact (229 C leaves, SHA `452fb033`)
- No PS1 emulator in the native executable
- This is NOT a playable game yet
