# Parasite Eve Native PC Port — Phase 6E-B3

**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**Current milestone:** func_8003E974 initialization rung — the subsystem
state-clearing wrapper is translated retail logic (5 `$gp`-relative global
clears, a 32-word clear of `D_800A76F0`, then the complete ROM-ordered series
of 20 `func_8003EAC8(mask,value)` registration calls, ending with
`D_8009D1A0 |= 0x4000`).  `func_8003EAC8` itself is audited (a GTE
LZCS/LZCR highest-set-bit-index table writer, 63 call sites exe-wide) but
remains the unresolved strict-mode frontier.

**Status:** FUNC_8003E974 INITIALIZATION RUNG VERIFIED — 159 native tests
pass; ASan/UBSan clean; oracle dump byte-identical to
`tools/rng_oracle.py` on the retail exe; 3 deterministic headless runs
(framebuffer `fb28dc21…`); 3 real-disc load traces byte-identical;
strict mode with `--disc-image` stops at `func_8003EAC8` (from
`func_8003E974`); windowed SHA matches headless;
matching build remains exact at SHA `452fb033`.

**Previous milestones:** 6E-B2 (func_80070D6C RNG advance rung — the game's
lagged-Fibonacci RNG, init/advance/ranged wrapper, byte-exact against an
independent oracle on the retail executable, which the port loads into
guest RAM via SYSTEM.CNF `BOOT=` because the RNG read cursor provably
cycles through 14 retail code words below its table);
6E-B1 (func_80070D10 RNG-table init rung);
6E-A batch 3 (real Disc 1 byte path into guest RAM at `D_80011614`).

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
- **Disc layer:** `pe_disc.[ch]` — read-only host access to the
  user-supplied Disc 1 image (never embedded, copied, staged, or
  committed).  Single-track BIN/CUE MODE2/2352: 2048 user bytes at raw
  sector offset +24, ISO9660 PVD at user sector 16.  All reads
  bounds-checked; missing/truncated/malformed/wrong-disc inputs rejected
  safely.  `pe_libcd.c` carries the real disc providers:
  `func_80082314` (PVD verify, result word `D_800B28F8`),
  `func_80081414` (DsSearchFile, 24-byte CdlFILE to a guest address),
  `func_80080C48` (CdPosToInt), `func_8006E6D4` (read issue — synchronous
  bounded copy into guest RAM), `func_800811E4` (poll: 0 done / -1
  timeout).  `func_800698D4` runs the verbatim retail mount sequence;
  `--bootstrap-disc` remains as an explicit test fixture only.
- **Guest exe image:** `pe_guest_image.[ch]` — with `--disc-image`, the
  retail boot executable named by SYSTEM.CNF `BOOT=` is validated
  (PS-X EXE header, geometry, guest-RAM bounds) and loaded into guest
  RAM at its `taddr`.  Retail code reads its own text as data — the
  func_80070D6C RNG read cursor cycles through 14 code words below its
  table (`tools/rng_oracle.py`, proven against the retail exe) — so
  faithful RNG output needs those bytes.  Without an image the words
  read as zero: a documented fixture-mode divergence that nothing on
  the boot-to-black path consumes.

## Translated RNG (Phase 6E-B1/B2)

| Function | Size | Purpose |
|----------|------|---------|
| `func_80070D10` | 0x5C | Lagged-Fibonacci table init (17 words, 2 index words) |
| `func_80070D6C` | 0x64 | RNG advance; verbatim `i2 \|= 0x40` wrap (cycles, never clamps) |
| `func_80070DD0` | 0x34 | Handwritten ranged random: `a + (rand16·(b−a) >> 16)` |

Correctness gate: `--rng-oracle-dump` output is byte-identical to
`pc_port/tools/rng_oracle.py` running independently on the retail exe
(SHA-1 `452fb033…`).

## Translated subsystem-init rung (Phase 6E-B3)

| Function | Size | Words | Purpose |
|----------|------|-------|---------|
| `func_8003E974` | 0x154 | 85 | Subsystem state clear + 20 `func_8003EAC8` registration calls |

