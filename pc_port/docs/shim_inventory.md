# Shim Inventory — Phase 6E-B32

Bootstrap stubs invoked in the `func_8001220C` (main) → first-clear path.
All stubs are explicitly classified. No anonymous empty stubs.

## Real translated functions (not stubs)

### Phase 6E-B21 correction

`func_80071A24` is the verified retail BIOS A(28h) trampoline, not
SysEnqIntRP: raw words are `240A00A0 01400008 24090028` at executable
`0x80071A24..0x80071A2F` (file offset `0x62224`).  A(28h) is
`bzero(dst,len)`; the checked host adaptation uses `pe_addr_t` and clears
exactly the requested guest byte range.  `func_80064964` calls it with
`dst=0x800A3060`, `len=0x120`, then stores `0xFF` at
`0x800A3078, 0x800A30A0, 0x800A30B0, 0x800A30B8, 0x800A30C0,
0x800A30C4, 0x800A3124, 0x800A3134` in that order.  The return value is
not consumed by this caller.  The general provider returns the incoming
destination, as documented for the BIOS memset family; an executable-wide
scan finds 11 call sites (including `0x8005D70C`, `0x80040238`,
`0x80040BB8` where `$v0` is copied to `$s2`, `0x80040D30`, `0x80042204`,
`0x80042548`, both `0x80064974/0x800649EC`, `0x80084598`, and
`0x80084880`).  C(02h) SysEnqIntRP requires vector `0xC0` and is not this
call.  Independent checks live in `tools/b21_bzero_oracle.py` and
`tools/b21_order_oracle.py`.

| Function | Size | Words | Port file |
|----------|------|-------|-----------|
| `func_8001220C` | - | 187 | `bootstrap/func_8001220C_port.c` |
| `func_800698D4` | - | - | `bootstrap/func_800698D4_port.c` |
| `func_8006E834` | - | 91 | `bootstrap/func_8006E834_port.c` |
| `func_8006E9A0` | - | - | `bootstrap/func_8006E9A0_port.c` |
| `func_8003E610` | 0x70 | 28 | `game/boot/func_8003E610_port.c` |
| `func_8003E680` | 0xD4 | 53 | `game/boot/func_8003E680_port.c` |
| `func_8006A5BC` | 0x90 | 36 | `game/boot/func_8006A5BC_port.c` |
| `func_8006A64C` | 0x28 | **10** | `game/boot/func_8006A64C_port.c` |
| `func_8006A674` | 0x260 | 152 | `game/boot/func_8006A674_port.c` |
| `func_8006A8D4` | 0x110 | **68** | `game/boot/func_8006A8D4_port.c` |

## IMPLEMENTED (real host equivalents)

| Function | PS1 role | Host implementation |
|----------|----------|---------------------|
| `func_80073A44` | VSync | `HostFB_VSync()` |
| `func_80074D28` | SetDispMask | `HostFB_SetDispMask()` |
| `func_80074DC0` | DrawSync | `HostFB_DrawSync()` |
| `func_80074F44` | ClearImage | `HostFB_ClearImage()` |
| `func_800755F0` | PutDispEnv/Present | `HostFB_Present()` |

## HOST_ADAPTED

| Function | Adaptation |
|----------|------------|
| `scratchpad_stack_handoff` | Native stack call, no MIPS scratchpad manipulation |

## BOOTSTRAP_RET (51 stubs invoked in first-clear path)

### func_8001220C callees
- `func_800725DC`
- ~~`func_8006A9E4`~~ — TRANSLATED (Phase 6E-B16; see the func_8003E680
  callees section)
- `func_8006AD40`
- `func_8006ECEC`
- `func_8006F044`
- `func_80069B08(int)`
- `func_8003F3C4`
- `func_801235DC`
- `func_8019234C`
- `func_801909B4`

### func_8003E610 callees (10)
- `func_80073C94`
- `func_8003E754(int, int)`
- `func_8007D054`
- `func_80077F7C`
- `func_80079004(int, int)`
- `func_80079024(int)`
- `func_800409B4`
- `func_8003E944`
- `func_8007EC14`
- `func_80080CC8(int)`

