# Parasite Eve Native PC Port — Phase 6E-B22

**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**Current milestone:** B22 func_8005DE88 rung — resource-list & state
initializer (23 words at `0x8005DE88..0x8005DEE3`, file `0x4E688`, live
split `4CC98.s`).  It links the 20 twelve-byte records beginning at
`D_800A2090`, null-terminates `D_800A2174`, and initializes the six
`$gp`-relative state words `D_8009D0DC..D_8009D0F0` exactly.  No callees;
returns void.  Independent oracle: `tools/b22_5de88_oracle.py`.

Previous milestone: func_8006A9E4 rung — PE.IMG streaming resource
load, translated as retail logic with two unresolved callees routed
through the centralized bootstrap boundary (classification 1).  Complete
body is 215 retail words / `0x35C` verified against the SHA-1-exact
executable (VRAM `0x8006A9E4`–`0x8006AD3F`, file `0x5B1E4`; live split
`asm/disc1/5B1E4.s`).  ROM order: `ClearImage({0,0,0x3FF,0x1FF},0,0,1)`
through the REAL host SDK (func_80074F44), four streaming
sector-read/poll cycles from PE.IMG (tables at `D_800930DC..E8`,
destinations `D_800A8028` and the `lw(D_800B0E6C)` stream buffer;
A/B polls restart the whole cycle on a -1, C/D polls are sltu-clamped so
the restart branch is dead retail code), a `0x10A50`-byte copy to
`D_800E2858`, two func_8006E498 archive lookups (keys `0x57D40D84` /
`0x57D41D84`, exact delay-slot store order `D_800B0E20`→`E18`→`E1C`),
the unresolved func_80087090 SPU upload, and a `0x1400`-byte copy to
`lw(D_800B0E08)`.  Sole exe call site `func_8001220C` @`0x80012284`
(nop delay slot, immediately after the func_8003E680 jal @`0x8001227C`);
return ignored → `void(void)`.  Three dependencies translated with it:
func_8006E6A8 (11-word issue wrapper; sector→byte unit conversion at
the host-adaptation boundary, proven by the 34-sector/67792-byte and
3-sector/5120-byte cycle/copy pairs), func_8006E7E8 (19-word completion
poll + `D_800B0CD8 &= 0xFEFFBFFF` RMW on st∈{-1,0}), and func_8006E498
(31-word pure guest table walk, guest-address result).  The rung also
fixed a 6D-S-class split-brain defect: `D_800B0E24..D_800B0E6C` are now
guest-RAM lvalue macros (retail readers load guest RAM) instead of
duplicate host globals.  **Phase 6E-B17 advances the strict frontier to
`func_800528F0`**: func_800527C8 (the multi-subsystem bootstrap dispatcher,
49 words, 17 calls) is now fully translated with 7 leaf implementations
and 10 unresolved callees routed through the centralized bootstrap
boundary in retail ROM order.  Strict mode with `--disc-image` stops at
`func_800528F0` (first unresolved callee INSIDE the translated dispatcher);
`--bootstrap-disc` still stops at `func_8007F72C` by design.**

**Status:** B23 func_80052C6C RUNG — 271 native tests pass (and 271/271
sanitized); strict real-disc frontier advanced to `func_8005BCBC` from
`func_800527C8` (exit 1), bootstrap-disc still stops at `func_8007F72C`
by design.  Independent oracle: `tools/b23_oracle.py` (delay-slot MIPS
interpreter on the SHA-1-verified retail words; asserts the full
func_80052C6C footprint in retail ROM order).  Dispatcher oracle now
reports 4 unresolved callees in retail order: `func_8005BCBC`,
`func_8005D6F4`, `func_80051CC4`, `func_80042C78`.  Matching build exact at SHA
`452fb033` (227 C leaves).

Previous status: FUNC_800527C8 RUNG VERIFIED — 251 native tests pass;
ASan/UBSan clean (tests + headless + strict runs);
RNG, LZCR, callback, and dispatcher oracle dumps byte-identical to their
independent interpreters on the retail exe; 3 deterministic test runs
identical (251/251); windowed framebuffer matches headless; matching
build remains exact at SHA `452fb033` (227 C leaves).