`func_8003E974` (retail `$gp` = `0x8009CD70`) clears 5 `$gp`-relative
globals, clears the 32-word array at `D_800A76F0`, then issues the complete
ROM-ordered `func_8003EAC8(mask,value)` series
(`(1,0x4000) (0x80,0x1000) (0x100,0x2000) (0x8,0x10) (0x20,0x40)
(0x40,0x80) (0x10,0x20) (0x2,1) (0x4,8) (0x200,0x2000) (0x400,0x4000)
(0x2000,0x8000) (0x1000000,0x100) (0x2000000,0x200) (0x4000000,0x400)
(0x8000000,0x800) (0x10000000,0x1000) (0x20000000,0x2000)
(0x40000000,0x4000) (0x80000000,0x8000)`), ending with
`D_8009D1A0 |= 0x4000`.  Idempotent.  `func_8003EAC8` is a handwritten GTE
LZCS/LZCR leaf (highest-set-bit index, `0x80000000` → slot 31) that writes
`D_800A76F0[idx] = a1`; it has 63 call sites exe-wide and remains the
unresolved strict-mode frontier — production calls route through
`Bootstrap_ReturnVoid`, with bounded test-only argument recording
(`PE_3EAC8_RecordReset/Count/At`).

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

# Sanitizer build (ASan + UBSan, static runtimes)
cmake .. -DPE_PORT_SANITIZERS=ON
make
```

Produces:
- `parasite-eve-port` — native executable
- `pe-native-tests` — test suite (159 tests, all pass)

## Running

```bash
# Headless deterministic (CI/validation; bootstrap-disc fixture)
./parasite-eve-port --headless --bootstrap-disc \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Real Disc 1 boot (image opened read-only, never copied)
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Real-disc byte-path verification driver: PVD verify → DsSearchFile
# "\PE.IMG;1" → CdPosToInt → bounded guest-RAM load at D_80011614 → poll,
# tracing every value (deterministic for a given image)
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" \
  --disc-load-test --trace /tmp/pe-disc.trace
# Expected trace: pvd_verify=4, PE.IMG lba=1013 size=206213120,
# issue=1 dest=0x8010BD00 len=32768, poll=0, fnv1a64=7D860391E1ED6C97

# Windowed (requires X11 display)
DISPLAY=:10.0 ./parasite-eve-port --bootstrap-disc --hold-ms 5000 --debug-overlay

# Strict mode — centralized abort at first unresolved provider
./parasite-eve-port --headless --strict-stubs --disc-image "/path/disc1.bin"
# Expected: exit 1, names func_8003EAC8 (from func_8003E974) — the first
# unresolved provider past the translated RNG (70D10/70D6C/70DD0) and the
# translated subsystem-init wrapper (func_8003E974)

# RNG oracle gate — must equal tools/rng_oracle.py on the retail exe
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" --rng-oracle-dump
```

`--bootstrap-disc` and `--disc-image` are mutually exclusive.  Without
either, the retail disc-wait is modeled honestly (bounded by a documented
host adaptation); the boot is not faked.

## Testing

```bash
cd pc_port/build
./pe-native-tests   # 159 tests (baseline + guest-RAM/Boot Rung/policy
                    # + real-disc: pe_disc fixtures, disc providers,
                    # guest-copy bounds, func_800698D4 sequences
                    # + 6E-B1: func_80070D10 RNG-init rung
                    # + 6E-B2: func_80070D6C/70DD0 RNG model equality,
                    #   wrap quirk, footprints, warm-up integration
                    # + 6E-B3: func_8003E974 write map, func_8003EAC8
                    #   call order/args, idempotence, footprints)
```

## Deterministic framebuffer

| Property | Value |
|----------|-------|
| SHA-256 (headless, 3 runs) | `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` |
| Windowed SHA matches headless | ✅ |
| Trace SHA-256 (3 runs) | `42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af` |
| Real-disc load trace (3 runs) | `7b8724acf4d4787f58ca0068e68839f171e2d3f36f72a42f0a4ef03f0041672b` |
| Unsupported traps | 0 |

## Next steps

1. **Phase 6E-B continued:** `func_8003EAC8` rung — host LZCR
   (highest-set-bit-index) implementation of the GTE LZCS/LZCR
   registration provider, then the provider frontier toward the
   image-load consumers
2. Identify the first boot asset (likely MDEC logo data)
3. Wire MDEC decoding and display
4. Audio, input, save/load (later phases)

## Constraints

- PCSX-Redux is the retail oracle only — never used as runtime
- Matching decomp remains separate and SHA-exact (227 C leaves, SHA `452fb033`)
- No PS1 emulator in the native executable
- The Disc 1 image is user-supplied and read-only; never commit it or
  any derivative captures
- This is NOT a playable game yet