### func_8006A5BC callees (7)
- `func_80085644`
- `func_80086FF8`
- `func_80087024`
- `func_8008682C(int)`
- `func_8007ED58` (returns 1)
- `func_8007F72C` (CdReady — strict-mode gate)
- `func_8007F7A8` (returns 0)

### func_800698D4 callees (5)
- `func_8007F72C` (shared with 6A5BC)
- `func_8007F778`
- `func_80082314`
- `DsSearchFile(PEDISC01.IDF)`
- `DsSearchFile(PE.IMG)`
- `func_80080C48`

### func_8003E680 callees (2000 + 16)
- ~~`func_80070D10`~~ — TRANSLATED (Phase 6E-B1): lagged-Fibonacci RNG table
  init, `game/boot/func_80070D10_port.c`
- ~~`func_80070D6C`~~ (×2000) — TRANSLATED (Phase 6E-B2): lagged-Fibonacci
  RNG advance, `game/boot/func_80070D6C_port.c`; read cursor cycles through
  the 2 index words + 14 retail code words below the table (verbatim
  `|= 0x40` wrap), so retail-exact output needs the exe image in guest RAM
  (`pe_guest_image`); ranged wrapper `func_80070DD0` also translated
- ~~`func_8003E974`~~ — TRANSLATED (Phase 6E-B3): state zeroing
  (5 $gp-relative globals + 32-word array D_800A76F0) + ordered series of 20
  `func_8003EAC8(mask,value)` registration calls, `game/boot/func_8003E974_port.c`
- ~~`func_8003EAC8`~~ — TRANSLATED (Phase 6E-B4): GTE LZCS/LZCR-indexed
  registration leaf (idx = (a0 == 0x80000000) ? 31 : 31 - LZCR(a0); word
  store D_800A76F0[idx]=a1, below-table write at 0x800A76EC for zero/
  all-ones inputs preserved, never clamped), `game/boot/func_8003EAC8_port.c`
  + narrow `PE_GTE_LZCR` in `platform/pe_gte.c`; oracle gate
  `--lzcr-oracle-dump` ≡ `tools/lzcr_oracle.py`.  Call-site correction:
  20 DISTINCT sites (all in func_8003E974, constant args) — the earlier
  "63" counted overlapping split files
- ~~`func_80036DC8`~~ — TRANSLATED (Phase 6E-B5): timer-record init
  dispatcher + leaves func_80036DF8/36E34/36E58 (11 word stores ROM-order;
  three 12-byte records at 0x800A76A0/AC/B8 = {1,0,x}, record 0 field2
  0x1499700; consumers divide field1 by 60 — 60 Hz tick counters),
  `game/boot/func_80036DC8_port.c`
- ~~`func_80073D24`~~ — IMPLEMENTED (Phase 6E-B6): libetc VBlank
  callback slot setter (jump-table field 0x14 → func_80074478 semantics:
  prev = D_8009568C[4]; store if different; return prev), guest-backed
  8-slot table + func_8007440C-faithful dispatcher in
  `platform/pe_callback.[ch]`, wrapper in `platform/pe_libetc.c`;
  oracle gate `--callback-oracle-dump` ≡ `tools/callback_oracle.py`.
  Both func_8003E680 call sites discard the return (retail-matching)
- ~~`func_800371A4(int)`~~ — TRANSLATED (Phase 6E-B7): 3-word
  $gp-relative byte setter (`sb $a0, 0x124($gp)` → D_8009CE94 =
  guest 0x8009CE94), exe-verified words; 2 call sites (3E680 arg 0,
  527C8 arg 1), return unused; `game/boot/func_800371A4_port.c`
