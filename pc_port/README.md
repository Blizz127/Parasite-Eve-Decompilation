# Parasite Eve Native PC Port — Phase 6E-B6

**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**Current milestone:** func_80073D24 VBlank callback registration rung —
the libetc jump-table wrapper (13 words, forces slot 4 in the jal delay
slot, forwards the callee return) now executes its instruction-derived
contract: `prev = D_8009568C[4]; if (h != prev) D_8009568C[4] = h;
return prev;`.  That is the semantics of func_80074478, the handler
installed at jump-table field 0x14 by ResetCallback (func_80073E28's
delay-slot store of func_800743B4's return value — verified against the
retail exe bytes).  All retail-visible state lives in guest RAM: the
8-word handler table at `0x8009568C` and the dispatch counter at
`0x800956AC`, driven by a func_8007440C-faithful dispatcher (counter++,
non-null slots 0..7 in order, no args).  The host side keeps only a typed
guest-address → host-function binding map at full pointer width; unknown
guest identities are visible errors, never silent skips.  Verified by an
independent MIPS-interpreter oracle (`tools/callback_oracle.py`)
executing the verified retail words of func_80074478/func_8007440C —
the port's `--callback-oracle-dump` is byte-identical.

**Status:** FUNC_80073D24 CALLBACK RUNG VERIFIED — 184 native tests pass;
ASan/UBSan clean; RNG, LZCR and callback oracle dumps byte-identical to
their independent interpreters on the retail exe; 3 deterministic
headless runs (framebuffer `fb28dc21…`); 3 real-disc load traces
byte-identical; strict mode with `--disc-image` now stops at
`func_800371A4` (from `func_8003E680`) — the next provider past the
real callback slot writes; windowed SHA matches headless; matching build
remains exact at SHA `452fb033`.