**Previous milestones:** 6E-B15 (func_80038D1C byte test-and-clear —
11 retail words on `D_80091A20`, both return paths; with it
func_8003E680 became fully translated);
6E-B14 (func_8006536C record-table clear +
index byte clear — 19 retail words: 28×3-word table at `D_800A3180`
span `..0x800A32CF`, index byte `0x8009CDB4`);
6E-B13 (func_80034F10 subsystem table clear +
flag-bit clear — 45 retail words: 512-word array at `D_800A77F0`,
14×160-word matrix at `D_800BEA90` span `..0x800C0D8F`, six scalars,
`D_800B0CD8 &= ~0x3000`);
6E-B12 (func_8001A890 subsystem scalar/array
clear — 34 retail words: `0x8009CE08`–`0x8009CE17`, six stride-4
halfwords, four words, two halfwords, 20-word array at `D_8009DFB0`);
6E-B11 (func_800124F8 subsystem table
clear — 31 retail words: five scalars, 72×11-word matrix at
`D_8009D310`, 16-word array at `D_8009DF70`);
6E-B10 (func_80068D28 display-record data
init — 63 retail words, scalar block + 320×224 records + GP0-shaped
`0xE1000440` data words at `D_800BCF88`);
6E-B9 (func_8005BCA8 empty `jr $ra; nop`
stub rung — 2 retail words, zero footprint);
6E-B8 (func_80029388 slot-table clear +
default-record init — leaves func_8002F658 / func_80020EFC; 7×220
SlotRecord table at D_800A5D58);
6E-B7 (func_800371A4 `$gp`-relative byte
setter rung — 3-word `sb $a0, 0x124($gp)` → `D_8009CE94`);
6E-B6 (func_80073D24 VBlank callback rung —
libetc slot-4 wrapper with func_80074478 previous-handler semantics and
func_8007440C dispatch, guest-backed at `0x8009568C`/`0x800956AC`,
oracle-verified);
6E-B5 (func_80036DC8 timer-record init rung —
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

## $gp-relative byte setter rung (Phase 6E-B7)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_800371A4` | 0xC | 3 | `sb $a0, 0x124($gp)` — `(uint8_t)a0` → `D_8009CE94` (guest `0x8009CE94`) |

Complete body verified word-for-word against the retail executable
(`0xA3840124 / 0x03E00008 / 0x00000000`).  Two call sites
(`func_8003E680` arg 0, `func_800527C8` arg 1), both return-discarding;
sole reader `func_80037870` (`lbu` + compare) is off the boot path.
Matches the decomp's matching C leaf statement-for-statement.  Single
byte of write footprint, proven by a full 2 MiB canary scan.

## Slot-table clear + default-record init rung (Phase 6E-B8)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80029388` | 0x6C | 27 | Clear 7 `D_800A5D58` slot in-use words (stride 220) + `D_8009D2A0`/`D_8009D2EC` bytes; calls the two leaves below |
| `func_8002F658` | 0x114 | 69 | Copy exe-rodata default records `D_80010928`→`D_800B8A20` (0x70) and `D_80010998`→`D_800B0CB0` (0x18); zero `D_8009D1B0`/`D_8009D1B4` |
| `func_80020EFC` | matched leaf | — | Five `$gp`-relative byte clears (`D_8009CE3C`/`D_8009D1D4`/`D_8009D1DC`/`D_8009D2D8`/`D_8009D1F0`) |

All 27 `func_80029388` words and all 69 `func_8002F658` words verified
against the retail executable (taddr `0x80010000`); `func_80020EFC` is
the decomp's Phase-5DE matching C leaf.  Exactly one exe-wide call site
for `func_80029388` (`func_8003E680` @0x8003E700, nop delay slot — the
`addu $a0,$zero,$zero` at 0x8003E6FC belongs to the preceding
`func_800371A4` call), one for `func_8002F658` (from `func_80029388`),
two for `func_80020EFC` (`func_80029388`, and `func_80029810` off the
boot path).  The slot table is the same 7×220 `SlotRecord` table the
decomp's matched `func_8002F9CC` leaf clears — retail has two clearing
functions; both are reproduced.  Write footprint proven by full 2 MiB
canary scans; copy content verified word-for-word against deterministic
source patterns.

## Empty stub rung (Phase 6E-B9)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8005BCA8` | 0x8 | 2 | Empty `jr $ra`/`nop` — no guest-memory effect |

Both words verified against the retail executable.  Sole call site
`func_8003E680`; return value unconsumed.  Zero write footprint proven
by a full 2 MiB canary scan; dirty state and `PE_RamReset` state pass
through unchanged.  Translating it advances strict mode past the call.

## Display-record data init rung (Phase 6E-B10)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80068D28` | 0xFC | 63 | Double-buffered display-record data init at `D_800BCF88` |

All 63 words verified against the retail executable (exe
`0x80068D28`–`0x80068E20`, live split `55430.s`).  Sole call site
`func_8003E680` @0x8003E710 (nop delay slot), `void(void)`, `$v0`=0
unconsumed.  Writes a scalar block at base+0x60..0x70 (halfwords
0xFF/0, bytes 1/2, post-loop halfwords 0), then two 16-byte records at
base+i*0x10 (bytes 3/0xFF×3/0x60→0x62, halfwords 0/0/0x140/0xE0) and
two 8-byte records at base+i*0x8 (byte 1, word `0xE1000440` — GP0-shaped
DATA in a guest struct, not a hardware write).  Loop byte values are
retail load-after-store from the just-written scalars (reproduced via
`PE_Load`).  Write extent `0x800BCFBB`–`0x800BCFF9`; idempotent,
including after `PE_RamReset`.  No SDK/GTE/hardware/callback use; no
oracle needed — the straight-line fixed-value init is fully proven by
the 63-word executable verification.  Strict mode advances to
`func_800124F8`.

## Subsystem table clear rung (Phase 6E-B11)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_800124F8` | 0x7C | 31 | Zero-fill 72×11-word matrix at `D_8009D310` (stride `0x2C`) + 16-word array at `D_8009DF70` + five scalars |

All 31 words verified against the retail executable (exe
`0x800124F8`–`0x80012570`, file `0x2CF8`; live split `2A0C.s`).
Sole call site `func_8003E680` @`0x8003E718` (nop delay slot),
`void(void)`, `$v0`=0 unconsumed.  Pure zero-stores in ROM order:
`0x8009D300` (word), `0x8009D308` (halfword — `0x8009D304`
untouched), `0x8009CDFC`, then the row-major matrix
(`0x8009D310`–`0x8009DF6F`), `0x8009CE00`, the array
(`0x8009DF70`–`0x8009DFAF`, contiguous with the table end), and
`0x8009CE04`.  No reads, no SDK/GTE/hardware/GPU work; idempotent,
including after `PE_RamReset`.  Write footprint proven by a full 2 MiB
canary scan; strict mode advances to `func_8001A890`.

## Subsystem scalar/array clear rung (Phase 6E-B12)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8001A890` | 0x88 | 34 | Zero-fill `0x8009CE08`–`0x8009CE17` + six stride-4 halfwords + four scattered words + two halfwords + 20-word array at `D_8009DFB0` |

All 34 words verified against the retail executable (exe
`0x8001A890`–`0x8001A914`, file `0xB090`; live split `A404.s`).
Sole call site `func_8003E680` @`0x8003E720` (nop delay slot),
`void(void)`, `$v0`=0 unconsumed.  Pure zero-stores in ROM order:
word `0x8009CE08`; stride-2 halfword loop `0x8009CE0C`–`0x8009CE13`;
word `0x8009CE14`; 20-word array `0x8009DFB0`–`0x8009DFFC`
(contiguous above 124F8's array); six stride-4 halfwords
`0x8009CE18`–`0x8009CE2C` (interleaved upper halfwords untouched);
words `0x8009D1D8`/`D1FC`/`D2F8`/`D248`; halfwords `0x8009D264`/
`D1CC`.  No reads, no SDK/GTE/hardware/GPU work; idempotent,
including after `PE_RamReset`.  Write footprint proven by a full 2 MiB
canary scan; strict mode advances to `func_80034F10`.

## Subsystem table clear + flag-bit clear rung (Phase 6E-B13)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80034F10` | 0xB4 | 45 | Zero-fill 512-word array at `D_800A77F0` + 14×160-word matrix at `D_800BEA90` (stride `0x280`) + six scalars + `D_800B0CD8 &= ~0x3000` |

All 45 words verified against the retail executable (exe
`0x80034F10`–`0x80034FC0`, file `0x25710`; live split `2422C.s`).
Sole call site `func_8003E680` @`0x8003E728` (nop delay slot),
`void(void)`, `$v0`=&`D_800B0CD8` unconsumed.  ROM order: word
`0x8009D2E8`; 512-word array `0x800A77F0`–`0x800A7FEC`; `D_800B6A80`
(retail stores the same word 64× via a delay-slot loop with no pointer
advance — documented, reproduced as one store with identical end
state); 14×160-word matrix `0x800BEA90`–`0x800C0D8F` (row stride
`0x280`); scalars `0x8009D2AC`/`0x8009D20C`/`0x8009D2F0`/halfword
`0x8009D2A6`/`0x8009D254`/`0x8009D224` (ROM order 53C, 49C, 580, 536,
4E4, 4B4); final RMW `D_800B0CD8 &= ~0x3000` (sole guest read; store
in the `jr $ra` delay slot).  No SDK/GTE/hardware/GPU work;
idempotent, including after `PE_RamReset`.  Write footprint proven by
a full 2 MiB canary scan; strict mode advances to `func_8006536C`.

## Record-table clear + index byte clear rung (Phase 6E-B14)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8006536C` | 0x4C | 19 | Zero-fill 28×3-word record table at `D_800A3180` (stride `0xC`, contiguous 84 words) + index byte `0x8009CDB4` |

All 19 words verified against the retail executable (exe
`0x8006536C`–`0x800653B4`, file `0x55B6C`; live split `55430.s`).
Sole call site `func_8003E680` @`0x8003E730` (nop delay slot),
`void(void)`, `$v0`=0 unconsumed.  ROM order: the nested table clear
(`0x800A3180`–`0x800A32CF`, row-major) then `sb` 0 → `0x44($gp)` =
`0x8009CDB4` — the current-record index (func_800653B8 below reads
`lbu 0x44($gp)` and indexes `D_800A3180` + byte×12, confirming the
28-record×12-byte structure).  No reads, no SDK/GTE/hardware/GPU
work; idempotent, including after `PE_RamReset`.  Write footprint
proven by a full 2 MiB canary scan; strict mode advances to
`func_80038D1C`.

## Byte test-and-clear status leaf rung (Phase 6E-B15)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80038D1C` | 0x2C | 11 | `D_80091A20` test-and-clear: return 0 if flag was set (and clear it), else return `0xFF` |

All 11 words verified against the retail executable (exe
`0x80038D1C`–`0x80038D44`, file `0x2951C`; live split `2951C.s`);
also a matched C leaf in the matching decomp (`src/func_80038D1C.c`).
Exactly two exe call sites, both ignoring the return:
`func_8003E680` @`0x8003E738` (final call; void epilogue follows) and
`func_8006E9A0` @`0x8006EB7C`.  One byte read, one conditional byte
write (only when the flag was nonzero); no SDK/GTE/hardware/GPU work.
Repeated calls are state-stable but not return-idempotent (the first
call consumes the flag); after `PE_RamReset` the flag is 0 → return
`0xFF`, no write.  Write footprint proven by a full 2 MiB canary scan
on BOTH paths (write and no-write).  **Milestone: func_8003E680 is
now fully translated** — strict mode advances past it to
`func_8006A9E4` (from `func_8001220C`).

## PE.IMG streaming resource load rung (Phase 6E-B16)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8006A9E4` | 0x35C | 215 | ClearImage + four PE.IMG sector-read/poll cycles + two archive copies/lookups + translated func_800527C8 dispatcher + unresolved func_80087090 boundary call |
| `func_8006E6A8` | 0x2C | 11 | Issue wrapper: `func_8006E6D4(lba, 0, dest, sectors << 11)` — sector→byte conversion at the host-adaptation boundary |
| `func_8006E7E8` | 0x4C | 19 | Completion poll: `func_800811E4` + `D_800B0CD8 &= 0xFEFFBFFF` when st∈{-1,0} |
| `func_8006E498` | 0x7C | 31 | Archive directory lookup by 32-bit key; pure guest table walk; guest-address result |

All 276 words verified against the retail executable (func_8006A9E4:
exe `0x8006A9E4`–`0x8006AD3F`, file `0x5B1E4`, live split `5B1E4.s`).
Sole func_8006A9E4 call site `func_8001220C` @`0x80012284` (nop delay
slot, return ignored).  The four cycles read `end-off` SECTORS at
`D_800B0DD8 + off` (proven: cycle B reads 34 sectors = 69632 bytes
followed by a 67792-byte copy; cycle D reads 3 sectors = 6144 bytes
followed by a 5120-byte copy).  Polls A/B restart the whole cycle on a
-1; polls C/D clamp the status with sltu so a -1 re-polls without
re-issuing.  func_800527C8 is now fully translated (Phase 6E-B17):
invoked exactly once inside the cycle-B poll loop via the retail $s1
one-shot guard; its 10 unresolved callees appear in retail ROM order
before the final func_80087090 call.  func_80087090 is still unresolved
and goes through the centralized bootstrap boundary — strict mode now
stops at `func_80052C6C` (the first unresolved callee inside the
translated dispatcher).  The retry/restart loop bodies beyond first-pass
completion are not externally triggerable in the synchronous host
model (issue always refreshes the poll timestamp before the poll), so
their -1 semantics are covered at the provider level instead.  This
rung also converted `D_800B0E24..D_800B0E6C` from duplicate host
globals to guest-RAM lvalue macros after the strict run proved retail
readers load them from guest RAM (6D-S split-brain defect class).
Write footprint proven by a full 2 MiB canary scan; patterned-fixture
test verifies every copied byte, both lookup results, the delay-slot
store order, and the provider invocation order.

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
- `pe-native-tests` — test suite (251 tests, all pass)

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
# Expected: exit 1 at func_80052C6C from func_800527C8 — the first
# unresolved provider past the fully translated func_8003E680, the
# translated func_8006A9E4 streaming-load rung, and the fully
# translated func_800527C8 dispatcher: the
# translated RNG (70D10/70D6C/70DD0), the subsystem-init pair
# (func_8003E974 + func_8003EAC8), the timer-record init
# (func_80036DC8 + leaves), the real func_80073D24 callback
# slot writes, the real func_800371A4 byte store, the real
# func_80029388 slot-table clear + record init, the real empty
# func_8005BCA8 stub, the real func_80068D28 display-record init,
# the real func_800124F8 subsystem table clear, the real
# func_8001A890 scalar/array clear, the real func_80034F10
# subsystem table clear + flag-bit clear, the real func_8006536C
# record-table clear + index byte clear, the real func_80038D1C
# test-and-clear leaf, and the real func_8006A9E4 PE.IMG streaming
# load (with translated func_8006E6A8/func_8006E7E8/func_8006E498)

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

## Phase 6E-B21 correction

The raw `func_80071A24` trampoline is A(28h) `bzero(dst,len)`: executable
words `240A00A0 01400008 24090028` at file offset `0x62224`.  A(28h) uses
the A vector `0xA0`; `SysEnqIntRP` is C(02h) via vector `0xC0`.  The checked
provider clears the exact guest byte range and returns the incoming
destination.  `func_80064964` clears `0x800A3060..0x800A317F`, then writes
eight ordered `0xFF` bytes.  Independent contracts are
`tools/b21_bzero_oracle.py` and `tools/b21_order_oracle.py`.

The corrective history preserves provisional `8e90ac7` and incorrect
`14ac77b`; the B22 strict real-disc frontier was `func_80052C6C` from
`func_800527C8` (three identical captures), now advanced to `func_8005BCBC`
(B23), while bootstrap-disc remains at `func_8007F72C` by design.  Native tests: 271/271.

## Testing

```bash
cd pc_port/build
./pe-native-tests   # 271 tests (baseline + guest-RAM/Boot Rung/policy
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
                    #   unknown identities, footprint, strict integration
                    # + 6E-B7: func_800371A4 byte-store contract, width
                    #   truncation, single-byte footprint, integration
                    # + 6E-B8: func_80029388 rung — 2F658 record copies,
                    #   20EFC byte clears, slot-table addresses/guards,
                    #   full-RAM footprints, idempotence, integration
                    # + 6E-B9: func_8005BCA8 empty stub — zero footprint,
                    #   dirty-state preservation, 3E680 integration,
                    #   frontier advance to func_80068D28
                    # + 6E-B10: func_80068D28 exact record state, canary
                    #   footprint, idempotence, 3E680 integration,
                    #   frontier advance to func_800124F8
                    # + 6E-B11: func_800124F8 exact cleared state, canary
                    #   footprint, halfword width, idempotence, 3E680
                    #   integration, frontier advance to func_8001A890
                    # + 6E-B12: func_8001A890 exact cleared state,
                    #   stride-4 halfword footprint, idempotence, 3E680
                    #   integration, frontier advance to func_80034F10
                    # + 6E-B13: func_80034F10 exact cleared state, RMW
                    #   mask, 64x redundant store, canary footprint,
                    #   idempotence, 3E680 integration, frontier
                    #   advance to func_8006536C
                    # + 6E-B14: func_8006536C exact table/index state,
                    #   byte-width proof, canary footprint, idempotence,
                    #   3E680 integration, frontier advance to
                    #   func_80038D1C
                    # + 6E-B15: func_80038D1C both return paths, no-write
                    #   path canary, byte-width proof, 3E680 integration
                    #   (dispatcher fully translated), frontier advance
                    #   past 3E680 to func_8006A9E4
                    # + 6E-B16: func_8006A9E4 rung — 6E6A8 sector→byte
                    #   conversion/guards/wraparound, 6E7E8 RMW on all
                    #   three poll outcomes, 6E498 hit/miss/empty/packed
                    #   fields/read-only footprint, full patterned run
                    #   (exact bytes, lookup slots, provider order),
                    #   2 MiB canary footprint, RamReset rerun, 3E680
                    #   integration; frontier advance to func_800527C8)
```

## Deterministic framebuffer

| Property | Value |
|----------|-------|
| SHA-256 (headless, 3 runs) | `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` |
| Windowed SHA matches headless | ✅ |
| Trace SHA-256 (3 runs) | `42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af` |
| Real-disc load trace (3 runs) | `7b8724acf4d4787f58ca0068e68839f171e2d3f36f72a42f0a4ef03f0041672b` |
| Unsupported traps | 0 |

## Phase 6E-B22 correction

`func_8005DE88` is a translated 23-word resource-list/state initializer at
`0x8005DE88..0x8005DEE3` (file offset `0x4E688`, live split `4CC98.s`). It
links the 12-byte records at `0x800A2090..0x800A217F`, null-terminates the
last link at `0x800A2174`, and initializes `$gp+0x36C..0x380` exactly. It has
no callees and no return value. The strict real-disc frontier is now
`func_80052C6C` from `func_800527C8`, exit status 1; native tests are 271/271.
The independent write-order oracle is `tools/b22_5de88_oracle.py`.

## Next steps

1. **Phase 6E-B continued:** `func_800527C8` rung (next strict-mode
   frontier, from `func_8006A9E4` — 49-word subsystem init with 17
   callees, invoked once inside the cycle-B poll; classify by raw MIPS
   before any implementation), then the remaining subsystem inits
   toward the image-load consumers
2. Identify the first boot asset (likely MDEC logo data)
3. Wire MDEC decoding and display
4. Audio, input, save/load (later phases)

## Constraints

- PCSX-Redux is the retail oracle only — never used as runtime
- Matching decomp remains separate and SHA-exact (SHA `452fb033`; 227 C
  leaves in this repo's matching rebuild)
- No PS1 emulator in the native executable
- The Disc 1 image is user-supplied and read-only; never commit it or
  any derivative captures
- This is NOT a playable game yet
