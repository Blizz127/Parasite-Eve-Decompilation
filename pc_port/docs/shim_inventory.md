# Shim Inventory — Phase 6D-R

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
  10 unresolved callees in retail ROM order routed through the
  centralized boundary:
  func_800528F0, func_8005E588, func_80062568, func_80064964,
  func_8005DE88, func_80052C6C, func_8005BCBC, func_8005D6F4,
  func_80051CC4, func_80042C78.
  Sole call site func_8006A9E4 @0x8006AAD0 ($s1-guarded one-shot in
  the cycle-B poll loop); void(void), return unconsumed.
  `game/boot/func_800527C8_port.c`, leaf implementations in
  `game/boot/func_800{5B890,5BC98,4F808,42B38,51084}_port.c`.
- `func_800528F0`  ← current strict-mode frontier (first unresolved
  callee inside the translated func_800527C8 dispatcher)

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

## Remaining bootstrap providers

With `--disc-image`, strict mode stops at `func_800528F0` (first
unresolved callee INSIDE the translated `func_800527C8` dispatcher, from
`func_8006A9E4`) — past the fully translated func_8003E680, func_8006A9E4
streaming-load rung, AND func_800527C8 dispatcher (7 translated leaves +
3 direct sw clears committed before the first unresolved callee).  The
`--bootstrap-disc` fixture still stops at `func_8007F72C` (CdReady) by
design: the fixture never initializes the drive lane.

Disc-path providers are REAL since Phase 6E-A (host disc model over the
read-only image): `func_8007F72C` (CdReady), `func_8007F778`,
`func_80082314` (PVD verify), `DsSearchFile`, `func_80080C48`
(CdPosToInt), `func_8006E6D4` (image read), `func_800811E4`
(completion poll).  Remaining unresolved providers for boot-to-logo:
- `func_800528F0` — first unresolved dispatcher callee (current strict frontier)
- `func_8005E588`, `func_80062568`, `func_80064964`, `func_8005DE88`,
  `func_80052C6C`, `func_8005BCBC`, `func_8005D6F4`, `func_80051CC4`,
  `func_80042C78` — remaining 9 unresolved dispatcher callees
- `func_80087090` — SPU upload retry wrapper
- `func_800749D8` — display environment setup (currently memset stub)
- `func_800752AC` (ClearOTagR) — ordering table clear
