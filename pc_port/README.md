# Parasite Eve Native PC Port — Phase 6D-S

**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**Current milestone:** Host-safe guest-memory boot foundation — translated
Boot Rung runs entirely on a contiguous 2 MiB guest RAM model with typed
`pe_addr_t` addresses, a full-width callback registry, and a centralized
bootstrap/strict-mode policy.

**Status:** HOST-SAFE BOOT FOUNDATION VERIFIED — 81 native tests pass;
ASan/UBSan clean; 3 deterministic headless runs byte-identical; strict mode
aborts centrally at the first unresolved provider; windowed SHA matches
headless; matching build remains exact at SHA `452fb033`.

## Architecture

- **Build system:** CMake 3.16+
- **Backend:** Headless software framebuffer + X11 window via dlopen
- **Output:** PPM (Portable Pixmap), deterministic byte-identical results
- **Game entry:** `port_main.c` → `func_8001220C_port.c` → full PE boot chain
- **Memory strategy:** Contiguous 2 MiB guest RAM (`pe_guest_ram.[ch]`).
  All guest addresses are `pe_addr_t` (`uint32_t`); translation to host
  pointers happens only at real access sites via bounds-checked
  `PE_Translate` / `PE_LoadU8/16/32` / `PE_StoreU8/16/32`.  Named globals
  that live in guest RAM (the `0x800B0CD8` block, `D_80094488/8C`,
  `D_800BCE80`) are typed lvalue macros over `PE_Translate` — one store,
  no split-brain.  `PE_RamInit` allocates + zero-fills; `PE_RamReset`
  re-zeroes; `PE_RamDestroy` frees.
- **Callback registry:** `pe_callback.[ch]` — full-width function pointers,
  no `(int)(uintptr_t)` narrowing.  Reset / Register / Get / Invoke.
- **Bootstrap policy:** `pe_bootstrap.[ch]` — every `BOOTSTRAP_RET`
  provider goes through `Bootstrap_ReturnInt/Void`.  Strict mode
  (`--strict-stubs`) aborts centrally at the first invoked provider.
  Deterministic provider sequences (`Bootstrap_SetIntSequence`) script
  per-symbol return values for wait-loop testing.
- **Disc strategy:** Bootstrap-disc mode (real-disc deferred to Phase 6E)

## Translated Boot Rung functions (Phase 6D/6D-R/6D-S)

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

# Sanitizer build (ASan + UBSan)
cmake .. -DPE_PORT_SANITIZERS=ON
make
```

Produces:
- `parasite-eve-port` — native executable
- `pe-native-tests` — test suite (81 tests, all pass)

## Running

```bash
# Headless deterministic (CI/validation)
./parasite-eve-port --headless --bootstrap-disc \
  --stop-after-event first-clear \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Windowed (requires X11 display)
DISPLAY=:10.0 ./parasite-eve-port --bootstrap-disc \
  --stop-after-event first-clear --hold-ms 5000 --debug-overlay

# Strict mode — centralized abort at first unresolved provider
./parasite-eve-port --headless --strict-stubs \
  --stop-after-event first-clear
# Expected: exit 1, names func_800725DC (first provider on the boot path)
```

## Testing

```bash
cd pc_port/build
./pe-native-tests   # 81 tests (12 baseline + 69 guest-RAM/Boot Rung/policy)
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
- Matching decomp remains separate and SHA-exact (227 C leaves, SHA `452fb033`)
- No PS1 emulator in the native executable
- This is NOT a playable game yet