**Previous milestones:** 6E-B5 (func_80036DC8 timer-record init rung —
three 12-byte records at `0x800A76A0/AC/B8`, all 11 word stores in ROM
order);
6E-B4 (func_8003EAC8 LZCR registration rung —
GTE LZCS/LZCR leaf, exact edge semantics incl. the preserved below-table
write at `0x800A76EC`, oracle-verified; call-site correction: 20 distinct
sites, not 63);
6E-B3 (func_8003E974 initialization rung —
subsystem state clear + the 20-call ROM-ordered registration series);
6E-B2 (func_80070D6C RNG advance rung — the game's
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
- **Callback slot model:** `pe_callback.[ch]` — guest-backed retail
  callback queue (Phase 6E-B6): 8 handler slots at guest `0x8009568C` +
  dispatch counter at `0x800956AC`, with func_80074478-faithful
  set-slot (previous-handler return, no store on identical handler) and
  func_8007440C-faithful dispatch.  Host side: typed guest-address →
  host-function bindings, full pointer width, never guest-visible.
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

## Translated subsystem-init rungs (Phase 6E-B3/B4)

| Function | Size | Words | Purpose |
|----------|------|-------|---------|
| `func_8003E974` | 0x154 | 85 | Subsystem state clear + 20 `func_8003EAC8` registration calls |
| `func_8003EAC8` | 0x3C | 15 | GTE LZCS/LZCR-indexed registration leaf |

`func_8003E974` (retail `$gp` = `0x8009CD70`) clears 5 `$gp`-relative
globals, clears the 32-word array at `D_800A76F0`, then issues the complete
ROM-ordered `func_8003EAC8(mask,value)` series
(`(1,0x4000) (0x80,0x1000) (0x100,0x2000) (0x8,0x10) (0x20,0x40)
(0x40,0x80) (0x10,0x20) (0x2,1) (0x4,8) (0x200,0x2000) (0x400,0x4000)
(0x2000,0x8000) (0x1000000,0x100) (0x2000000,0x200) (0x4000000,0x400)
(0x8000000,0x800) (0x10000000,0x1000) (0x20000000,0x2000)
(0x40000000,0x4000) (0x80000000,0x8000)`), ending with
`D_8009D1A0 |= 0x4000`.  Idempotent.  Result: 20 slots populated
(0-10, 13, 24-31), no duplicates; slots 11/12/14-23 stay 0 from the clear.

`func_8003EAC8` is a handwritten GTE LZCS/LZCR leaf:
`idx = (a0 == 0x80000000) ? 31 : 31 - LZCR(a0)`, then a word store
`D_800A76F0[idx] = a1` via the `sll`/`addu` chain.  LZCR counts leading
sign bits, so `idx ∈ [-1, 31]`: zero/all-ones inputs write one word below
the table at `0x800A76EC` (valid guest RAM — preserved, never clamped),
positives map to the highest SET bit, other negatives to the highest ZERO
bit.  20 distinct call sites, all in `func_8003E974`, all constant
arguments (the earlier "63" counted overlapping split files).  Bounded
test-only argument recording (`PE_3EAC8_RecordReset/SetEnabled/Count/At`)
is non-semantic: production stores identically with the recorder off.
Correctness gate: `--lzcr-oracle-dump` is byte-identical to
`tools/lzcr_oracle.py`, a tiny MIPS interpreter executing the verified
retail words.

## Translated timer-record init rung (Phase 6E-B5)

| Function | Size | Words | Purpose |
|----------|------|-------|---------|
| `func_80036DC8` | 0x28 | 10 | Timer-record init dispatcher (3 calls, retail order) |
| `func_80036DF8` | 0x3C | 15 | Record 0 init (5 stores incl. 2 dead zero-stores) |
| `func_80036E34` | 0x24 | 9 | Record 2 init |
| `func_80036E58` | 0x24 | 9 | Record 1 init |

Net state: three 12-byte records at `0x800A76A0/AC/B8`, each `{ 1, 0, x }`
with record 0 field2 preloaded to `0x1499700` (21,600,000).  Consumers
(`func_80019DB8`, `func_80052894/528C4`, and the packed-field writer
`func_80036E7C`) index field1 via base+i*12 and divide by 60 — consistent
with 60 Hz tick counters.  Sole caller of the dispatcher is
`func_8003E680`; the leaves are called only from it.  Idempotent;
verbatim ROM-order stores, all guest-RAM-backed.

## VBlank callback slot rung (Phase 6E-B6)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80073D24` | 0x34 | 13 | libetc jump-table wrapper: field 0x14, slot forced to 4, return forwarded |
| `func_80074478` | 0x2C | 11 | installed setter: `prev = tab[slot]; if (h != prev) tab[slot] = h; return prev` |
| `func_8007440C` | 0x6C | 26 | dispatcher: counter++, invoke non-null slots 0..7 in order |

Retail state is guest-RAM resident: 8 handler slots at `0x8009568C`
(D_8009568C, zero-initialized image bytes) and the dispatch counter at
`0x800956AC` (D_800956AC).  Exact retail address arithmetic is preserved
— slot 8 aliases the counter.  The host binding map (guest address →
host function, full width) is plumbing only; dispatch of an unbound
guest address is a visible error.  Both func_8003E680 call sites
(`0x8003E6E0` clear, `0x8003E6F0` install of `0x8003E91C`) discard the
return, matching retail.  ResetCallback's first guard-passing call
zeroes all 8 slots + counter (func_800743B4 semantics).  Correctness
gate: `--callback-oracle-dump` is byte-identical to
`tools/callback_oracle.py`, a MIPS interpreter executing the verified
retail words.

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
- `pe-native-tests` — test suite (184 tests, all pass)

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
# Expected: exit 1, names func_800371A4 (from func_8003E680) — the first
# unresolved provider past the translated RNG (70D10/70D6C/70DD0), the
# subsystem-init pair (func_8003E974 + func_8003EAC8), the timer-record
# init (func_80036DC8 + leaves), and the real func_80073D24 callback
# slot writes

# RNG oracle gate — must equal tools/rng_oracle.py on the retail exe
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" --rng-oracle-dump

# LZCR oracle gate — must equal tools/lzcr_oracle.py on the retail exe
./parasite-eve-port --headless --lzcr-oracle-dump

# Callback oracle gate — must equal tools/callback_oracle.py on the
# retail exe
./parasite-eve-port --headless --callback-oracle-dump
```

`--bootstrap-disc` and `--disc-image` are mutually exclusive.  Without
either, the retail disc-wait is modeled honestly (bounded by a documented
host adaptation); the boot is not faked.

## Testing

```bash
cd pc_port/build
./pe-native-tests   # 184 tests (baseline + guest-RAM/Boot Rung/policy
                    # + real-disc: pe_disc fixtures, disc providers,
                    # guest-copy bounds, func_800698D4 sequences
                    # + 6E-B1: func_80070D10 RNG-init rung
                    # + 6E-B2: func_80070D6C/70DD0 RNG model equality,
                    #   wrap quirk, footprints, warm-up integration
                    # + 6E-B3: func_8003E974 write map, func_8003EAC8
                    #   call order/args, idempotence, footprints
                    # + 6E-B4: LZCR helper exactness, func_8003EAC8
                    #   edge/one-hot contract, below-table write, recorder
                    #   semantics, func_8003E974 final registered table
                    # + 6E-B5: func_80036DC8 final record state, per-leaf
                    #   write sets, footprint, idempotence, integration
                    # + 6E-B6: func_80073D24 slot contract, previous-
                    #   handler returns, oracle equality, dispatch order,
                    #   unknown identities, footprint, strict integration)
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

1. **Phase 6E-B continued:** `func_800371A4` rung (next strict-mode
   frontier, from `func_8003E680`), then the provider frontier toward
   the image-load consumers
2. Identify the first boot asset (likely MDEC logo data)
3. Wire MDEC decoding and display
4. Audio, input, save/load (later phases)

## Constraints

- PCSX-Redux is the retail oracle only — never used as runtime
- Matching decomp remains separate and SHA-exact (SHA `452fb033`; 229 C
  leaves in the current matching checkout)
- No PS1 emulator in the native executable
- The Disc 1 image is user-supplied and read-only; never commit it or
  any derivative captures
- This is NOT a playable game yet