- ~~`func_80029388(void)`~~ — TRANSLATED (Phase 6E-B8): slot-table clear
  + default-record init, with leaves func_8002F658 (copy exe-rodata
  records D_80010928→D_800B8A20 0x70 bytes, D_80010998→D_800B0CB0 0x18
  bytes; zero D_8009D1B0/D_8009D1B4) and func_80020EFC (matched decomp
  leaf: 5 $gp-relative byte clears).  7 in-use words at D_800A5D58 +
  i*220 (same SlotRecord table as the decomp's func_8002F9CC leaf) +
  bytes D_8009D2A0/D_8009D2EC; sole call site func_8003E680 @0x8003E700
  (nop delay slot); `game/boot/func_80029388_port.c`
- ~~`func_8005BCA8(void)`~~ — TRANSLATED (Phase 6E-B9): empty jr/nop
  stub (2 words / 0x8 at VRAM 0x8005BCA8, file 0x4C4A8; retail words
  0x03E00008 / 0x00000000).  Zero guest reads/writes; sole call site
  func_8003E680 @0x8003E708 (nop delay slot); `game/boot/func_8005BCA8_port.c`
- ~~`func_80068D28(void)`~~ — TRANSLATED (Phase 6E-B10): 63-word
  double-buffered display-record data initializer at D_800BCF88 (scalar
  block +0x60..0x70, two 16-byte records +0x30+i*0x10, two 8-byte
  records +0x50+i*0x8 with 0xE1000440 GP0-shaped data word; loop byte
  values are retail load-after-store from the just-written scalars;
  write extent 0x800BCFBB..0x800BCFF9); all 63 words exe-verified
  (live split 55430.s); sole call site func_8003E680 @0x8003E710
  (nop delay slot, $v0=0 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_80068D28_port.c`
- ~~`func_800124F8(void)`~~ — TRANSLATED (Phase 6E-B11): 31-word
  subsystem table clear (sw 0 -> 0x8009D300, sh 0 -> 0x8009D308 with
  0x8009D304 untouched, sw 0 -> 0x8009CDFC; 72x11-word matrix at
  D_8009D310, row stride 0x2C, span 0x8009D310..0x8009DF6F; sw 0 ->
  0x8009CE00; 16-word array at D_8009DF70 = 0x8009DF70..0x8009DFAF,
  contiguous with the table end; sw 0 -> 0x8009CE04); all 31 words
  exe-verified (live split 2A0C.s); pure zero-stores, no reads, no
  SDK/GTE/hardware/GPU work; sole call site func_8003E680 @0x8003E718
  (nop delay slot, $v0=0 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_800124F8_port.c`
- ~~`func_8001A890(void)`~~ — TRANSLATED (Phase 6E-B12): 34-word
  subsystem scalar/array clear (sw 0 -> 0x8009CE08; stride-2 halfword
  loop 0x8009CE0C..0x8009CE13; sw 0 -> 0x8009CE14; 20-word array at
  D_8009DFB0 = 0x8009DFB0..0x8009DFFC, contiguous above 124F8's array;
  six stride-4 halfwords 0x8009CE18/1C/20/24/28/2C — interleaved upper
  halfwords untouched, ROM order A8, B8, B4, B0, AC, BC; words
  0x8009D1D8/D1FC/D2F8/D248, ROM order 468, 48C, 588, 4D8; halfwords
  0x8009D264/D1CC, ROM order 4F4, 45C); all 34 words exe-verified
  (live split A404.s); pure zero-stores, no reads, no SDK/GTE/
  hardware/GPU work; sole call site func_8003E680 @0x8003E720
  (nop delay slot, $v0=0 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_8001A890_port.c`
- ~~`func_80034F10(void)`~~ — TRANSLATED (Phase 6E-B13): 45-word
  subsystem table clear + flag-bit clear (sw 0 -> 0x8009D2E8; 512-word
  array at D_800A77F0 = 0x800A77F0..0x800A7FEC; D_800B6A80 = 0 — retail
  stores the same word 64x via a delay-slot loop with no pointer
  advance, reproduced as one store with identical end state; 14x160-word
  matrix at D_800BEA90, row stride 0x280, span 0x800BEA90..0x800C0D8F;
  scalars sw 0 -> 0x8009D2AC/0x8009D20C/0x8009D2F0/0x8009D254/0x8009D224
  and sh 0 -> 0x8009D2A6, ROM order 53C, 49C, 580, 536, 4E4, 4B4; sole
  guest read + RMW D_800B0CD8 &= ~0x3000, store in the jr $ra delay
  slot); all 45 words exe-verified (live split 2422C.s); no SDK/GTE/
  hardware/GPU work; sole call site func_8003E680 @0x8003E728
  (nop delay slot, $v0=&D_800B0CD8 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_80034F10_port.c`
- ~~`func_8006536C(void)`~~ — TRANSLATED (Phase 6E-B14): 19-word
  subsystem record-table clear + index byte clear (28x3-word table at
  D_800A3180, row stride 0xC — contiguous 84 words, span
  0x800A3180..0x800A32CF; sb 0 -> 0x44($gp) = 0x8009CDB4, the
  current-record index byte — func_800653B8 below reads lbu 0x44($gp)
  and indexes D_800A3180 + byte*12, confirming the 28x12-byte record
  structure); all 19 words exe-verified (live split 55430.s); no
  reads, no SDK/GTE/hardware/GPU work; sole call site func_8003E680
  @0x8003E730 (nop delay slot, $v0=0 unconsumed); idempotent incl.
  after PE_RamReset; `game/boot/func_8006536C_port.c`
- ~~`func_80038D1C(void)`~~ — TRANSLATED (Phase 6E-B15): 11-word
  (0x2C, exe 0x80038D1C–0x80038D44, file 0x2951C) byte test-and-clear
  status leaf: lbu D_80091A20; if nonzero sb 0 → D_80091A20 and return
  0, else return 0xFF (255); conditional write only; all 11 words
  exe-verified (live split 2951C.s); ALSO a matched decomp C leaf
  (src/func_80038D1C.c); two exe call sites, both return-ignored:
  func_8003E680 @0x8003E738 (final call, void epilogue follows) and
  func_8006E9A0 @0x8006EB7C; with this leaf func_8003E680 is FULLY
  translated; `game/boot/func_80038D1C_port.c`
- ~~`func_8006A9E4(void)`~~ — TRANSLATED (Phase 6E-B16): 215-word
  (0x35C, exe 0x8006A9E4–0x8006AD3F, file 0x5B1E4, live split 5B1E4.s)
  PE.IMG streaming resource load: ClearImage({0,0,0x3FF,0x1FF},0,0,1)
  via REAL func_80074F44; four sector-read/poll cycles (table
  D_800930DC..E8, dests D_800A8028 and lw(D_800B0E6C); A/B polls
  restart-on-(-1), C/D sltu-clamped re-poll); 0x10A50-byte copy to
  D_800E2858; two archive lookups (keys 0x57D40D84/0x57D41D84, exact
  delay-slot order D_800B0E20→E18→E1C); 0x1400-byte copy to
  lw(D_800B0E08); all 215 words exe-verified; sole call site
  func_8001220C @0x80012284 (nop slot, return ignored);
  `game/boot/func_8006A9E4_port.c`.  Dependencies translated with it:
  func_8006E6A8 (11 words, issue wrapper with sector→byte << 11 at the
  host-adaptation boundary), func_8006E7E8 (19 words, poll + D_800B0CD8
  &= 0xFEFFBFFF on st∈{-1,0}), func_8006E498 (31 words, archive lookup,
  guest-address result).  func_80087090 remains UNRESOLVED via the
  centralized boundary.
  func_800527C8 is **TRANSLATED** (Phase 6E-B17):
  49 words / 0xC4 at 0x800527C8, live split 42FC8.s, all exe-verified.
  Multi-subsystem bootstrap dispatcher: 17 calls (16 distinct callees).
  7 translated leaves (func_8005B890, func_8005BC98, func_8004F808,
  func_80042B38, func_80051084, func_800371A4) + 3 direct sw clears;
  every direct callee is now translated, including func_80052C6C,
  func_8005BCBC, func_8005D6F4, func_80051CC4 (B39), and func_80042C78.
  B40 translates the first B39 nested dependency, func_8005332C; only
  func_8005218C remains unresolved in that path.
  Sole call site func_8006A9E4 @0x8006AAD0 ($s1-guarded one-shot in
  the cycle-B poll loop); void(void), return unconsumed.
  `game/boot/func_800527C8_port.c`, leaf implementations in
  `game/boot/func_800{5B890,5BC98,4F808,42B38,51084}_port.c`.
- `func_80052C6C`  ← TRANSLATED (B23: resource-table search + init)
- `func_8005BCBC`  ← TRANSLATED (B24: resource-state pointer/count selector)
- `func_80051CC4`  ← TRANSLATED (B39: 77-word resource command-state
  initializer; independent `tools/b39_oracle.py`)
- `func_8005332C`  ← TRANSLATED (B40: 42-word signed-ID resource-record
  lookup; independent `tools/b40_oracle.py`); its direct B39 path now reaches
  the later sibling boundary `func_8005218C`

### func_8006E834 callees (9)
- `func_80086FF8` (shared with 6A5BC)
- `func_8006E6D4(int,int,uchar*,int)`
- `func_800811E4(void*)`
- `func_80072714`
- `func_800726C4`
- `func_80072724`
- `func_800749D8(SetDefDispEnv)`

### func_8006E9A0 callees (7)
- `func_8005E588`
- `func_80066B60(int)`
- `ClearOTagR`
- `func_80068E24`
- `func_80070E54`
- ~~`func_80038D1C`~~ — TRANSLATED (Phase 6E-B15, shared with 3E680;
  see the func_8003E680 callees section)

### Phase 6E-B22

`func_8005DE88` is translated retail logic: 23 words at executable
`0x8005DE88..0x8005DEE3`, file offset `0x4E688`, live split `4CC98.s`.
It has no callees, links 12-byte records from `0x800A2090` through
`0x800A2174`, then null-terminates the final link and initializes the six
`$gp` state words at `0x8009D0DC..0x8009D0F0`. Strict real-disc execution now
reaches `func_80052C6C` from `func_800527C8` (exit 1).

### Phase 6E-B24

`func_8005BCBC` is translated retail logic: 21 words at executable
`0x8005BCBC..0x8005BD0F`, file offset `0x4C4BC`, live split `4C4BC.s`.
Resource-state pointer/count selector: stores the incoming record pointer
into `D_8009D0C8` ($gp+0x358), selects a buffer base into `D_8009D0C0`
($gp+0x350) — a0 != 0 → 0x800C20A4 (+0x10 iff byte6(a0) == 9); a0 == 0 →
0x800C0DE0 (+0x10 iff D_8009D218 != 0) — and stores count 8 into
`D_8009D0C4` ($gp+0x354). Returns 8 on every path; both call sites
(func_800527C8 @0x80052834 with a0=0 in the delay slot, func_8004DD64
@0x8004DF28 with a0 = D_8009D004) discard the return. No callees; the
three state words are guest-RAM resident (shared with func_8005BD10,
func_8005BE1C, and the func_8005D6F4 region). Strict real-disc execution
now reaches `func_8005D6F4` from `func_800527C8` (exit 1).

### Phase 6E-B25

`func_8005D6F4` is translated retail logic: 147 words at executable
`0x8005D6F4..0x8005D93F`, file offset `0x4DEF4`, live split `4CC98.s`.
Resource-buffer + display-state initializer: REAL bzero
`0x800C0DE0..0x800C20C3` via func_80071A24; two 0xFF fills of
`0x800C0DF0..0x800C0DF7` (retail reload-per-iteration loops); two
0xFF-terminated string copies whose sources come from the REAL
func_8005DC4C (B26);
`D_8009D0C8/C0/C4` + `D_8009D218` state stores (block-1 order C8, C0,
C4; block-2 order flag, C4, C8, C0); `sh 0x0203 → 0x800C1F80`,
`sw 0x00404040 → 0x800C0E44`; timer-tick clears `0x800A76A4/B0/BC/C8`;
terminator bytes `0x800C20A4/B4`; returns 0xFF (sole call site
func_800527C8 @0x8005283C, return discarded).  After B27, SIX
unresolved callees route through the centralized bootstrap boundary in
retail ROM order: func_8005CCA4, func_800614AC,
func_8005E884, func_8005E850, func_800649D0, func_80052790 — plus the
statically dead func_8005DC9C arm (C8 is always 0 at the selection).

### Phase 6E-B26

`func_8005DC4C` is translated retail logic: 20 words at executable
`0x8005DC4C..0x8005DC9B` (exclusive end = func_8005DC9C), file offset
`0x4E44C`, live split `4CC98.s:1779-1802`.  A read-only PE.IMG
message/string-table lookup over a nested relative-offset archive based
at the cycle-A streaming destination `0x800A8028`:

```
ptr = mem32[0x800A802C] + 0x800A8028
tbl = ptr + mem32[ptr + 4]
return (idx <u mem16[tbl]) ? tbl + sext16(mem16[tbl + 2 + 2*idx]) : 0
```

Both header addresses are sign-extended from `lui 0x800B` + a bit-15-set
immediate, so they are `0x800A80xx`, NOT `0x800B80xx` (same finding as
B23's func_8005DB44).  Record stride is **2** (`sll $v0,$a0,1`) and the
entry is a **signed** 16-bit offset (`lh`), so records must lie within
`tbl ± 32 KiB`.  Zero is retail's own out-of-range return
(`addu $v0,$zero,$zero`), not an invented sentinel.

Signature: ONE argument.  No instruction in the body reads `$a1`; the
values retail leaves there at the call sites are residual (in
func_8005D6F4 the 0xFF fill constant, and at 0x8004685C the jal delay
slot is `sb $v0,0($s1)` — not argument setup at all).

Three guest reads on the failure path, four on the success path, **zero
guest writes**, no callees, no SDK/GTE/GPU/MDEC/SPU/disc/input activity,
cannot block, deterministic from guest state alone, repeated calls
stable.  39 call sites in 24 distinct callers exe-wide; observed constant
indices 3..117, all below the measured count of 120; no call site
compares the return to zero.

Measured against the real Disc 1 USA image: `R = 0x30`, `S = 0x14`,
`count = 120`, entries `242..1971` (records `0x800A815E..0x800A881F`);
`min(entry) == 2 + 2*count` exactly.  Index 30 resolves to `0x800A82A9`
(`10 48 30 FF`).  Independent oracle: `tools/b26_oracle.py`.

The archive is guest-resident and arrives from PE.IMG via cycle A of
func_8006A9E4 before the dispatcher runs.  With the region zeroed the
lookup correctly returns 0 and func_8005D6F4's copy loop dereferences
address 0 — surfaced by the checked-access layer, deliberately not
masked with a KUSEG mirror, a clamp, or a bypass.  Strict real-disc
execution now reaches `func_80053D2C` from `func_8005CCA4` (exit 1,
normal and sanitizer agree).

### Current Phase 6E-B32

`func_800614AC` is translated retail logic: 36 instructions / `0x90` bytes
at executable `0x800614AC..0x8006153B` (exclusive end `0x8006153C`, file
offset `0x51CAC`, live split `asm/disc1/51CAC.s`). It stores
`a0 & 0x00FFFFFF` at `0x8009D14C`, computes the three pairwise byte means
with the retail arithmetic and saturation branches, stores the packed result
at `0x8009D150`, and returns that result. It has no guest reads, direct
callees, hardware activity, blocking, pointer storage, clamping, or fallback.
The `func_8005D6F4` call is at `0x8005D8C8`, with `a0=0x00404040` formed by
the delay-slot `ori`; its return is discarded. The remaining sibling calls
are `func_8005E884`, `func_8005E850`, `func_800649D0`, and
`func_80052790` in raw ROM order. The independent `tools/b32_oracle.py`
verifies the executable SHA-1 and all 36 words, models delay slots, and
checks ordered writes and return values. Native and fresh ASan/UBSan suites
are 344/344. Real-disc strict now stops at `func_8005E884` from
`func_8005D6F4`; bootstrap strict remains at `func_8007F72C` from
`func_800698D4`. Nothing has been pushed and the next rung has not started.

### Historical Phase 6E-B30

`func_80042C78` is translated retail logic through its proven 16-instruction
prefix at executable `0x80042C78..0x80042CB4` (exclusive end `0x80042CB8`,
file offset `0x33478`). With retail `$gp = 0x8009CD70`, it writes zero to
`$gp+0x168/0x170/0x174`, writes `0x20` to `$gp+0x16C`, calls translated
`func_80042CC4` with `a0=0x90` and `a1=0xFF` after the delay slot, and writes
`0x48` to `$gp+0x17C`. The direct guest footprint is exactly
`0x8009CED8`, `0x8009CEDC`, `0x8009CEE0`, `0x8009CEE4`, and `0x8009CEEC`.
The two executable callers are `func_8005CCA4` at `0x8005CFF8` and
`func_800527C8` at `0x8005284C`; both discard the void return.

`tools/b30_oracle.py` independently transcribes and executes all 16 words,
checks every word against executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, and verifies the delayed call
arguments, ordered writes, and return state. The final native and ASan/UBSan
suites were 339/339. B31 supplies the former dependency. No generated or
retail artifacts were added.

### Historical Phase 6E-B31

`func_80042CC4` is translated retail logic: 31 instructions / `0x7C` bytes
at executable `0x80042CC4..0x80042D3C` (exclusive end `0x80042D40`, file
offset `0x334C4`, live split `asm/disc1/334C4.s`). It is a void leaf taking
`(a0, a1)`. It clears `0x800A1878`, executes the initial color-base shift in
the branch delay slot, fills the termination-dependent byte ramp while signed
`lbu < a1` holds, and stores `(cursor - 0x800A1878) + 1` at `0x8009CEE0`
(`$gp+0x170`). The B30 call `(0x90, 0xFF)` produces
`00 90 CF EA F6 FB FD FE FF` and count 9. Direct writes are the actual
subset of `0x800A1878..0x800A1887` plus the word at `0x8009CEE0`; it has no
direct callees or SDK/GPU/disc/audio/input operations. Call sites are
`func_80042C78 @ 0x80042C98` and `func_8005D2B4 @ 0x8005D5E8`; both discard the void return,
and incoming a2/a3 are overwritten before use.

Independent oracle `tools/b31_oracle.py` checks executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, all 31 words, delay slots,
ordered writes, threshold paths, and counts. Native and fresh ASan/UBSan
tests are 341/341. Real-disc strict now stops at `func_800614AC` from
`func_8005D6F4`; bootstrap strict remains at `func_8007F72C` from
`func_800698D4`. Nothing has been pushed and the next rung has not started.

### Historical Phase 6E-B29 (accepted commit `eedd456`)

`func_80053D2C` is translated retail logic: 80 instructions / `0x140` bytes
at executable `0x80053D2C..0x80053E6B` (exclusive end `0x80053E6C`, file
offset `0x4452C`). It scans the shared table at `D_8009D048` with count
`D_8009D050`, calls translated `func_8005DB44`, dispatches record types 1–18,
and returns the exact retail status. Types 1–9 call unresolved
`func_80053968` with `a0 = arg`; types 16–18 call unresolved
`func_80053B48` with no arguments. Both calls use `Bootstrap_ReturnInt` from
the centralized boundary. Types 10 and 12–15 store `arg` as one halfword at
the first free table slot; `arg >= 0x100` uses the same store path without a
record lookup. Missing records, type 11, and unknown types return 0; a full
table returns 1. The only direct guest write is that exact halfword store;
there is no blocking, host pointer leakage, low-address mirror, clamping, or
address-zero fallback.

The independent `tools/b29_oracle.py` transcription contains all 80 words,
checks every word against executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, and executes the transcription
with delay slots, table dispatch, exact writes, return paths, and controlled
unresolved calls. The final native and ASan/UBSan suites are 337/337. The
completed B28 provenance remains provisional `a1559ae` plus corrective
`cd2e375`; no generated or retail artifacts were added.

### Historical Phase 6E-B28 (corrective)

`func_8005CCA4` is translated retail logic: 223 words at executable
`0x8005CCA4..0x8005D01F` (exclusive end `0x8005D020`, file offset
`0x4D4A4`).  The provisional B28 implementation is commit `a1559ae`; the
oracle/bootstrap corrective is commit `cd2e375`. A resource-table initializer that: writes 7
halfwords via `*(u16*)func_8005DB8C(i)`, reads PE.IMG archive header via
func_8005DBAC, zeros 50 halfwords descending from **0x800C0EAA** to
**0x800C0E48** (v1 = D_8009D048 + 0x62) — this range is ABOVE the earlier
GA_E24/E28 stores, which survive — searches resource tables, sets the
shared host state globals (D_8009D048/50/58/64, $gp base 0x8009CD70), and
calls func_800438C0(0x3D).  GA_E40 (below the loop) receives a u16 0x3D
store after the loop; GA_E22 is written 1 unconditionally.
Coupled callees: func_800438C0 (8 words, masked-state setter),
func_8005DB8C (8 words, table base), func_8005DBAC (20 words,
clamped table base).  All 223 words are independently transcribed (W_5CCA4),
compared against the SHA-verified executable, and executed by the B28 oracle
(`tools/b28_oracle.py`).  Fixture correction: FxPattern offsets 0x10-0x17
return 0 (retail BSS state).  Split-brain fix: D_8009D018 + 7 globals
moved to extern with PE_Sdk_ResetState reset.  Bootstrap-disc fixture
seeds valid archive at D_800A8028 (R=0x30, S=0x14, count=120,
entry[30] → 0xFF record) so func_8005DC4C returns `0x800A8400`.
`PE_StoreU32(0x800A803C, 0xA49D968F)` makes malformed
`func_8005DBAC(0)` return `0x24A816B7`, without dereferencing that result.
Address zero remains invalid; no KUSEG or low-address mirror exists. 335/335
tests pass (normal + ASan/UBSan).

### Historical Phase 6E-B27

`func_80052594` is translated retail logic: 22 words at executable
`0x80052594..0x800525EB`, file offset `0x42D94`, live split
`42D94.s` (yaml segment `[0x42D94, asm]`).  A leaf string-copy function
that copies bytes from `$a0` (source pointer) into a fixed 8-byte
buffer at `D_80091694`, stopping at the `0xFF` terminator or buffer
full, stores the byte count at `D_8009169D`, and returns the count
(0-8).  The `0xFF` terminator is NOT copied.  No callees, no
`$gp` usage (all addresses via absolute `lui`/`addiu`), no
SDK/GTE/GPU/MDEC/SPU/disc/input activity.

Five executable call sites (`jal` word `0x0C014965`):
`func_8005D6F4` @ `0x8005D898` (delay: `addu $a0,$v0`; return
discarded), `func_8004DD64` @ `0x8004E28C/0x8004E428/0x8004E6A8`
(delay: `addu $a0,$v0`; return discarded), and `func_8005C46C` @
`0x8005C46C` (delay: `nop`; return discarded).  All call sites discard
the return value.

`D_80091694` (8-byte buffer) and `D_8009169D` (1-byte count) are
guest-RAM resident.  Exhaustive executable-wide `lui`/`addiu` scan
found NO readers — these are write-only stores.  Not `$gp`-relative
(`$gp = 0x8009CD70`, offset would be `-0xB6DC`, outside small-data
area).

Independent oracle: `tools/b27_oracle.py`.

## Remaining bootstrap providers

With Disc 1, strict mode stops at pre-existing `func_80053968` from
translated-prefix `func_80053D2C`. B40's correction to the shared
`func_8005DB44` record lookup makes that dormant B29 boundary reachable
before the translated `func_80051CC4` path. The later `func_8005218C`
boundary remains untouched. The `--bootstrap-disc` fixture still
stops at `func_8007F72C` (CdReady) by design. Normal and fresh sanitizer
builds agree on both frontiers.

Disc-path providers are REAL since Phase 6E-A (host disc model over the
read-only image): `func_8007F72C` (CdReady), `func_8007F778`,
`func_80082314` (PVD verify), `DsSearchFile`, `func_80080C48`
(CdPosToInt), `func_8006E6D4` (image read), `func_800811E4`
(completion poll).  Remaining unresolved providers for boot-to-logo:
- `func_8005DC9C` — dead-arm callee of func_8005D6F4 (C8 always 0);
  the same lookup as func_8005DC4C but reading `ptr+8` instead of
  `ptr+4`, i.e. a second table in the same sub-chunk
- `func_8005CCA4` — completed B28 translated rung
- `func_80053D2C` — completed B29 translated rung; its unresolved
  dependencies are `func_80053968` and `func_80053B48`
- `func_80042C78` — completed B30 translated prefix; its B31 dependency is
  complete
- `func_80051CC4` — completed B39 translated rung; B40 completed its first
  nested dependency `func_8005332C`, while `func_8005218C` remains unresolved
- `func_80087090` — SPU upload retry wrapper
- `func_800749D8` — display environment setup (currently memset stub)
- `func_800752AC` (ClearOTagR) — ordering table clear
