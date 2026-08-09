# ACTIVE HANDOFF

Single source of truth for current working state. Read this first; update after
every meaningful change. Prefer shortening over accruing.

## PC port branch state (this checkout)

## Phase 6E-B53E LoadImage issue worker verified

The exact B53D base is
`da297bea4aafebc734a21e55be2d98bf6d624565`. B52 translates the complete
Psy-Q LoadImage wrapper `func_8007506C` (24 words, 0x60 bytes,
`0x8007506C..0x800750CC`) and its complete read-only debug validator
`func_80074E28` (71 words, 0x11C bytes,
`0x80074E28..0x80074F44`). The wrapper validates its transient four-signed-
halfword RECT, reloads `D_80095744`, and dispatches through
`jtb[2] = func_80076C34` with `a0 = jtb[8] = func_80076664`, `a1 = RECT *`,
`a2 = 8`, and `a3 = data`.

B53A completely recovered that hardware contract. B53B now implements one
private native authority in `pc_port/platform/pe_gpu.[ch]`: 1024x512x16
VRAM, GPUSTAT ready bit 26, GP0 A0 parsing, the proven GP1 subset, DMA2 raw
registers and tokenized deferred completion, DPCR/DICR channel-2 state, and
an explicit VBlank counter. It has no retail ring or callback state and does
not touch HostFB.

B53C translates complete 13-word timeout helper `func_800773D0` and the
dispatcher prefix; B53D adds the single 16-bit I_MASK authority and complete
6-word `func_80073E10`. B53E translates the 143-word LoadImage issue worker
`func_80076664` (`0x80076664..0x800768A0`, body SHA-256
`79dd44e3819f51eb5928c9af3ec0d6906cc3d95765dc2718c3c10e49ef78e0f7`)
against B53B. It preserves signed RECT clamps, complete source-span safety,
the inert GPUSTAT bit-26 wait, exact GP1/GP0 A0 sequence, CPU-fed remainder,
and asynchronous DMA2 MADR/BCR/CHCR issue. The worker does not complete DMA,
pump the ring, or invoke callbacks. An executable-wide DPCR audit proved the
enable belongs to earlier ResetGraph initialization: guard-passing
ResetCallback now writes `0x33333333`, collapsed `func_80077144` ORs
`0x800`, and canonical entry is `0x33333B33`; the worker never force-enables
the channel. The first canonical request issues DMA and leaves it busy, so
the second request selects enqueue. Current strict frontier is therefore
`func_80073CF4` from `func_80076C34`, before ring construction/publication.
A full ring and the worker timeout-recovery suffix expose `func_80077404`.
The B52 transient RECT remains two by-value words with no native pointer
retained. B53C/D/E oracles independently execute literal words with exact
delay slots and explicit hardware/dependency inputs.

The raw retail trampoline at executable `0x80071A24..0x80071A2F` (file
offset `0x62224`) is exactly `240A00A0 01400008 24090028`: load `$t2=0xA0`,
jump through `$t2`, and load `$t1/r9=0x28` in the delay slot.  Authoritative
BIOS tables identify this as A(28h) `bzero(dst,len)`; C(02h)
`SysEnqIntRP(priority,struc)` uses vector `0xC0`.  The corrected provider
uses checked guest-memory `PE_Fill`, and `func_80064964` clears
`0x800A3060..0x800A317F` before its eight ordered `sb 0xFF` stores.
Independent contracts are `pc_port/tools/b21_bzero_oracle.py` and
`pc_port/tools/b21_order_oracle.py`.  History remains intact: provisional
`8e90ac7`, incorrect `14ac77b`, then one corrective commit only after gates.

| Fact | Value | Derive |
| --- | --- | --- |
| Branch | `phase6e-b-provider-frontier` (from `phase6d-s-guest-memory-safety` @ `9ac15f8`) | `git branch --show-current` |
| Port phase | **6E-B53E LoadImage issue worker verified**; retail strict frontier `func_80073CF4` from `func_80076C34` | fresh normal and ASan/UBSan `pe-native-tests` (506/506) |
| Guest memory | Contiguous 2 MiB guest RAM; `pe_addr_t`; typed lvalue macros in `psx_compat.h`; `PE_RamInit/Reset/Destroy` | `pc_port/platform/pe_guest_ram.[ch]` |
| Policy | Centralized `Bootstrap_ReturnInt/Void` + strict abort; deterministic provider sequences | `pc_port/bootstrap/pe_bootstrap.[ch]` |
| Disc layer | Read-only user-supplied Disc 1 (BIN/CUE MODE2/2352, ISO9660); real providers func_80082314/func_80081414/func_80080C48/func_8006E6D4/func_800811E4; `func_800698D4` retail mount sequence; PE.IMG bytes land at `D_80011614` (0x8010BD00); retail boot exe (SYSTEM.CNF `BOOT=`, PS-X EXE) loaded into guest RAM at taddr with `--disc-image` | `pc_port/platform/pe_disc.[ch]`, `pc_port/platform/pe_libcd.c`, `pc_port/platform/pe_guest_image.[ch]` |
| RNG | func_80070D10/70D6C/70DD0 TRANSLATED (lagged-Fibonacci; verbatim `i2 \|= 0x40` wrap cycles through 14 retail code words below the table); oracle gate `--rng-oracle-dump` ≡ `pc_port/tools/rng_oracle.py` on retail exe | `pc_port/game/boot/func_80070D{10,6C,D0}_port.c` |
| Subsystem init | func_8003E974 + func_8003EAC8 TRANSLATED (state clear + 20 ROM-ordered registrations; GTE LZCS/LZCR leaf: idx = (a0==0x80000000) ? 31 : 31−LZCR(a0), below-table write at 0x800A76EC preserved, never clamped; 20 distinct call sites, not 63); oracle gate `--lzcr-oracle-dump` ≡ `pc_port/tools/lzcr_oracle.py`. func_80036DC8 + leaves func_80036DF8/36E34/36E58 TRANSLATED (timer-record init: three 12-byte records at 0x800A76A0/AC/B8 = {1,0,x}, record 0 field2 0x1499700; consumers divide field1 by 60 — 60 Hz tick counters) | `pc_port/game/boot/func_8003E{974,AC8}_port.c`, `pc_port/game/boot/func_80036DC8_port.c`, `pc_port/platform/pe_gte.c` (`PE_GTE_LZCR`) |
| VBlank callbacks | func_80073D24 IMPLEMENTED (libetc jump-table wrapper, slot 4 forced, return forwarded → func_80074478 semantics: prev = D_8009568C[4], store-if-different, return prev); guest-backed 8-slot table at 0x8009568C + dispatch counter at 0x800956AC; func_8007440C-faithful dispatcher; typed guest→host binding map, full pointer width, unknown identities = visible errors; slot 8 aliases the counter (exact retail arithmetic preserved); oracle gate `--callback-oracle-dump` ≡ `pc_port/tools/callback_oracle.py` (MIPS interpreter on the verified retail words) | `pc_port/platform/pe_callback.[ch]`, `pc_port/platform/pe_libetc.c` |
| Boot globals | func_800371A4 TRANSLATED (3-word $gp-relative byte setter: `sb $a0, 0x124($gp)` → D_8009CE94 = guest 0x8009CE94, exe-verified words; call sites 3E680 arg 0 + 527C8 arg 1, returns unused; sole reader func_80037870 off boot path). func_80029388 TRANSLATED (slot-table clear + record init: 7 in-use words at D_800A5D58 + i*220 — same 7×220 SlotRecord table as decomp leaf func_8002F9CC — plus bytes D_8009D2A0/D_8009D2EC; leaves func_8002F658 = rodata-record copies D_80010928→D_800B8A20 0x70 / D_80010998→D_800B0CB0 0x18 + zero D_8009D1B0/D_8009D1B4, func_80020EFC = matched 5-byte clear; sole 29388 call site 3E680 @0x8003E700, nop slot). func_8005BCA8 TRANSLATED (empty jr/nop stub: 2 words at 0x8005BCA8 / file 0x4C4A8 = `03E00008 00000000`, zero guest effects; sole call site 3E680 @0x8003E708, nop slot; matching decomp C leaf since Phase 5AH). func_80068D28 TRANSLATED (63 words / 0xFC at 0x80068D28, live split 55430.s, all exe-verified: double-buffered display-record data init at D_800BCF88 — scalar block +0x60..0x70, two 16-byte records +0x30+i*0x10, two 8-byte records +0x50+i*0x8 with 0xE1000440 GP0-shaped DATA word, not a hardware write; loop bytes are retail load-after-store from the scalars; write extent 0x800BCFBB..0x800BCFF9; idempotent incl. after PE_RamReset; sole call site 3E680 @0x8003E710, nop slot, $v0=0 unconsumed). func_800124F8 TRANSLATED (31 words / 0x7C at 0x800124F8, live split 2A0C.s, all exe-verified: subsystem table clear — sw 0 → 0x8009D300, sh 0 → 0x8009D308 with 0x8009D304 untouched, sw 0 → 0x8009CDFC/0x8009CE00/0x8009CE04, 72×11-word matrix at D_8009D310 stride 0x2C span ..0x8009DF6F, contiguous 16-word array at D_8009DF70 span ..0x8009DFAF; pure zero-stores, no reads, no SDK/GTE/hardware/GPU work; idempotent incl. after PE_RamReset; sole call site 3E680 @0x8003E718, nop slot, $v0=0 unconsumed). func_8001A890 TRANSLATED (34 words / 0x88 at 0x8001A890, live split A404.s, all exe-verified: subsystem scalar/array clear — sw 0 → 0x8009CE08, stride-2 halfword loop 0x8009CE0C..CE13, sw 0 → 0x8009CE14, 20-word array at D_8009DFB0 span ..0x8009DFFC contiguous above 124F8's array, six stride-4 halfwords 0x8009CE18/1C/20/24/28/2C with interleaved upper halves untouched (ROM order A8, B8, B4, B0, AC, BC), words 0x8009D1D8/D1FC/D2F8/D248 (ROM order 468, 48C, 588, 4D8), halfwords 0x8009D264/D1CC (ROM order 4F4, 45C); pure zero-stores, no reads; idempotent incl. after PE_RamReset; sole call site 3E680 @0x8003E720, nop slot, $v0=0 unconsumed). func_80034F10 TRANSLATED (45 words / 0xB4 at 0x80034F10, live split 2422C.s, all exe-verified: subsystem table clear + flag-bit clear — sw 0 → 0x8009D2E8, 512-word array at D_800A77F0 span ..0x800A7FEC, D_800B6A80 = 0 (retail stores the same word 64× via delay-slot loop with no pointer advance, reproduced as one store), 14×160-word matrix at D_800BEA90 stride 0x280 span ..0x800C0D8F, scalars sw 0 → 0x8009D2AC/D20C/D2F0/D254/D224 + sh 0 → 0x8009D2A6 (ROM order 53C, 49C, 580, 536, 4E4, 4B4), sole guest read + RMW D_800B0CD8 &= ~0x3000 with the store in the jr $ra delay slot; idempotent incl. after PE_RamReset; sole call site 3E680 @0x8003E728, nop slot, $v0=&D_800B0CD8 unconsumed). func_8006536C TRANSLATED (19 words / 0x4C at 0x8006536C, live split 55430.s, all exe-verified: subsystem record-table clear + index byte clear — 28×3-word table at D_800A3180, row stride 0xC, contiguous 84 words span ..0x800A32CF, sb 0 → 0x44($gp) = 0x8009CDB4 current-record index byte (func_800653B8 below reads lbu 0x44($gp) and indexes D_800A3180 + byte×12, confirming 28×12-byte records); no reads, no SDK/GTE/hardware/GPU work; idempotent incl. after PE_RamReset; sole call site 3E680 @0x8003E730, nop slot, $v0=0 unconsumed). func_80038D1C TRANSLATED (11 words / 0x2C at 0x80038D1C, live split 2951C.s, all exe-verified + matched decomp C leaf: byte test-and-clear status leaf — lbu D_80091A20, if nonzero sb 0 → D_80091A20 return 0, else return 0xFF; conditional write only; two call sites both return-ignored: 3E680 @0x8003E738 final call + 6E9A0 @0x8006EB7C; with this leaf func_8003E680 FULLY translated) | `pc_port/game/boot/func_800371A4_port.c`, `pc_port/game/boot/func_80029388_port.c`, `pc_port/game/boot/func_8005BCA8_port.c`, `pc_port/game/boot/func_80068D28_port.c`, `pc_port/game/boot/func_800124F8_port.c`, `pc_port/game/boot/func_8001A890_port.c`, `pc_port/game/boot/func_80034F10_port.c`, `pc_port/game/boot/func_8006536C_port.c`, `pc_port/game/boot/func_80038D1C_port.c` |
| Streaming load | func_8006A9E4 TRANSLATED (215 words / 0x35C at 0x8006A9E4, live split 5B1E4.s, all exe-verified: ClearImage({0,0,0x3FF,0x1FF},0,0,1) via REAL func_80074F44; four PE.IMG sector-read/poll cycles — table D_800930DC..E8, dests D_800A8028 / lw(D_800B0E6C), A/B restart-on-(-1), C/D sltu-clamped re-poll, sector counts proven by the 34-sector/67792-byte and 3-sector/5120-byte cycle/copy pairs; 0x10A50-byte copy to D_800E2858; two func_8006E498 lookups keys 0x57D40D84/0x57D41D84 exact delay-slot order D_800B0E20→E18→E1C; 0x1400-byte copy to lw(D_800B0E08); sole call site func_8001220C @0x80012284, nop slot, return ignored). Dependencies TRANSLATED: func_8006E6A8, func_8006E7E8, func_8006E498, func_800527C8. B45: func_80087090 TRANSLATED (retry wrapper). B46: func_800851A8 PREFIX TRANSLATED (magic check + error path). B47: func_80085EB4 TRANSLATED (SPU address validation); func_800851A8 extended prefix past func_80085EB4 through payload copy; func_800850F4 (DMA transfer) remains UNRESOLVED | `pc_port/game/boot/func_8006A9E4_port.c`, `func_8006E6A8_port.c`, `func_8006E7E8_port.c`, `func_8006E498_port.c`, `func_80087090_port.c`, `func_800851A8_port.c`, `func_80085EB4_port.c` |
| Dispatcher | func_800527C8 TRANSLATED (49 words / 0xC4 at 0x800527C8, live split 42FC8.s, all 49 exe-verified): multi-subsystem bootstrap dispatcher, 17 calls (16 distinct callees, func_8005BC98 twice). Every direct callee is translated. Call 15 is real func_80051CC4; B40 translates its first nested dependency func_8005332C, B41/B42 complete the exposed B29 func_80053968/func_80053B48 dependencies, and B43 translates func_8005218C only through its first honest internal boundary at func_8005B91C. Sole call site func_8006A9E4 @0x8006AAD0, `$s1`-guarded one-shot inside cycle B; void return unconsumed. | `pc_port/game/boot/func_800527C8_port.c`, `func_80051CC4_port.c`, `func_8005218C_port.c`, `func_8005332C_port.c`, `func_80053968_port.c`, `func_80053B48_port.c` |
| Framebuffer SHA-256 | `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` (3 headless + windowed identical, bootstrap and real-disc runs) | `sha256sum` of `--screenshot` PPM |
| Real-disc load trace | PE.IMG lba=1013, size=206213120, load 32 KiB at 0x8010BD00, fnv1a64 `7D860391E1ED6C97`; trace SHA-256 `7b8724acf4d4787f58ca0068e68839f171e2d3f36f72a42f0a4ef03f0041672b` (3 runs identical) | `--disc-image <bin> --disc-load-test --trace` |
| Strict mode | continuing real data: exit 1 at `func_80073CF4` from `func_80076C34`; `--bootstrap-disc`: exit 1 at `func_8007F72C` from `func_800698D4` | retained B49 harness, normal + sanitizer |
| GPU/DMA2 | One private 1024x512x16 VRAM; GPUSTAT bit26; GP0 A0; GP1 00/01/02/04; exact DMA2 block issue; DPCR/DICR channel2; tokenized explicit completion; deterministic VBlank. B53E issues the exact LoadImage CPU prefix/DMA suffix and leaves completion explicit. No retail ring/HostFB/callback alias. I_MASK remains the single 16-bit `pe_irq.[ch]` authority | `pc_port/platform/pe_gpu.[ch]`, `pc_port/platform/pe_irq.[ch]`, `pc_port/docs/b53e_func_80076664.md` |
| Sanitizers | fresh `-DPE_PORT_SANITIZERS=ON`: 506/506 tests; bootstrap, real-data boot/load, and retained harness pass; strict frontiers agree exactly | B53E isolated sanitizer build |
| Matching build | **EXACT SHA-1 MATCH** (227 leaves) via docker `pe-mipsel:trixie` (`dev/mipsel/Dockerfile`) | `docker run --rm -v $PWD:/workspace -w /workspace pe-mipsel:trixie bash scripts/build_us.sh` |
| Next frontier | B53F: recover/translate only callback-registration boundary `func_80073CF4` reached from `func_80076C34`; do not start the queue pump or complete pending DMA merely to advance execution | — |

### Phase 6E-B43 func_8005218C — current findings

`func_8005218C` is classification 1, translated retail
resource-table/record derivation logic, with an architecture-B prefix-only
production implementation. Its complete 155-instruction / `0x26C`-byte
body is at `0x8005218C..0x800523F7` (exclusive end `0x800523F8`, file offset
`0x4298C`), live at `asm/disc1/42664.s:246-406`. The exact top-level ABI is
`void func_8005218C(void)`: all five executable callers pass no arguments and
ignore the residual register value.

The full body has 15 direct calls in retail order: seven unresolved
`func_8005B91C`, seven translated `func_8005DBAC`, and one unresolved
`func_80052F24`. The first `func_8005B91C` receives
`(0, (int32_t)(int16_t)load16(0x800C0E28), sp+0x10, 0)`. Its return is
ignored, but it must write the selected 32-bit table index through `a2`;
instruction `0x800521B8` immediately loads that word for the following
`func_8005DBAC` call. A return-only bootstrap result cannot reproduce that
state effect. Production therefore preserves the exact prefix read and all
four arguments, then stops through the centralized boundary before any
persistent guest or authoritative state is written. Neither dependency is
implemented by B43.

`pc_port/tools/b43_oracle.py` verifies all 155 literal words and all five
caller contexts against the SHA-exact executable, then executes the full
body with controlled dependency output contracts. Ten production tests
bring the normal suite to 417/417 and prove the prefix width, signed
argument, full-width transient pointer, repeated/reset behavior, canary
footprint, five-caller ABI, internal call map, B39 integration, and strict
advancement. Complete caller/dependency/operation/state proof is in
`pc_port/docs/b43_func_8005218C.md`. Fresh ASan/UBSan also passes 417/417;
all 28 oracle programs pass. Three bootstrap, real-boot, and real-disc-load
captures are byte-identical, all canonical hashes/FNV are unchanged, and the
isolated Docker matching rebuild remains an exact executable SHA-1 match.

### Phase 6E-B42 func_80053B48 — historical findings

`func_80053B48` is classification 1, translated retail resource-category
logic. Its complete 121-instruction / `0x1E4`-byte body is at
`0x80053B48..0x80053D2B` (exclusive end `0x80053D2C`, file offset
`0x44348`), live at `asm/disc1/43724.s:940-1082`. The exact ABI is
`int32_t func_80053B48(pe_addr_t record)`. Types 1-7 and 16-18 map to one of
three fixed records at `0x800A1E64 + category*0x20`; all other byte types
return one immediately. It searches the authoritative ID table for
`0x200+category`, installs it in the first zero signed-halfword slot if
absent, accumulates the input record's `+0xA` halfword, and applies the exact
signed threshold/999 cap. A full ID table returns one but does not suppress
the fixed-record update.

There are exactly two callers. `func_80053D2C @ 0x80053E1C` receives the
selected guest record in `$a0` and forwards the return unchanged for record
types 16-18. `func_8005833C @ 0x80058408` also forwards every 32-bit result,
branching only to clear its source mapping halfword when the result is zero.
Controlled `0,1,7,0x7FFFFFFF,0x80000000,0xFFFFFFFF` returns are exact. The
body has no callees or unresolved internal boundary.

`pc_port/tools/b42_oracle.py` verifies all 121 words and both caller tails
before its delay-slot-aware MIPS-I interpreter executes them. Ten production
tests bring normal and fresh ASan/UBSan suites to 407/407; all 27 oracle
programs pass. Three bootstrap, real-boot, and real-disc-load captures are
byte-identical. Framebuffer, bootstrap trace, real-disc trace, FNV, and exact
Docker matching SHA remain unchanged. Detailed proof is
`pc_port/docs/b42_func_80053B48.md`. Normal/sanitizer real-disc strict now
advances to untouched `func_8005218C` from `func_80051CC4`; bootstrap strict
remains `func_8007F72C` from `func_800698D4`.

### Phase 6E-B41 func_80053968 — historical findings

`func_80053968` is classification 1, translated retail resource-record
materialization. Its complete 120-instruction / `0x1E0`-byte body is at
`0x80053968..0x80053B47` (exclusive end `0x80053B48`, file offset
`0x44168`), live at `asm/disc1/43724.s:805-936`. The exact ABI is
`pe_addr_t func_80053968(int32_t resource_id)`. It scans 128 records at
`0x800C0EAC` with stride 32 and the current authoritative ID table for their
first free entries. Success calls translated `func_8005DB44(arg-1)`, copies
32 bytes in two ordered 16-byte load/store groups, commits the primary shared
table state, calls translated `func_80052F70`, writes
`0x100+record_slot`, and returns the exact destination. Either full search
returns zero without persistent writes or calls.

The sole executable caller is `func_80053D2C @ 0x80053DF8`; types 1..9
reach it. The call delay moves the original argument from `s0` to `a0`.
The following jump delay is `sltiu s1,v0,1`, so controlled provider returns
`0,1,0xFFFFFFFF,7,0x800C0EAC` produce caller returns `1,0,0,0,0`. The
provider never normalizes its pointer-or-zero result. On real Disc 1 the
first path is `func_8005CCA4 @ 0x8005CDC8` with argument `0x44`, and its
`func_80053D2C` result is discarded by the immediately following call.

`pc_port/tools/b41_oracle.py` verifies all 120 words, the sole call site,
and the complete 18-word B29 jump table/consumed-return sequence before a
delay-slot-aware interpreter executes them. Ten production tests bring
normal and fresh ASan/UBSan suites to 397/397; all 26 oracle programs pass.
Detailed instruction map, footprints, dependencies,
state authority, and test proof are in
`pc_port/docs/b41_func_80053968.md`. Normal real-disc strict now advances to
untouched `func_80053B48` from `func_80053D2C`; bootstrap strict remains
`func_8007F72C` from `func_800698D4`.

### Phase 6E-B40 func_8005332C verified — historical findings

`func_8005332C` is classification 1, translated retail logic. Its complete
42-instruction / `0xA8`-byte body is at
`0x8005332C..0x800533D3` (exclusive end `0x800533D4`, file offset
`0x43B2C`), live at `asm/disc1/43724.s:331-378`. The exact contract is
`pe_addr_t func_8005332C(int32_t resource_id)`. It rejects negative and
signed-out-of-count IDs, reads one signed halfword from the authoritative
resource indirection table, returns one of two fixed-stride guest records,
or forwards translated `func_8005DB44(entry-1)` for entries `1..255`.
It has no writes, cache, blocking, platform behavior, or unresolved callee.
The dependency audit also corrects `func_8005DB44`'s prior native
misresolution: signed immediates `0x8038/0x8034` address
`0x800A8038/34`, and the surviving register contributes literal
`0x800A8028` to the result. The retained B23 oracle had already modeled
both facts exactly; B40 production tests now reject the former aliases.

`pc_port/tools/b40_oracle.py` independently verifies and executes all 42
words with delay slots, verifies all 46 direct executable call sites plus
the compatible indirect callback ABI evidence, and never calls production
C. Ten retail-derived production tests bring normal and fresh ASan/UBSan
suites to 387/387.
The detailed operation map, all callers, global authority census, exact
footprints, dependency proof, and test scope are recorded in
`pc_port/docs/b40_func_8005332C.md`. Post-translation real-disc strict stops
at pre-existing `func_80053968` from `func_80053D2C`; the corrected shared
lookup makes that B29 boundary reachable before B39. Bootstrap strict remains
`func_8007F72C` from `func_800698D4`.

### Phase 6E-B39 func_80051CC4 verified — historical findings

`func_80051CC4` is classification 1, translated retail logic: a
resource/table command-state initializer. Its complete body is 77
instructions / `0x134` bytes at executable
`0x80051CC4..0x80051DF7` (exclusive end `0x80051DF8`), file offset
`0x424C4`. The live split is `asm/disc1/420A8.s:314-401`, selected by
`configs/USA/disc1.yaml:298` (`[0x420A8, asm]`). The map labels it
nonmatching at `0x80051CC4`; the only current port declaration is the true
`void func_80051CC4(void)` prototype. No matching/nonmatching C body or
SDK-map entry existed before B39; the port path was the centralized stub in
`func_800527C8_port.c`. The SHA-exact executable has SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The complete 77-word body and eight-word jump table at `0x800111F8` are
literal constants in `pc_port/tools/b39_oracle.py`. The oracle compares
every word with the executable before execution, uses one register file,
samples branches at issue, executes every delay slot exactly once, models
32-bit arithmetic and little-endian guest memory, logs ordered reads,
writes, calls and the single void return, and exposes state at both
unresolved boundaries. Its opcode self-tests cover signed `lb`/`slt`,
unsigned `sltiu`, and the MIPS low-five-bit variable-shift rule.

ROM order is exact:

1. Save the exact boolean result of `func_80052F0C()`
   (`D_8009D048 != 0x800C0E48`).
2. Call translated `func_80052E30(0)` while the old `D_8009D018` is still
   visible through `func_80052F70`.
3. Clear authoritative `D_8009D018`, then store zero in descending order
   at `0x800A1B48, 44, 40, 3C, 38, 34, 30`.
4. Sign-extend the byte at `0x800C0E22` and call unresolved
   `func_8005332C(source_id)`. A null return skips the record loop.
5. For a non-null record, read unsigned count at `record+0x14`; for signed
   loop index `i < count`, decode `U8(record+0x15+i) & 0x1F`. Commands
   8/9/10 set `D_8009D018` to 1/2/4; 11 stores 3 at `0x800A1B30`; 12
   stores 2 at `0x800A1B34`; 13 stores `-2` at `0x800A1B44`; 14 is a
   no-op; 15 stores `-2` at `0x800A1B34`; all other values are no-ops.
6. Call unresolved `func_8005218C()`.
7. Call translated `func_80052E30(0)`, then
   `func_80052E30(saved_boolean)`, and return void through the sole return
   path.

The three distinct executable call sites all use `jal` plus a `nop` delay
slot, consume no argument registers, and discard the void return:

- `func_800512AC` at `0x800514C0`, conditional command-12 switch arm.
  Immediately before it, `D_8009D018=4` and `func_80052E30(0)` execute;
  immediately after it, `$v0=2` is stored at `$gp+0x2A0`, proving the
  callee return is overwritten. This event path is repeatable.
- `func_80051E64` at `0x80052170`, unconditional converged epilogue after
  the caller's record update/parser paths. The following instructions load
  `$ra/$s0` and return; no result is consumed. This routine is repeatable.
- `func_800527C8` at `0x80052844`, operation-map position 15 and the 14th
  `jal` instruction. The preceding retail
  operation is translated `func_8005D6F4`; the following operation is
  translated `func_80042C78`. The dispatcher does not consume the return.
  `func_8006A9E4` invokes the dispatcher at most once per streaming run via
  its `$s1` guard, although explicit dispatcher invocations remain
  repeatable.

Before B39, the dispatcher-local unresolved sequence was
`func_80051CC4`, then `func_80042C78`. After B39 every direct dispatcher
callee is translated; the first transitive unresolved sequence is now
`func_8005332C`, then `func_8005218C`, both from `func_80051CC4`.
`func_8005332C` has a proven signed 32-bit argument and guest-address
return but its `0xA8`-byte body is not begun. `func_8005218C` has a void
contract and a `0x26C`-byte body with broad fan-out, also not begun. In
strict mode the prefix through the seven clears is committed before the
central policy exits at `func_8005332C`; no return is fabricated and no
function-specific bypass exists.

The direct persistent guest-write footprint is exactly the 28-byte range
`0x800A1B30..0x800A1B4B`, as seven aligned 32-bit stores. Retail also has
the ordinary transient ABI frame saves at `incoming_sp-8` and
`incoming_sp-4`, with matching reloads before return; the independent
oracle models and logs them, while native C uses the host ABI stack. Fixed guest
reads include the signed byte at `0x800C0E22`; translated
`func_80052E30` can read the byte at `0x800C0E0C`. Data-dependent record
reads span an absolute checked range from `0x80000014` through
`0x801FFFFF`, with one-byte width only. Thus fixed/data guest reads span
`0x80000014..0x801FFFFF`, and fixed persistent guest writes span
`0x800A1B30..0x800A1B4B`; transient stack bounds are caller-SP-relative.
Every guest state access uses checked
`PE_LoadU8`/`PE_StoreU32`; address zero remains invalid. There is no
hardware, BIOS, Psy-Q, GPU, DMA, MDEC, SPU, controller, disc, timer,
event or interrupt access; no callback; no multiplication/division;
no unaligned access; no poll/loop other than the bounded record scan; and
no blocking in the translated body. The unresolved callees' blocking
behavior is unknown, and current execution is not deterministic from
guest state alone because the `$gp` fields below are authoritative host
globals plus controlled dependency returns.

With retail `$gp=0x8009CD70`, the touched global authority is:

| Retail address | Role and recovered readers/writers | Current authority/reset |
| --- | --- | --- |
| `0x8009D018` (`gp+0x2A8`) | command mask; read by `func_80051E58`/`func_80052F70`; written by `func_800512AC` and B39 | `pe_globals.c` host `uint32_t`; reset only by `PE_Sdk_ResetState` |
| `0x8009D048` (`gp+0x2D8`) | active resource-buffer guest address; read by `func_80052F0C`, `func_80053D2C`, `func_8005CCA4`; written by `func_80052E30`/`func_8005CCA4` | same host authority/reset |
| `0x8009D04C` (`gp+0x2DC`) | saved/reuse buffer; written by `func_80052EB0` and cleared by `func_80052C6C`; read by `func_80052E30` | same host authority/reset |
| `0x8009D050` (`gp+0x2E0`) | resource count/ID; read by `func_80053D2C`; written by `func_80052E30`/`func_8005CCA4` | same host authority/reset |
| `0x8009D054` (`gp+0x2E4`) | saved count/ID; written by `func_80052EB0`; read by `func_80052E30` | same host authority/reset |
| `0x8009D058` (`gp+0x2E8`) | resource-table guest address; written by `func_80052E30`/`func_8005CCA4` | same host authority/reset |
| `0x8009D064` (`gp+0x2F4`) | resource-buffer kind (2 or 4); written by `func_80052E30`/`func_8005CCA4` | same host authority/reset |

Guest words at those retail addresses are deliberately not a second copy;
full-RAM canary tests prove they remain untouched. `PE_RamReset` clears
guest state but preserves these authoritative fields, while
`PE_Sdk_ResetState` clears them. Pointer-shaped values remain 32-bit guest
addresses (`pe_addr_t`); no host pointer is stored or truncated.

B39 adds ten retail-derived production tests, bringing both normal and a
fresh ASan/UBSan suite to 377/377. The B39 oracle and all 23 retained
oracles pass. Normal and sanitizer real-disc strict runs agree on
`func_8005332C` from `func_80051CC4` (exit 1); bootstrap strict remains
`func_8007F72C` from `func_800698D4` (exit 1). Three bootstrap traces,
three real-disc load traces, three bootstrap framebuffers, and three
real-disc boot framebuffers are pairwise identical within each group.
Hashes remain framebuffer `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`,
bootstrap trace `42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af`,
real-disc load trace `7b8724acf4d4787f58ca0068e68839f171e2d3f36f72a42f0a4ef03f0041672b`,
and real-disc FNV-1a-64 `7D860391E1ED6C97`.

### Phase 6E-B33 func_8005E884 verified — current findings

`func_8005E884` is translated retail logic: 4 instructions / 0x10 bytes
at executable `0x8005E884..0x8005E890` (exclusive end `0x8005E894`, file
offset `0x4F084`, yaml segment `[0x4F084, c]`). It loads the signed byte
at `D_800B0DB1` (`lui $v0,0x800B; lb $v0,0x0DB1($v0)`) and returns it.
It has no callees, no guest writes, no SDK/GPU/disc/audio/input activity,
and cannot block. All 4 words are exe-verified against SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The sole writer to `D_800B0DB1` is `func_8006A2E8` (a value-validated
setter: `sltiu $v0,$a1,16; beqz → skip`; stores `$a1` as signed byte
only when `$a1 < 16`; also stores to `0x800C0DFF` and `0x800C0CEA`).
During boot `D_800B0DB1` is unwritten BSS, so the retail return is 0.

Six executable call sites:
- `func_8005D6F4` @ `0x8005D8F0` → r; `func_8005E850(0, 8-r)` (nop slot)
- `func_8004B5A4` @ `0x8004B5C0` → `sw $v0,628($gp)`; epilogue
- `func_8004B5DC` @ `0x8004B5F0` → `subu a0,a0,v0`; `func_8005FCAC(8-r)`
- `func_8004B6CC` @ `0x8004B6CC` → `lw $gp+0x274`; `subu a1,a1,v0`;
  `func_8005E850(0, stored-r)`
- `func_8005C150` @ `0x8005C300` → `sb $v0,0x800C0DFF`
- `func_8005C310` @ `0x8005C414` → `lb a1,0x800C0DFF`; `subu a1,a1,v0`;
  `func_8005E850(0, stored-r)`

Classification: 1 — translated retail logic (trivial leaf). The returned
integer is the signed byte at `D_800B0DB1`, representing the alarm timer
value set by `func_8006A2E8`.

Independent oracle `pc_port/tools/b33_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; every modeled
word cross-checked against the exe; every read logged with address,
width, value and order; asserts exact reads-only footprint and return
for unwritten-BSS, positive, negative, boundary, and repeated scenarios.

The strict frontier advances to **func_8005E850** from `func_8005D6F4`
(three identical captures, exit 1). The D6F4 boundary now has 3 remaining
unresolved callees in retail ROM order: `func_8005E850`, `func_800649D0`,
`func_80052790`. The dispatcher oracle still reports two unresolved callees
in order: `func_80051CC4`, `func_80042C78`. Native and fresh ASan/UBSan
suites are 349/349. Framebuffer (`fb28dc21…`), bootstrap trace
(`42c1956e…`), and real-disc load trace (`7b8724ac…`, FNV
`7D860391E1ED6C97`) hashes are unchanged. Nothing has been pushed and the
next rung has not started.

### Phase 6E-B34 func_8005E850 verified — current findings

`func_8005E850` is translated retail logic: 13 instructions / 0x34 bytes
at executable `0x8005E850..0x8005E880` (exclusive end `0x8005E884`, file
offset `0x4F050`, live split `[0x4F050, c]`). All 13 words exe-verified
against SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Non-leaf void wrapper: reads D_800B0DB0 (signed byte, always 0 — never
written in the retail executable) and D_800B0DB1 (signed byte, alarm
timer set by func_8006A2E8), adds each to the corresponding argument,
and calls func_8006A2E8(a0 + D_800B0DB0, a1 + D_800B0DB1). Stack
frame: addiu $sp,-0x18 / sw $ra,0x10($sp) / lw $ra,0x10($sp) /
addiu $sp,+0x18 / jr $ra / nop.

func_8006A2E8 is UNRESOLVED — it routes through the centralized
bootstrap boundary. Its return value is discarded (no caller of
func_8005E850 ever reads $v0).

Four executable call sites:
- func_8004B61C @ 0x8004B684 — a0=0, a1=1; nop slot; ret discarded
- func_8004B674 @ 0x8004B6DC — a0=0, a1=lw(gp+0x274)-r; subu slot
- func_8005C310 @ 0x8005C428 — a0=0, a1=lb(0x800C0DFF)-r; subu slot
- func_8005D6F4 @ 0x8005D900 — a0=0, a1=8-r; subu slot

Classification: 1 — translated retail logic (thin wrapper) with
unresolved callee on the centralized boundary.

Independent oracle `pc_port/tools/b34_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; every modeled
word cross-checked against the exe; every read logged; delay-slot
argument computation verified; asserts exact call arguments and
no guest writes for all five scenarios.

The strict frontier advances to **func_8006A2E8** from `func_8005E850`
(three identical captures, exit 1). The D6F4 boundary now has 2 remaining
unresolved callees in retail ROM order: `func_800649D0`, `func_80052790`.
The dispatcher oracle still reports two unresolved callees in order:
`func_80051CC4`, `func_80042C78`. Native and fresh ASan/UBSan suites are
354/354. Framebuffer (`fb28dc21…`), bootstrap trace (`42c1956e…`), and
real-disc load trace (`7b8724ac…`, FNV `7D860391E1ED6C97`) hashes are
unchanged. Docker matching rebuild remains EXACT at
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Nothing has been pushed and
the next rung has not started.

### Phase 6E-B35 func_800649D0 verified — current findings

`func_800649D0` is translated retail logic: 30 instructions / 0x78 bytes
at executable `0x800649D0..0x80064A44` (exclusive end `0x80064A48`, file
offset `0x551D0`). All 30 words exe-verified against SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Stores the argument at D_8009D16C ($gp+0x3FC). If a0 is zero:
bzero(0x800A3060, 0x120) via REAL func_80071A24, then stores 0xFF to
eight specific bytes: 0x800A3078, 0x800A30A0, 0x800A30B0, 0x800A30B8,
0x800A30C0, 0x800A30C4, 0x800A3124, 0x800A3134. If a0 is non-zero:
returns immediately after the state store. No other callees. Void return.

Three executable call sites:
- func_8004AF38 @ 0x8004B0EC — a0=func_80063428 ret; addu slot
- func_8005C310 @ 0x8005C450 — a0=lb(0x800C0DFF)&1; andi slot
- func_8005D6F4 @ 0x8005D908 — a0=0; addu slot

Classification: 1 — translated retail logic (resource-state reset).

Independent oracle `pc_port/tools/b35_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; bzero modeled
from the proven A(28h) contract; every word cross-checked; asserts exact
footprint for both the zero and nonzero paths.

The strict frontier advances to **func_80052790** from `func_8005D6F4`
(three identical captures, exit 1). The D6F4 boundary now has 1 remaining
unresolved callee: `func_80052790`. The dispatcher oracle still reports
two unresolved callees: `func_80051CC4`, `func_80042C78`. Native and
fresh ASan/UBSan suites are 358/358. Framebuffer (`fb28dc21…`),
bootstrap trace (`42c1956e…`), and real-disc trace hashes are unchanged.
Docker matching rebuild remains EXACT at
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Nothing has been pushed and
the next rung has not started.

### Phase 6E-B36 func_80052790 verified — D6F4 chain complete

`func_80052790` is translated retail logic: 9 instructions / 0x24 bytes
at executable `0x80052790..0x800527B0` (exclusive end `0x800527B4`, file
offset `0x42F90`). All 9 words exe-verified against SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Stores the argument at D_8009D020 ($gp+0x2B0), converts it to a
boolean (a0 < 1 → 1, else 0) in the jal delay slot, and calls
func_80086728(bool) through the centralized bootstrap boundary. Stack
frame with $ra save/restore. func_80086728 is UNRESOLVED. Void return.

Three executable call sites:
- func_8004AF38 @ 0x8004AFEC — a0=func_80063428 ret; addu slot
- func_8005C310 @ 0x8005C438 — a0=lb(0x800C0DFF)&3; andi slot
- func_8005D6F4 @ 0x8005D910 — a0=1; addiu slot

Classification: 1 — translated retail logic (thin wrapper).

Independent oracle `pc_port/tools/b36_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; asserts exact
state store, boolean conversion, call arguments, and write footprint
for all four scenarios.

**The func_8005D6F4 dependency chain is now complete.** All callees are
REAL: func_80071A24 (B21), func_8005DC4C (B26), func_80052594 (B27),
func_8005CCA4 (B28), func_800614AC (B32), func_8005E884 (B33),
func_8005E850 (B34), func_800649D0 (B35), func_80052790 (B36). The
remaining bootstrap boundary stubs come from func_8005E850's callee
func_8006A2E8, func_80052790's callee func_80086728, the dispatcher's
func_80051CC4, and func_8006A9E4's func_80087090. Native and fresh
ASan/UBSan suites are 362/362. Framebuffer (`fb28dc21…`), bootstrap
trace (`42c1956e…`), and real-disc trace hashes are unchanged. Docker
matching rebuild remains EXACT at
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Nothing has been pushed and
the next rung has not started.

### Phase 6E-B37 func_8006A2E8 verified — current findings

`func_8006A2E8` is translated retail logic: 12 instructions / 0x30 bytes
at executable `0x8006A2E8..0x8006A314` (exclusive end `0x8006A318`, file
offset `0x5AAE8`). All 12 words exe-verified against SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Conditional setter: if a1 < 16, stores a1 as halfword to 0x800BCE9E
and 0x800BCE8A, stores a1 as signed byte to D_800B0DB1, and returns 0
(retail jr delay slot: `addu $v0,$zero,$zero`). If a1 >= 16, returns 0
without writing. The first argument (a0) is ignored.

Sole call site: func_8005E850 @ 0x8005E86C (delay slot: addu $a1,$v1,$a1;
return discarded by all callers of 5E850).

Classification: 1 — translated retail logic (conditional leaf).

Independent oracle `pc_port/tools/b37_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; asserts exact
conditional stores, threshold boundary, return value, and a0-independence
for six scenarios.

The strict frontier advances past func_8006A2E8 to the next unresolved
provider. The remaining bootstrap boundary stubs are func_80086728 (from
func_80052790), func_80051CC4 (from func_800527C8), and func_80087090
(from func_8006A9E4). Native and fresh ASan/UBSan suites are 367/367.
Framebuffer (`fb28dc21…`), bootstrap trace (`42c1956e…`), and real-disc
trace hashes are unchanged. Docker matching rebuild remains EXACT at
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Nothing has been pushed and
the next rung has not started.

**Window-white is a known host-layer artifact:** the X11 window background
is white and Expose events are not re-blitted; the port blits once after
boot returns.  The guest framebuffer remains `fb28dc21…` (near-black).
Do not report the white window as a retail frame.

### Historical B30 func_80042C78 verified

`func_80042C78` is translated retail logic through its proven prefix:
executable `0x80042C78..0x80042CB4`, exclusive end `0x80042CB8`, file
offset `0x33478`, 16 instructions. It writes zero to `$gp+0x168/0x170/0x174`,
writes `0x20` to `$gp+0x16C`, calls translated `func_80042CC4` with
`a0=0x90` and `a1=0xFF` after the delay slot, then writes `0x48` to
`$gp+0x17C`. With `$gp=0x8009CD70`, the direct guest footprint is
`0x8009CED8`, `0x8009CEDC`, `0x8009CEE0`, `0x8009CEE4`, and `0x8009CEEC`.
The B30 oracle independently transcribes, SHA-verifies, and executes all
16 words, including the delayed call arguments and exact prefix state.

There are two executable callers: `func_8005CCA4` at `0x8005CFF8` after
its final direct stores, and `func_800527C8` at `0x8005284C` after
`func_80051CC4`; both discard the void return and have a nop delay slot.
Normal and repeated calls preserve the same prefix ordering. The final
suite was 339/339 normal and 339/339 ASan/UBSan. Real-disc strict now stops
at `func_800614AC` from `func_8005D6F4`, exit 1, consistently in normal and
sanitizer runs. Bootstrap strict remains `func_8007F72C` from
`func_800698D4`, exit 1. Nothing has been pushed and the next rung has not
started.

### Historical Phase 6E-B31 func_80042CC4 verified

`func_80042CC4` is translated retail logic: 31 instructions / `0x7C` bytes
at executable `0x80042CC4..0x80042D3C` (exclusive end `0x80042D40`, file
offset `0x334C4`, live split `asm/disc1/334C4.s`). It is a void leaf with
arguments `(a0, a1)`. It clears `0x800A1878`, shifts the color base in the
initial branch delay slot, and fills subsequent bytes while signed `lbu < a1`
holds. The final count `(cursor - 0x800A1878) + 1` is stored at
`0x8009CEE0` (`$gp+0x170`). The B30 call `(0x90, 0xFF)` produces the retail
ramp prefix `00 90 CF EA F6 FB FD FE FF` and count 9.

Its direct write footprint is the termination-dependent subset of bytes
`0x800A1878..0x800A1887` plus word `0x8009CEE0`; reads are current ramp bytes.
It has no direct callees, SDK/GPU/disc/audio/input activity, blocking, host
pointers, low-address mirror, or clamping. Executable call sites are
`func_80042C78 @ 0x80042C98` (a0=0x90, delay-slot a1=0xFF) and
`func_8005D2B4 @ 0x8005D5E8` (delay-slot a0=`$s0`); both discard the void
return. Incoming
a2/a3 are overwritten before use. `pc_port/tools/b31_oracle.py` verifies the
executable SHA-1, all 31 words, delay slots, ordered writes, threshold paths,
and count using its independent transcription.

The final native and fresh ASan/UBSan suites were 341/341. Real-disc strict
then stopped at `func_800614AC` from `func_8005D6F4`; bootstrap strict remained at
`func_8007F72C` from `func_800698D4`. Nothing has been pushed and the next
rung has not started.

### Phase 6E-B32 func_800614AC verified — current findings

`func_800614AC` is translated retail logic: 36 instructions / `0x90` bytes
at executable `0x800614AC..0x80061538` (exclusive end `0x8006153C`, file
offset `0x51CAC`, live split `asm/disc1/51CAC.s`). It masks the input to
24 bits, stores that word at `0x8009D14C` (`$gp+0x3DC`), computes the three
pairwise arithmetic means of the packed color bytes with the exact retail
saturation branches, stores the packed result at `0x8009D150` (`$gp+0x3E0`),
and returns that result. It has no callees, reads no guest memory, cannot
block, and has no GPU/SDK/disc/audio/input activity. For D6F4, the call at
`0x8005D8C8` is unconditional: `$a0` is built as `0x00404040` by the
`ori` delay slot, the preceding stores are `sh 0x0203 → 0x800C1F80` and
`sw 0x00404040 → 0x800C0E44`, the return is discarded, and the following
operations clear `0x800A76A4/B0/BC/C8`. The D6F4 unresolved sibling order
after B32 is `func_8005E884`, `func_8005E850`, `func_800649D0`,
`func_80052790`.

The other executable call sites are `func_800434C0 @ 0x80043564` and
`0x8004358C`, `func_8005C374 @ 0x8005C3C0`, `func_8004B394 @
0x8004B440`, `0x8004B494`, and `0x8004B504`, and `func_8004FEEC @
0x8004FF10`; their return values are overwritten or otherwise discarded.
The B32 oracle independently verifies the executable SHA-1 and all 36 words,
executes delay slots and branches from its transcription, and checks exact
ordered stores and returns. Native and fresh ASan/UBSan suites are 344/344.
Real-disc strict now stops at `func_8005E884` from `func_8005D6F4`; bootstrap
strict remains at `func_8007F72C` from `func_800698D4`. Nothing has been pushed
and the next rung has not started.

### Historical B29 func_80053D2C verified — accepted commit `eedd456`

`func_80053D2C` is translated retail logic: executable
`0x80053D2C..0x80053E6B`, exclusive end `0x80053E6C`, file offset `0x4452C`,
80 instructions. It scans `D_8009D048/D_8009D050` for the first zero
halfword, calls translated `func_8005DB44`, dispatches record types 1–18,
and performs the proven exact halfword store/return behavior. Types 1–9
route `func_80053968`; types 16–18 route `func_80053B48`. Both were
centralized unresolved integer providers at B29; B41 and B42 now complete
them, and B42 corrects the latter ABI to `$a0=selected record`. The B29 oracle independently
transcribes, SHA-verifies, and executes all 80 words with delay slots.

Executable call sites are preserved: `func_8005CCA4` calls at
`0x8005CDC8/0x8005CDD0/0x8005CDD8/0x8005CDE0/0x8005CDE8` with constants
`0x44/0x96/0x3F/1/6`, plus conditional index calls at
`0x8005CED0` and `0x8005CFB8`; other callers are
`func_80021D4C` (`0x80021D8C`), `func_80022394` (`0x80022410`),
`func_800236E8` (`0x8002381C`), `func_8005112C` (`0x800511B0`),
`func_80044444` (`0x80044538`), `func_800194B0` (`0x800194CC`), and
`func_8005D020` (`0x8005D184`). Their delay slots and return consumers were
audited; no caller requires a fabricated return or bypass.

The B29 suite was 337/337 normal and 337/337 ASan/UBSan. Real-disc strict
now stops at `func_80042C78` from `func_8005CCA4`, exit 1, consistently in
normal and sanitizer runs. Bootstrap strict remains
`func_8007F72C` from `func_800698D4`, exit 1. Nothing has been pushed and the
next rung has not started.

### Historical B28 func_8005CCA4 verified — key findings (corrective commit)

**Zero loop range (CORRECTED):** retail loads `$gp+0x2D8` (= D_8009D048 =
0x800C0E48, set by this rung) and adds 0x62 (`addiu v1,v1,0x62`) → the
descending zero loop writes 50 halfwords covering **0x800C0E48..0x800C0EAA**,
NOT 0x800C0DE6..0x800C0E48. It does NOT overlap the earlier GA_E06/E08/E0C/
E24/E28..E34 stores (all below 0x800C0E48) — those SURVIVE. GA_E24 stays 1;
GA_E22 is written 1 unconditionally (retail `beq` delay slot). GA_E40
(0x800C0E40, below the loop) receives its u16 0x3D once, after the loop.
An earlier draft dropped the +0x62 and descended from 0x800C0E48; that
zeroed the wrong 100-byte region. Now executed word-by-word by the oracle.

**gp base + shared state (CORRECTED):** gp = 0x8009CD70 (proven via
func_800438C0's 0x180(gp)=D_8009CEF0).  The four state words are
$gp+0x2D8/2E0/2E8/2F4 = **D_8009D048/D_8009D050/D_8009D058/D_8009D064**
(an earlier draft used a +0x10-shifted gp).  func_8005CCA4 writes the SAME
host globals func_80052C6C uses — one authoritative storage, no host/guest
duplicate.  Values: D_8009D048=0x800C0E48, D_8009D050=func_80052F70(),
D_8009D058=0x8009D05C, D_8009D064=2.

**First loop (CORRECTED):** stores `*(u16*)func_8005DB8C(i)` (retail
`lhu 0(v0)`), not the low half of the returned address.

**Fixture correction:** FxPattern offsets 0x10-0x17 return 0 (retail BSS
state for D_800A8038/D_800A803C). The original fixture wrote canary data
there, causing func_8005DBAC to dereference a garbage pointer
(0xA49D968F + 0x800A8028 = 0x24A816B7). The function was correct; the
fixture was wrong.

**Split-brain:** the 8 func_80052C6C state globals (D_8009D018/D_8009D03C/
D_8009D048/D_8009D04C/D_8009D050/D_8009D054/D_8009D058/D_8009D064) live in
pe_globals.c (extern via psx_compat.h), reset by PE_Sdk_ResetState.
func_8005CCA4 now shares those globals directly (never guest RAM), so the
four it writes are unified with func_80052C6C — verified by
test_5CCA4_no_split_brain (host values correct, aliasing guest words stay
canary, PE_RamReset does not clear them, PE_Sdk_ResetState does).

**Oracle transcription:** func_8005DB8C is 8 words; 0x8005DB90 is `addiu`
(sign-extended 0x8038 → 0x800A8038), not `lw`. func_8005DBAC uses the `j`
delay slot (addu $v1,$zero,$zero) for negative clamping. func_800438C0
stores before the zero-check branch (sw, bne, addiu, sw).

**Test count:** 335/335 (normal + ASan/UBSan).

**B28 provenance:** provisional implementation `a1559ae`; oracle/bootstrap
corrective `cd2e375`. **B28 oracle (CORRECTIVE):**
`pc_port/tools/b28_oracle.py` — independently
transcribes all 223 func_8005CCA4 words (W_5CCA4), cross-checks every word
against the SHA-verified exe at load time (first mismatch fails with address
and both values), then executes the transcribed words with a delay-slot-aware
MIPS-I interpreter. func_8005DB8C/func_8005DBAC/func_800438C0 are also
cross-checked word-by-word and executed. func_8005CCA4 execution uses the
transcription (not the exe) for its own words; sub-callees read from the exe.
The body is executable `0x8005CCA4..0x8005D01F` (exclusive end
`0x8005D020`), 223 retail words, file offset `0x4D4A4`. Boundary funcs
func_80053D2C/func_80042C78 are stubbed. Asserts the retail final
state (zero loop 0x800C0E48..0x800C0EAA, GA_E24=1, GA_E22=1,
D_8009D048=0x800C0E48, D_8009D058=0x8009D05C, D_8009D064=2, GA_E40=0x003D,
D_8009CEF0=0x3D).

**Bootstrap-disc fixture (CORRECTIVE):** `func_800698D4_port.c` now seeds a
minimal valid archive at D_800A8028 in the bootstrap-disc path (R=0x30,
S=0x14, count=120, entry[30] → lone 0xFF record).  On real hardware this
data arrives from PE.IMG via cycle A of func_8006A9E4; the fixture
establishes the same precondition so func_8005DC4C returns `0x800A8400`
instead of 0. Address zero remains invalid; no KUSEG or low-address mirror
exists, no clamping, and no function-specific bypass was added. With
`PE_StoreU32(0x800A803C, 0xA49D968F)`, malformed-header
`func_8005DBAC(0)` returns `0x24A816B7`; that malformed result is not
dereferenced. Normal and ASan/UBSan native tests are 335/335. Normal
bootstrap-disc completes with 15 bootstrap stubs invoked; bootstrap strict
exits 1 at `func_8007F72C` from `func_800698D4`, and real-disc strict exits 1
at `func_80053D2C` from `func_8005CCA4`. Framebuffer SHA-256 is
`fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`;
bootstrap trace begins `42c1956e…`, real-disc trace begins `7b8724ac…`,
real-disc FNV-1a-64 is `7D860391E1ED6C97`, and matching executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

### B22 audit complete

`func_8005DE88` is translated as a 23-word, no-callee resource-list/state
initializer. It links the 12-byte records at `0x800A2090..0x800A2174`,
null-terminates `0x800A2174`, and initializes `$gp+0x36C..0x380`. The current
strict frontier is `func_80052C6C` from `func_800527C8`, captured with exit
status 1. The dispatcher oracle now reports five unresolved callees in order:
`func_80052C6C`, `func_8005BCBC`, `func_8005D6F4`, `func_80051CC4`,
`func_80042C78`. Bootstrap-disc remains intentionally stopped at
`func_8007F72C`. Native and sanitizer tests are 271/271; LSAN leak detection
requires `LSAN_OPTIONS=detect_leaks=0` in this ptrace-restricted environment.

The independent oracle `pc_port/tools/b22_5de88_oracle.py` is a
delay-slot-aware MIPS-I interpreter executing the verified retail words
(SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`); it cross-checks the
transcription against the retail exe bytes, then records and validates all
27 guest writes in retail ROM order.

### B23 audit complete — func_80052C6C translated

`func_80052C6C` (113 words / 0x1C4 at 0x80052C6C, live split 43408.s) is
translated as a resource-table search + init rung. Coupled callees translated
in the same file: func_80052E30 (resource-buffer init/reuse, 31 words),
func_80052EB0 (two-word state setter, 3 words), func_80052F0C (buffer-identity
comparison, 5 words), func_80052F70 (capped-add allocator, 22 words),
func_8005DB44 (32-byte record-table lookup, 17 words). func_80051E58 is the
pre-existing C leaf (2 words). The strict frontier advances to **func_8005BCBC**
from `func_800527C8`; the dispatcher oracle now reports four unresolved
callees in order: func_8005BCBC, func_8005D6F4, func_80051CC4, func_80042C78.

Independent oracle `pc_port/tools/b23_oracle.py` — a delay-slot-aware MIPS-I
interpreter executing the verified retail words (SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`) for func_80052C6C and all coupled
callees, asserting every guest write, the $GP-state writes, and the exact
internal call order. **Addressing-mode finding:** func_8005DB44's base/alt-base
pointers are loaded via `lui 0x800B ; addiu 0x8038` — the 16-bit immediate
0x8038 has bit 15 set, so MIPS sign-extends it and the real access is
0x800A8038 / 0x800A8034, NOT 0x800B8038 / 0x800B8034. The oracle models the
hardware faithfully (reads `0x800A8038`). The original B23 C port and test
fixture incorrectly used `0x800B8038`; B40 corrected production to
`0x800A8038/34` after the shared real-disc lookup exposed the divergence.
Current B40-B42 tests poison the obsolete `0x800B...` bytes only as inert
canaries and prove they are never semantic inputs. The B23 oracle has always
modeled the retail addresses correctly.

**B23 oracle corrective (Phase 6E-B26 mandatory pre-audit).** The B26
interpreter audit proved the committed b23_oracle.py materially
defective and corrected it in a separate commit (history preserved, no
amend): (1) two word transcription errors in W_80052C6C, never
cross-checked against the exe — 0x80052CC8 was byte-swapped residue
0x06004290, retail word is 0x90420006 (lbu $v0,6($v0), the search
record-byte load); 0x80052DC8 was 0x14C0FF44 (branch to an
out-of-function address), retail word is 0x1440FFCB (bne $v0,$zero,-53
— the main loop's i<9 back-edge; the corrupted word removed the loop);
(2) interpreter semantics errors: branch conditions evaluated after
the delay slot, not-taken branches re-executed their delay slot, and
callees ran on fresh register files without argument propagation.
Under the corrected words + corrected MIPS-I semantics the scenario
produces: D_8009D03C = 2 (unchanged), db44 called 11 times (a0
sequence 0,1 | 1,2,3 ×3), return 0x800C1F7E, and ALL NINE output
records receive byte[9]=0 and halfword[18]=999 plus the full 32-byte
record copy from the seeded source records.  The retired "rec0-only"
assertion was an artifact of the defects.  The corrected oracle
cross-checks every modeled word against the SHA-verified exe at load
time, runs seven interpreter self-tests (delay-slot single execution
taken/not-taken, non-idempotent register and memory slot effects,
next-PC target/PC+8, jal/jr slots, shared register file), and asserts
the exact 409-write ROM-order footprint.  The production C port is
unchanged by this corrective (its 9-iteration main loop already
matches the retail structure; its db44 literal-address divergence
remains documented); the ctest suite remains the C-port authority.

**Leaf-count reconciliation (227 vs 229).** This checkout's committed yaml at
base `71114ac` has **227** C leaves (`grep -cE ',[[:space:]]*c,'
configs/USA/disc1.yaml`) and builds EXACT SHA-1. The **229** figure in the
Phase 6D-R docs came from the sibling checkout
`/home/blizz/dev/parasite-eve` (branch `phase5fm-main-barrier-revisit` @
`7467308`), whose *uncommitted* working tree promotes `func_8006E9A0` (5FJ)
and `func_8006E834` (5FK) to C leaves; that state also builds EXACT SHA-1
(docker, "Matching claim: YES (229 C leaves)"). Both are exact because C
leaves are byte-exact asm replacements — the count is conversion progress,
not output content. Documentation now reports the count of the checkout it
lives in.

`D_80011614` is a `pe_addr_t` guest pointer (bootstrap-policy value
`0x8010BD00`; no translated retail writer yet).  The boot-time
`func_8006E6D4(D_800B0DD8 + 0, 0, D_80011614, 0)` from `func_8006E834` is
degenerate on retail too (D_80093164 is unwritten BSS), so the
`--disc-load-test` driver is the deterministic proof of the real byte
path.  `func_8001220C`'s disc-wait loop is retail-corrected
(`while (func_800698D4() != 0)`; a Phase 6D-S port bug had it inverted);
without a disc the run ends via the established stop-at-first-present
adaptation plus a bounded-wait backstop, never a faked mount.

### B24 audit complete — func_8005BCBC translated

`func_8005BCBC` (21 words / 0x54 at 0x8005BCBC, live split 4C4BC.s) is
translated as a resource-state pointer/count selector rung.  ROM order:
`sw $a0 → 0x358($gp)` (D_8009D0C8) always first; then a0 != 0 selects
via `lbu 6($a0)` (== 9 → 0x800C20B4, else 0x800C20A4) or a0 == 0 selects
via `lw 0x4A8($gp)` (D_8009D218 != 0 → 0x800C0DF0, else 0x800C0DE0);
final stores `sw $a1 → 0x350($gp)` (D_8009D0C0) and `sw $v0 → 0x354($gp)`
(D_8009D0C4 = 8); returns 8 on every path.  Exactly two exe call sites
(jal word 0x0C016F2F): `func_800527C8` @0x80052834 (a0 = 0 in the delay
slot, immediately after jal func_80052C6C @0x8005282C) and
`func_8004DD64` @0x8004DF28 (a0 = lw 0x294($gp) = D_8009D004; off the
boot frontier); both discard the return.  No callees, no SDK/GTE/GPU/
MDEC/SPU/disc/input activity; deterministic; idempotent; PE_RamReset
restores initial conditions (post-reset flag 0 → 0x800C0DE0).  The three
state words are guest-RAM resident — exe-wide $gp-access scan shows
retail readers/writers in func_8005BD10/func_8005BE1C (same split) and
the func_8005D6F4 region (stores at 0x8005D730-38 / 0x8005D7F4-F8), all
via $gp-relative guest accesses (B22-style storage decision).  The
dispatcher call reads D_8009D218 = 1 that func_8005BC98 stores (guest
RAM cross-function dependency, asserted in tests).

Independent oracle `pc_port/tools/b24_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words (branch
conditions sampled at issue, before the delay slot executes; shared
register file; every read/write logged with width and order).  It
asserts the exact ROM-order footprint and return for all four selection
paths: (a0=0,flag=1) → C0=0x800C0DF0; (a0=0,flag=0) → C0=0x800C0DE0;
(record,byte6=9) → C0=0x800C20B4; (record,byte6≠9) → C0=0x800C20A4.
The B23 oracle remains green after the Phase 6E-B26 corrective
(hardware-faithful contract: all nine records updated under the seed;
see the corrective note in the B23 audit section).

The strict frontier advances to **func_8005D6F4** from `func_800527C8`
(three identical captures, exit 1); the dispatcher oracle now reports
three unresolved callees in order: func_8005D6F4, func_80051CC4,
func_80042C78.  Native and sanitizer tests are 279/279.  Framebuffer
(`fb28dc21…`), boot trace (`42c1956e…`), and real-disc load trace
(`7b8724ac…`, FNV `7D860391E1ED6C97`) hashes are unchanged; windowed
framebuffer matches headless.

### B25 audit complete — func_8005D6F4 translated

`func_8005D6F4` (147 words / 0x24C at 0x8005D6F4, live split 4CC98.s,
file 0x4DEF4) is translated as a resource-buffer + display-state
initializer rung.  ROM order: REAL `func_80071A24(0x800C0DE0, 0x12E4)`
bzero (A(28h), B21 contract); flag `D_8009D218 = 1`; block-1 state
stores in order C8(=0), C0(=0x800C0DF0), C4(=8); fill loop #1
(0xFF × 8 at 0x800C0DF0, retail reloads C0/C4 from guest RAM every
iteration); selection #1 — `lw C8` reads the 0 just stored, so the
func_8005DC9C arm is statically dead (preserved structurally) and
`func_8005DC4C(0x1E, a1=0xFF)` is called — retail sets the copy dest
`$a1 = D_8009D0C0` only AFTER the call returns (addu $a1,$s0 is not a
delay slot; proven by the oracle's register-level execution); string
copy #1 from the func_8005DC4C return into D_8009D0C0 until the copied
byte == 0xFF (dest cursor advances per byte including the terminator);
block 2 re-selects 0x800C0DF0 with store order flag, C4, C8, C0 and
re-fills the buffer — retail overwrites copy #1's bytes, reproduced;
selection #2 + copy #2; third `func_8005DC4C(0x1E, a1=cursor-after-
copy-2)`; `func_80052594(ret)`; `func_8005CCA4()`; `sh 0x0203 →
0x800C1F80`; `sw 0x00404040 → 0x800C0E44`; `func_800614AC(0x00404040)`;
timer-tick clears 0x800A76A4/B0/BC/C8; `func_8005E884() → r`;
`func_8005E850(0, 8−r)`; `func_800649D0(0)`; `func_80052790(1)`;
terminator bytes 0xFF at 0x800C20A4/0x800C20B4; returns 0xFF.  Sole
exe call site: func_800527C8 @0x8005283C (nop delay slot, a0 carries
the residual 0 from func_8005BCBC's delay-slot setup — the body never
reads $a0); return discarded.

Boundary (as of B26): func_80071A24 and func_8005DC4C are REAL; the
SEVEN remaining callees route through the centralized boundary in
retail ROM order (7 provider invocations non-strict: func_80052594,
func_8005CCA4, func_800614AC, func_8005E884, func_8005E850,
func_800649D0, func_80052790; the func_8005DC9C arm is dead).  The B25
degenerate in-RAM default for the func_8005DC4C return is RETIRED — the
copy source now comes from the real archive, and tests establish the
archive precondition instead of scripting a provider return.
Idempotence: state-reproducible (every store overwrites); PE_RamReset
restores initial conditions, and also clears the guest-resident archive
that cycle A re-populates on a real boot.

Independent oracle `pc_port/tools/b25_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; branch
conditions sampled at issue BEFORE the delay slot executes (the B24
oracle was audited and carries the same fix); shared register file;
exact little-endian widths; every read/write logged with address,
width, value, order; bzero modeled by the proven A(28h) contract; the
nine unresolved callees are recorded with their register-state
arguments but NOT executed.  Asserts the exact ROM-order footprint and
return for the degenerate boot path AND a controlled 4-byte string
return (seeded outside the bzero range — the seed region is zeroed
first, same as on hardware).  NOTE: while building B25 the committed
b23/b24 oracles' shared interpreter pattern was audited; b24 received
the not-taken `pc += 2` advancement fix (idempotent-safe slots — results
unchanged, still PASS), b23 was left untouched per the B23 rung
directive (its assertions encode its committed corrected contract).

The strict frontier advanced INSIDE the translated func_8005D6F4 to
**func_8005DC4C** (three identical captures, exit 1) — superseded by
B26, which translates func_8005DC4C and moves the frontier on to
func_80052594.  The dispatcher oracle reports two unresolved callees in
order: func_80051CC4, func_80042C78.  Native and sanitizer tests were
285/285 at B25.  Framebuffer
(`fb28dc21…`), boot trace (`42c1956e…`), and real-disc load trace
(`7b8724ac…`, FNV `7D860391E1ED6C97`) hashes are unchanged; windowed
framebuffer matches headless.

### B26 audit complete — func_8005DC4C translated

`func_8005DC4C` (20 words / 0x50 at `0x8005DC4C..0x8005DC9B`, file
`0x4E44C`, live split `4CC98.s:1779-1802`, yaml segment `[0x4CC98, asm]`)
is translated as a read-only PE.IMG message/string-table lookup.  Exactly
one body exists in the tree (one `glabel`, no matching or nonmatching C
source); all 20 words are exe-verified.

```
ptr = mem32[0x800A802C] + 0x800A8028
tbl = ptr + mem32[ptr + 4]
return (idx <u mem16[tbl]) ? tbl + sext16(mem16[tbl + 2 + 2*idx]) : 0
```

**Three corrections to the pre-audit B26 working state, all proven from
the raw bytes.** (1) `0x8005DC7C` is `sll $v0,$a0,1` (0x00041040 → sa=1),
so the record stride is **2**, not 16.  (2) the `j` delay slot at
`0x8005DC8C` is `addu $v0,$v1,$v0` (0x00621021 → rd=v0, rs=v1, rt=v0), so
the return is **tbl + off**, NOT `a1 + off`.  (3) the exe-wide `jal
0x0C017713` scan finds **39 call sites in 24 callers**, not three.  Under
the corrected decode every return is an ordinary guest-RAM address, so
the KUSEG RAM mirror the pre-audit state had added to `pe_guest_ram` —
which made every address below `0x80000000` valid, including the proven
failure return 0 — was unnecessary and was removed.  No clamping, no
sentinel, no function-specific bypass.

Signature is ONE argument: no instruction reads `$a1`.  Values retail
leaves in `$a1` at call sites are residual (in func_8005D6F4 the 0xFF
fill constant; at `0x8004685C` the jal delay slot is `sb $v0,0($s1)`).
Three guest reads on the failure path, four on the success path, ZERO
guest writes, no callees, cannot block, deterministic from guest state
alone, repeated calls stable.  Zero is retail's own out-of-range return
(`addu $v0,$zero,$zero`); no call site anywhere compares the return to
zero.  Observed constant indices across all sites run 3..117.

Measured in guest RAM at the func_8005D6F4 call on the real Disc 1 USA
image (probe, not port output): `R = 0x30` → ptr `0x800A8058`;
`S = 0x14` → tbl `0x800A806C`; count `0x78` (120); entries `242..1971`
→ records `0x800A815E..0x800A881F`.  `min(entry) == 2 + 2*count` exactly,
so the record pool begins where the offset table ends — an independent
confirmation of the stride-2 layout.  Index 30 (used by all three
func_8005D6F4 sites) → `0x800A82A9`, bytes `10 48 30 FF`.  The
independent oracle reproduces `0x800A82A9` from the retail words alone.

The archive is guest-resident and arrives from PE.IMG via cycle A of
func_8006A9E4 before the dispatcher runs.  With the region zeroed the
lookup returns 0 and func_8005D6F4's copy loop dereferences address 0 —
surfaced by the checked-access layer, deliberately not masked.  Tests
therefore establish the same precondition the real boot does
(`B26_SeedArchive`), and the fixture PE.IMG now carries a valid archive
header at the offsets cycle A copies.

Independent oracle `pc_port/tools/b26_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; seven
interpreter self-tests (taken/not-taken slot single execution,
non-idempotent register and memory slot effects, next-PC target vs PC+8,
j/jr slots, shared register file); every modeled word cross-checked
against the exe; every read/write logged with address, width, value and
order; 36 checks over empty, populated, immediate-terminator, failure,
first/last, repeated, `$a1`-independence, signed/wraparound, dirty,
PE_RamReset and return-address boundary scenarios (including a return
that legitimately leaves guest RAM and is NOT clamped).

The strict frontier advances to **func_80052594** from `func_8005D6F4`
(three identical captures, exit 1) — NOT to func_80051CC4, because
func_8005D6F4 still carries seven boundary callees that all precede the
dispatcher's remaining two.  The dispatcher oracle still reports two
unresolved callees in order: func_80051CC4, func_80042C78.  Native and
sanitizer tests are 298/298.  Framebuffer (`fb28dc21…`), boot trace
(`42c1956e…`), and real-disc load trace (`7b8724ac…`, FNV
`7D860391E1ED6C97`) hashes are unchanged; windowed framebuffer matches
headless.  Docker matching rebuild remains EXACT at
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

### B27 audit complete — func_80052594 translated

`func_80052594` (22 words / 0x58 at `0x80052594..0x800525EB`, file
offset `0x42D94`, live split `42D94.s`, yaml segment `[0x42D94, asm]`)
is translated as a leaf string-copy function.  All 22 words are
exe-verified against the SHA-1-verified retail executable.

ROM-order operation map (no `$gp` usage — all addresses via absolute
`lui`/`addiu`):
1. `$a1 = D_80091694` (buffer base), `$v1 = $a1 + 8` (buffer end)
2. `sltu $v0,$a1,$v1` — always true (8-byte buffer); `beqz` → skip
3. `$a2 = 0xFF` (terminator constant)
4. Loop: `lbu $v0,0($a0)` / `nop` (load delay) / `beq $v0,$a2 → exit`
   / `nop` / `sb $v0,0($a1)` / `addiu $a1,1` / `sltu $v0,$a1,$v1`
   / `bnez → loop` / `addiu $a0,1` (delay slot)
5. Exit: `$v1 = D_8009169D` / `$v0 = $a1 - D_80091694` = byte count
6. `jr $ra` / `sb $v0,0($v1)` (delay slot: store count at D_8009169D)

Signature: `int func_80052594(pe_addr_t src)`.  One argument in `$a0`.
Returns byte count (0-8) in `$v0`.  Side effect: stores count at
`D_8009169D`.  The `0xFF` terminator byte is NOT copied.

Five executable call sites (`jal` word `0x0C014965`):
- `func_8005D6F4` @ `0x8005D898` (delay: `addu $a0,$v0`; return discarded)
- `func_8004DD64` @ `0x8004E28C` (delay: `addu $a0,$v0`; return discarded)
- `func_8004DD64` @ `0x8004E428` (delay: `addu $a0,$v0`; return discarded)
- `func_8004DD64` @ `0x8004E6A8` (delay: `addu $a0,$v0`; return discarded)
- `func_8005C46C` @ `0x8005C46C` (delay: `nop`; return discarded)

All call sites discard the return value.

`D_80091694` (8-byte buffer) and `D_8009169D` (1-byte count) are
guest-RAM resident.  Exhaustive executable-wide `lui`/`addiu` scan found
NO readers — these are write-only stores.  Not `$gp`-relative (`$gp` =
`0x8009CD70`, offset would be `-0xB6DC`, outside small-data area).

Classification: 1 — translated retail logic (leaf, no callees, pure
guest-memory, deterministic, idempotent for same source).

Independent oracle `pc_port/tools/b27_oracle.py` — delay-slot-aware
MIPS-I interpreter over the SHA-1-verified retail words; every modeled
word cross-checked against the exe; every read/write logged with address,
width, value and order; asserts the exact ROM-order footprint and return
for terminator-only, partial, full, and boundary scenarios.

The strict frontier advances to **func_8005CCA4** from `func_8005D6F4`
(three identical captures, exit 1) — NOT to func_80051CC4, because
func_8005D6F4 still carries six boundary callees that all precede the
dispatcher's remaining two.  The dispatcher oracle still reports two
unresolved callees in order: func_80051CC4, func_80042C78.  Native and
sanitizer tests are 309/309.  Framebuffer (`fb28dc21…`), boot trace
(`42c1956e…`), and real-disc load trace (`7b8724ac…`, FNV
`7D860391E1ED6C97`) hashes are unchanged; windowed framebuffer matches
headless.  Docker matching rebuild remains EXACT at
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Decomp state (main repo)

| Fact | Value | Derive |
| --- | --- | --- |
| Branch / tip | `main` @ tip (227, synced) | `git branch --show-current` / `git log --oneline -1` |
| Phase | **5FI-62a34 / 227 exact leaves** (62CE4 parked loop-layout, sixth skew instance; main v5 + atom at stash; cc1 archaeology gates boot-to-black) | `scripts/verify_us.sh` summary + exact rebuild |
| Matching C leaves | **227** | `grep -c ',\s*c,' configs/USA/disc1.yaml` |
| Yaml asm segments | **152** | `grep -c ',\s*asm\]' configs/USA/disc1.yaml` |
| Era leaf compiles | **70** | `grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh` |
| Target SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | `scripts/build_us.sh` compare |
| Progress | https://blizz127.github.io/parasite-eve-progress/ | `scripts/publish_progress.sh` |

**Yaml `asm` segments are not remaining functions.** One segment can hold
dozens of glabels; do not subtract it from anything as a function count.

Oracle: bare `scripts/build_us.sh` exits 0 on exact SHA-1; `scripts/verify_us.sh`
reports Phase 5FE-table-2f970 / 224. Disc images / `asm/` / `build/` / `tools/era/`
are git-ignored inputs — never commit them.

**Toolchain**

- Default leaves: GCC 14.2 in Distrobox `pe-mipsel` (Phase 4J flags; selective
  `-G 8` / `-fno-delayed-branch` / `-fno-tree-ter`).
- Era leaves (opt-in): `scripts/setup_era.sh` → `era_compile` =
  cpp → cc1 → maspsx → GNU as, typically `-O2 -G0` (some leaves `-O1 -G0`).
- Era maspsx: `ERA_ASPSX_VER=2.21` + `--dont-expand-li`. **Why:**
  `expand_load_immediate` turns positive small `li` into `ori`; ROM wants
  `addiu`. Defer `li` expansion to GNU as. Same config also preserves
  large-literal `lui;ori` (cc1 emits PSY-Q `li` high + `ori` low natively).
  Do **not** bump aspsx-version casually — that also flips `nop_at_expansion`
  / `addiu_at`.
- **Vendored maspsx LOCAL PATCH:** `tools/era/maspsx/maspsx/__init__.py` is
  repo-tracked (`.gitignore` negations; `setup_era.sh` re-clones upstream
  AROUND it, restores the tracked file from git if absent). Patch 1 =
  **sw-store delay-slot fill**, opt-in per `era_compile` line via env
  `MASPSX_FILL_STORE_DELAY_SLOT=1`: an absolute `sw $r,SYM` macro immediately
  before a bare `j $31` is emitted as `lui $at,%hi` / `j $31` /
  `sw $r,%lo($at)`. sw only — sb/sh macro stores and multi-store epilogues
  are ROM-proven to stay pre-jr with a nop slot (e.g. func_8003FFAC vs
  func_8007FBC0: identical C shape, different ROM scheduling — the original
  units were assembled with different ASPSX scheduling).
- Patch 2 (`f0b9155`) / Patch 3 (`439c244`): **three-word indexed symbolic
  store AND load expansion**, opt-in per leaf via `MASPSX_THREE_WORD_SYMBOL_STORE=1`.
  Standalone `op $r,SYMBOL($index)` uses the retail/ASPSX-2.30-shaped
  `lui $at,%hi` / `addu $at,$at,$index` / `op $r,%lo($at)` sequence,
  for stores (2) and standalone indexed symbolic loads (3: lb/lbu/lh/lhu/lw/lwl/lwr;
  `lwc2` stays outside, durable negative test). Compound semicolon lines retain the
  2.21 four-word expansion. Flag-off rebuild is the exact leaf-count retail SHA.
  Full 224-leaf regression (flag OFF and flag ON over the three existing 3W store
  leaves) both exact; 153 vendored tests; re-clone restore byte-identical.
- Maspsx stdin: closed with `</dev/null` in `era_compile` (non-TTY hang under
  agent sockets). Bare `scripts/build_us.sh` is fine.

## How to count (do not hand-maintain)

```bash
grep -c ',\s*c,' configs/USA/disc1.yaml            # C leaves
grep -c ',\s*asm\]' configs/USA/disc1.yaml          # yaml asm segments (NOT fn count)
grep -c '^era_compile \|^\w*=1 era_compile ' scripts/build_us.sh  # era leaf compiles
git log --oneline -1
```

**Do not** count `asm/disc1/*.s` from disk. That tree is git-ignored and
contains orphans, stale duplicates, and nop-pads. **Yaml is the source of truth.**
Known stale orphans (counter ignore-list): `2E7D0.s` (superseded by live
`2E7D8.s`) and `807C.s` (stale duplicate of live `2A0C.s`; unreferenced in
yaml). Keep both out of function scans.

**asm/ sync invariant:** `$at` family totals from
`tools/analysis/at_absolute_store_counter.py` hard-fail (no SUMMARY) when asm/
is missing units or still holds glabels for yaml C leaves. Re-split with
`scripts/split_us.sh` before planning off a family count. Leaf count stays
yaml-only and still works when asm/ is stale.

## Proven era fingerprints (evidence, not claims)

| Fingerprint | Status |
| --- | --- |
| `move` → `addu` in delay slot | Proven 5EA / 5EB / 5EC / 5ED |
| `$v0` / `$v1` allocation | Proven 5EC / 5ED (sb+ret0 reuse) |
| `li` const materialization (`addiu` not `ori`) | Proven 5EC via `--dont-expand-li` |
| `$at` absolute `sw` macro expansion | Proven by scratch probe; integrated exact in 5EE |
| Branch delay-slot constant hoist (`beqz` slot) | **PROVEN** (5EG-first-branch): era cc1 `-O1 -G 8` reproduces the retail schedule on `func_8004F448` word-for-word |
| Test-and-clear-return if/else (`bnez` + j-over) | **PROVEN, VOLUME** (5ER): era `-O2 -G0` matches the adjacent byte/word twins `func_80038D1C` / `func_80038D48` — shared address in `$v1`, `addu $v0,$zero,$zero` in the `bnez` slot, `addiu $v0,$zero,0xFF` in the unconditional-jump slot, then `sb`/`sw` clear. Direct-global C rebuilt the address and used a 12-word `beq` form; one natural explicit-pointer phrasing retry matched all 11 words without pinning |
| `$a0`-in/`$v0`-out + redundant double store | **PROVEN** (5EH): era `-O2 -G8` preserves both stores + `addu` return-0 on `func_800438C0`; GCC 14.2 `-O1` merges stores and emits `move` — **era required for value-returning leaves**; era+gp `-G8` first proven here |
| Non-leaf stack frame + `jal` | **PROVEN** (5EI; repeated as volume in 5EK): era matches the `func_800197D0` / `func_800197F0` void-callee twins — `addiu $sp,-0x18` / `sw $ra,0x10($sp)` / `jal`+nop / `lw $ra` / `addiu $v0,1` / `jr $ra` with the `addiu $sp,+0x18` teardown **in the `jr` delay slot**, word-exact; 197F0 uses `-O2 -G0` and adds no primitive |
| Outgoing `$a0` + `jal` after double dereference | **PROVEN** (5EJ-outgoing-arg): era `-O2 -G0` on `func_80019484(int **)` emits `lw $v0,0($a0)` / load-delay nop / `lw $a0,0($v0)` / `jal func_800438C0` + nop, then the proven return-1 frame teardown shape; all 11 words exact |
| Return-forwarded `$v0` + teardown-before-`jr` epilogue | **PROVEN** (5EL-return-forwarding): era `-O2 -G0` on `func_8007F7A8` emits the frame + `jal func_8007FCAC` + nop, forwards `$v0` untouched, then `lw $ra`; `addiu $sp,+0x18`; `jr $ra`; nop. Era reproduces this per-function schedule as well as 197D0/F0's opposite teardown-in-slot schedule |
| Straight-line boot pointer-layout scheduling | **PROVEN, COMPILER-CONSTRAINED C** (5EM-boot-6a8d4): era `-O2 -G0` matches all 68 words / 19 absolute pointer stores in retail order. Both the initial plain-local source and one retail-order retry allocate cursors to `$a0/$a1`, constants to `$v0/$v1`, and sink `D_800B0E28` past `D_800B0E2C/E30`. The exact fallback therefore uses the established explicit-register convention (`$v0/$v1` cursors, `$a0/$a1` constants); it is target-specific matching C, not portable natural C |
| Counting-loop back-edge scheduling | **PROVEN; VOLUME-ELIGIBLE** (5EN/5EP `func_8006A674` probe): era `-O2 -G0` puts pointer advances in all five retail back-branch delay slots — `bnez` up-counters (`$a0+4`, `$v1+2`, `$a1+8`) and `bgez` down-counters (`$a3-4`, `$v0-4`) — and preserves the final store in the `jr` delay slot. The leaf remains parked for unrelated constant-hoist scheduling; the loop primitive passed. |
| Natural counting loop in volume | **PROVEN, VOLUME** (5ES `func_8004BF08`): era `-O2 -G0` matches a natural pointer-walk loop over parallel signed `int[8]` arrays in all 14 words, with no pins or maspsx opt-in. Explicit initialization in retail order (`i`, first pointer, second pointer) plus `do/while` phrasing gives `$a1/$a0/$v1` allocation; the first pointer advances before the bound test and the second pointer advances in the backward `bnez` delay slot. The declaration-initialized `for` form was semantically correct but allocated the three live values differently. |
| Pure-register bit-serial loop in volume | **PROVEN, VOLUME** (5ET `func_8005186C`): era `-O2 -G0` matches all 15 words on the first natural-C try — no loads/stores, calls, or `$gp`; explicit-init `do/while`; the unconditional `result <<= 1` fills the forward `bnez` skip-branch delay slot, the `bgez` back-edge keeps a nop slot, and the return lands as `addu $v0,$a1,$zero` in the `jr` delay slot |
| Indexed global-array store/load expansion | **PROVEN, TOOL-SOLVED** (`f0b9155` stores; `439c244` loads): per-leaf `MASPSX_THREE_WORD_SYMBOL_STORE=1` reproduces `lui` / indexed `addu` / op `%lo` and removed the extra L3 word in `func_8006A674` (153→152 words). `439c244` extends the gate to standalone indexed symbolic LOADS (all seven widths; `lwc2` stays outside — durable negative test; compound lines retain the 4-word expansion). Default off is byte-identical. |
| `lui;ori` large-literal synthesis | **PROVEN** (capability probe): both bit15-clear and bit15-set; cc1 emits PSY-Q `li` high + `ori` low; ROM-exact under 2.21 + `--dont-expand-li` |
| Rotated/peeled loop idiom | **PROVEN SHAPE** (5EV `func_80052BCC`, leaf parked on unrelated allocation): write the first iteration explicitly, then `while (cond) { body }` → era `-O2 -G0` emits the rotated shape: `beq`-exit head, bottom-tested `bne` back-edge, pointer advance in both delay slots |
| Signed `char` vs 0xFF-range constant | **PROVEN SHAPE** (5EV `func_80052BCC`, same parked leaf): signed `char c` compared against `0xFF` emits the conversion `andi` on the compare path even after `lbu`; `unsigned char` does not. Typing controls the mask |
| Return-accumulator vs direct-return phrasing | **PROVEN (5FH `func_80037548`)**: a search loop with a default return value must hold the result in an ACCUMULATOR (`signed char result = 0; ... result = v; break; return result;`). Direct `return v;` on the match path makes cc1 emit a SEPARATE `addu $v0,$zero,$zero` default path before `jr` (28 words vs ROM's 27) — the accumulator keeps one `$a2` merge with the `sll/sra` sign-extension pair hoisted to the merged exit |
| `-fschedule-insns2` load-delay `li` hoist | **PROVEN, FIRST LEAF** (5EW `func_80052BCC`, era `-O1 -G0 -fschedule-insns2`): the post-allocation scheduler hoists an independent `li` above `sb`/`andi` into the `lbu` delay — the exact spot retail's ccpsx scheduled it. At plain `-O1` the same `li` emits after the `andi` (14/15). Paired phrasing: two `0xFF` consts of different modes (u8 head const dies at the guard → loop re-materializes into the freed `$v1`; `int` loop byte → mask-free raw `bne`); comparing the loop byte against a *variable* or both consts sharing a mode cross-jumps/CSE-shares head and loop |
| sched2 scope (negative result) | **NARROWED (5EY `func_8003E610`)**: `-fschedule-insns2` is NOT a universal retail fingerprint — it governs **store-adjacent `li`/`addiu` placement and load-delay hoists** only (52BCC head-`li`, 6A674's 21 order swaps). Straight-line `jal`-arg scheduling (`$a0` hoisted + `$a1` in slot for two-arg calls; `$a0` slot-filled single-arg; nop slot no-arg) is already correct at plain `-O2`. Do NOT flip sched2 into the era default |
| dbr_sched `$v0`-steal screening rule | **CHARACTERIZED (5FB `func_800698D4`, PARKED)**: a `beqz`/`beq` whose delay-slot steal candidate is a `$v0`-setter gets the fill when the branch target hits a `jal` immediately (kills `$v0`), but retail DECLINES the steal when the target is the return-computation block (`$v0` live to `jr $ra`) — our cc1 steals anyway. Screening rule: nop slot + `$v0`-constant load on fall-through + branch to a RETURN block → expect divergence; same pattern to a `jal`-adjacent block → matches. reorg.c liveness skew (ccpsx vs 2.7.2-psx), not source-expressible |
| Nested-if defeats range-test collapse | **PROVEN IDIOM (5FB `func_800698D4`)**: `v != 0 && v != -1` folds to `addiu $v0,$v0,1; sltiu $v0,$v0,2; bnez` under era `-O2` (range test, not retail's shape). Two nested `if`s keep the separate `beqz`/`beq` compares. -O1 keeps compares but flattens other structure |
| Frame-size arithmetic for struct locals | **PROVEN (5FB `func_800698D4`)**: size opaque locals from the frame, not the type's rounded size — DsSearchFile's CdlFILE local is `0x18` (pos 4 + size 4 + name 16): `0x10` args + `0x18` local + `$s0` + `$ra` = frame `0x30`. A `0x20` local emits frame `0x38` and fails at word 0 |
| Five-arg call (o32 stack arg) | **PROVEN, FIRST LEAF** (5FC `func_8006E834`, PARKED on unrelated residual): the 5th argument emits `sw $v0,0x10($sp)` in the `jal`'s delay slot — plain C `f(a,b,c,d,e)` with an immediate 5th arg, era `-O2 -G0`, worked first try. `sb $v0,0x29($sp)` (struct byte field) also lands in a `jal` slot |
| Frame decomposition before writing | **PROVEN METHOD (5FB/5FC)**: decompose the frame BEFORE choosing local sizes — `args + locals + saves + pad = frame` must be exact (5FB: CdlFILE `0x18` not `0x20`; 5FC: args `0x18` + env `0x18` + local30 `0x8` + regs `0xC` + pad `0x4` = `0x48`, byte field lands at `env[0x11]` = `0x29($sp)`). Wrong local size fails at word 0 |
| Aggregate element type as addressing-mode lever | **PROVEN (5FD `func_8002F9CC`)**: for an indexed store into a symbol array, declaring the real aggregate element (`SlotRecord D_800A5D58[]`, `arr[i].field = 0`) makes cc1 emit the standalone indexed symbolic store (`sw $0,SYM($3)`) at plain `-O2` — flat `arr[i*55] = 0` instead hoists `la $5,SYM` out of the loop (invariant under `-O2`/`-O1`/`-O1 -fschedule-insns2`; an addressing choice, not scheduling). With the symbol store present, `MASPSX_THREE_WORD_SYMBOL_STORE=1` passes it to GNU as for retail's 3-word `lui $at / addu / sw %lo($at)` form. Also: a lone symbol materialization is NOT an `-O1` signal — the `-O1` lever is for *repeated* constant/address materialization |
| `-O1` per-use constant materialization — SELECTION RULE | **PREDICTIVE (three leaves)**: if ROM materializes the same constant/address more than once, try `-O1` FIRST. `-O2`'s shared hoist runs through the hardwired `optimize>1` path (not flag-reachable); `-O1` re-materializes per use. 6A674 (discovered: per-use `-1`), 6A5BC (applied: `$s0=1` twice), 3E680 (predicted from five per-store `lui`s with a shared `0x8009` high half retail didn't CSE) |
| Return-use readiness of asm callees | **VALIDATED (5EZ `func_8006A5BC`)**: a caller may USE a still-asm callee's return and stay matchable when the use is a **raw full-width compare** (`beq $v0,$s0`, no mask/sign-extend) or a **bare store** (`sh $v0`). Both are codegen-determined regardless of the callee's true return type, so `int f(void)` externs suffice. Extends the 5EY rule (immediates-only args, returns ignored) |
| Fn-ptr arg to still-asm callee | **PROVEN, FIRST LEAF** (5FA `func_8003E680`): `f(func_8003E91C)` emits `lui $a0,%hi(sym)` / `addiu $a0,$a0,%lo(sym)` with R_MIPS_HI16/LO16 relocs against a same-segment TEXT symbol; the linker resolves it exactly like a data symbol. Declare `extern void g(void);` and pass the bare name |
| Unsigned loop-bound compare | **PROVEN (5FA `func_8003E680`)**: ROM `sltiu` (unsigned) vs cc1's `slt` for `int i < const` — declare the counter `unsigned int`. One-word type-driven fix, no flag involvement |

All four fingerprints from the original 5EA era claim are now proven in bytes.
The “~290 era-blocked functions” figure remains an **ESTIMATE**, not a countdown.

## Known-open families

- **sb+ret0:** **done** in 5ED (family closed).
- **`$at` absolute-store population:** counter committed
  (`tools/analysis/at_absolute_store_counter.py`). The historical integration
  inventory was **18 pre-jr** / 14 delay-slot / 5 sb-sh; the current yaml-live
  population is **0 pre-jr** / **0 delay-slot** / 5 sb-sh. Weak-int policy **NO**.
  - **Pinned by 5EG-readers:**
    - `D_8009D240` = `unsigned short *`, `D_8009D260` = `unsigned char *`
      via `func_8008AB1C` (era `-O1 -G0`).
    - `D_800A1870` = `void (*)(void)` via `func_80042B6C` (era `-O2 -G0`).
  - **Integrated:** `func_80085728`; 5EI readers-typed trio; 5EJ `D_8009D28C`
    int-state (4); 5EK `D_8009D270` unsigned flags (2); **5EF all 14
    delay-slot `sw` members**. The pilot `func_8007FBC0` plus the remaining 13
    typed leaves are integrated exact. Current leaf count **217**.
  - **Delay-slot shape: FAMILY CLOSED (5EF).** Vendored maspsx LOCAL PATCH
    (`MASPSX_FILL_STORE_DELAY_SLOT=1`) fills the `j $31` slot with the trailing
    absolute `sw`. Pilot gate exact + objdump-probed (`3C01800A 03E00008
    AC2436A0`). The remaining 13 members now have per-global typing evidence,
    and all 14 members pass the full exact-match gate; see
    `docs/ai_context/PHASE5EF_TYPING.md`.
  - **sb-sh-five: RECLASSIFIED — never tool-blocked.** ROM words show sb/sh
    macro stores stay **pre-jr with a nop slot** (func_80033A2C sb,
    func_800C6ED8/C6EE8 sh, func_800C6EC0 dual-sh; func_8001A374 has a
    cc1-filled `li` slot). Current maspsx already emits that shape; the patch
    deliberately does not touch sb/sh. Remaining work is typing + integration,
    toolchain-independent.
  - **Still open (typing):** remaining opaque-word (`D_800A1868` other writers).
- **`lui;ori`:** **CAPABILITY-VERIFIED** — not a blocker. Constant-heavy
  computational functions (mult/div/mask, e.g. ÷100 via `0x51EB851F`) are
  approachable as a **separate future phase**; synthesis itself is solved.
- **gp arena loop `func_80055724`:** **PARKED-SCHEDULING** (branch
  `phase5eu-gp-loop-55724`; closest candidate stashed as `park phase5eu
  func_80055724 while-form 13-15`). Empty 8-byte frame **solved** (cc1 2.7.2
  `vars=8` home slots, natural). Blocker: three-way scheduling tension —
  while-form keeps frame+regs but hoists the cursor load above the `blez`
  guard (13/15); if+for keeps frame+regs but duplicates the guard and steals
  the prologue into its slot; if+do/while gets word order but `vars=0` and
  flipped regs. era `-O1 -G8` output is **byte-identical** to `-O2` for both
  leading phrasings — no per-function `-O` support from this leaf. Residual is
  scheduling, not proven allocation. Detail: `docs/ai_context/parked_blockers.json`.
- **disc mount `func_800698D4`:** **PARKED-SCHEDULING** (branch
  `phase5fb-boot-698d4`; closest candidate stashed as `park phase5fb
  func_800698D4 nested-ifs 140-141 (search3 beqz-slot residual)`). Disc
  identification/mount — clears the mount flag, verifies drive ready, searches
  for `\FMV1\PEDISC01.IDF;1` / `\PE.IMG;1` / `\FMV2\PEDISC02.IDF;1` via
  `DsSearchFile`, records via `func_80080C48` → `D_800B0DD8` + `D_800B0DCD`
  flag bits. 140/141 words; everything exact except ONE delay-slot steal:
  search #3's `beqz` (`0x5A24C`) — retail nop, ours steals `addiu $v0,$zero,-1`.
  Mechanism is the dbr_sched `$v0`-liveness screening rule (fingerprint table);
  not source-expressible. Banked idioms: nested-if defeats range-test collapse;
  CdlFILE local is `0x18` not `0x20` (frame arithmetic). Detail:
  `docs/ai_context/parked_blockers.json` (`boot-698d4-dbr-sched`).
- **post-mount loader `func_8006E834`:** **PARKED-ALLOCATION** (branch
  `phase5fc-boot-6e834`; candidate stashed as `park phase5fc func_8006E834
  89-91 (call-result register-home residual)`). Post-mount image loader +
  display env: reads a `D_80093164` lhu offset/size pair from the mounted
  image base `D_800B0DD8` (written by parked 698D4 — the two are producer/
  consumer), polls `func_800811E4`, then `VSync(0)`/`SetDispMask(0)`/
  `func_800749D8(&env,0,0,320,240)` (PROBABLE SetDefDrawEnv)/`PutDispEnv`.
  89/91 content words at era `-O2 -G0`. PROVEN firsts: five-arg call (5th
  arg `sw $v0,0x10($sp)` in the `jal` slot); frame decomposition method.
  Retail FOLDS the `r==0||r==-1` range test in this unit (698D4's did not —
  per-TU compile-settings datapoint). Residual: poll result homed in `$v0`
  by retail (two restores) vs `$v1` by ours — call-result register-home skew,
  not source-expressible. `$v0`-liveness rule NOT exercised (non-event).
  Detail: `docs/ai_context/parked_blockers.json` (`boot-6e834-register-home`).
- **flag-clear loop `func_800374E8`:** **PARKED-ALLOCATION, register COLORING**
  (branch `phase5ff-374e8`; candidate stashed as `park phase5ff func_800374E8
  (register-coloring skew; structure correct)`). Flag-clear loop over 4 x 56-byte
  records at `D_800BCEA8` — **RECORD TYPE ESTABLISHED** (durable deliverable;
  propagates to `func_80037548`): +0x00 `unsigned char` (lbu/sb), +0x0C
  `unsigned int` flags (lw/sw; bit 0x02000000 cleared here), +0x10 `signed short`
  (lh/sh); extent closes EXACTLY at +0xE0 = 4 x 56. **STRUCTURE CORRECT**: 5FD
  aggregate-subscript rule (no `rec` pointer) + the landed load gate (`439c244`)
  produce retail's 3-word indexed-symbolic shape (no `la` hoist, correct DAG and
  scheduling). **RESIDUAL — register coloring only**: era cc1 assigns
  mask->`$v0`/chain->`$v1`/value->`$v0`; ROM is mask->`$v1`/chain->`$v0`/value->`$v1`.
  Five phrasings x two loop forms x ladder rungs are ALL byte-identical —
  invariant under phrasing. Same class as 6E834's call-result home:
  hard-register-assignment skew. This leaf MOTIVATED the maspsx load-gate patch.
  Detail: `docs/ai_context/parked_blockers.json` (`register-coloring-374e8`).
  **TWIN FALSIFIED (5FH)**: `func_80037548` was probed and MATCHES 27/27
  (accumulator shape) — no coloring skew. Refined rule: coloring skew is
  LIVE-VALUE-PRESSURE DEPENDENT (374E8: mask+chain+value all live;
  37548: needle/accumulator/index in $a0/$a2/$a1 leave $v0/$v1 free), not
  per-table. Predict skew only when 3+ scratch values compete.
- **sentinel walk `func_80062CE4`:** **PARKED-SCHEDULING, loop-LAYOUT**
  (do/while form stashed as `park func_80062CE4 (loop-layout scheduling;
  do/while lever proven source-invariant)`). Sentinel validate-and-consume
  over the D_8009D154 list: if D_8009D160 (pending) is still linked, promote
  it to D_8009D15C (confirmed); clear D_8009D160 either way. 12/18; PROVEN
  source-invariant — both while-form and do/while produce BYTE-IDENTICAL
  output; ROM has sentinel-at-top->advance->null-back-edge. cc1
  canonicalizes loop body order before block layout. SIXTH skew instance.
  CARVE CORRECTION: spimdisasm 0x5C label OVERSHOOTS — active span 0x48;
  trailing 5 words are func_80062Fxx prologue. Postmortem: dual gp-four
  filters could not catch loop-layout skew (no pre-compile tell known).
  Detail: `docs/ai_context/parked_blockers.json` (`loop-layout-62ce4`).

- **CC1 PROVENANCE INVESTIGATION — COMPLETE (NULL RESULT):** **no closer community build exists.**
  Four-phase read-only investigation (Phases 1–4) into the era toolchain's cc1, the retail PE1 compiler
  (ccpsx), and whether a closer community build is obtainable. Blinded two GCC MIPS-backend mechanisms
  across the 2.7→2.8 version boundary (loop-body layout via `62CE4`, dbr_sched `$v0`-liveness via `698D4`);
  both survived REORGED (the `reorg.c` rewrite in 2.8 produced identical steal-vs-decline decisions).
  The six parks reflect GCC 2.x MIPS-backend ARCHITECTURE DECISIONS, not version-local divergences.
  FORK: (i) cc1 source patch (the maspsx model one layer deeper — the 698D4 liveness check is scoped)
  or (ii) accept the six residuals as structurally-correct-C with one-word compiler-decision deltas.
  Full report: `docs/ai_context/cc1_investigation.md`. Pipeline reconstructible from the report's
  candidate hashes and `git show stash@{N}^3:path` recovery procedure.

- **`main` (`func_8001220C`, 187 words):** **PARKED-SCHEDULING, WITH COMPLETE CANDIDATE**
  (candidate preserved at stash; five drafting iterations on scratch /tmp/mainvN.c).
  The boot keystone: init sequence, 20-call-site mount/read/dispatch loop, volume gate,
  A8-code three-way state switch. ~180 words match at opcode/position. DURABLE DELIVERABLES:
  the 9-word scratchpad stack handoff (sp → 0x1F8003FC, jal 8019234C, restore) is
  BYTE-EXACT as fenced inline asm with full caller-saved clobbers — the fenced-exception
  mechanism (register-pinning precedent) is validated for when main integrates. Role map
  pinned: $s0 data ptr (D_800B0CD8), $s1 dispatch, $s2 flagbyte (+0xF5), $s3 state_val
  (0xA9400048), $s4 bitmask (0x100000). All 20 externs typed (69B08 int, 1909B4→6E9A0
  raw-flow chain). RESIDUAL — ONE mechanism, proven scheduler-driven by an
  init-placement lever test (draft 4 declared bitmask at top, draft 5 moved init after two
  calls; cc1 kept the li at the same position and the $s-save interleaving identical —
  source cannot express the difference): prologue save-batching + invariant-constant
  placement, cc1 ordering pass vs ccpsx. Fifth scheduling-family instance.
  NOTE: this park joins 698D4 and 6E834 as the THIRD boot-chain function in the skew set —
  the cc1 archaeology now directly gates boot-to-black. Detail:
  `docs/ai_context/parked_blockers.json` (`main-prologue-scheduling`).

- **ccpsx-vs-2.7.2 SKEW SET — four distinct mechanisms:** (1) the
  allocation/scheduling family (`6A674`/`55724`/`52BCC`; two recovered via
  `-O1`), (2) dbr_sched `$v0`-liveness slot-steal (`698D4`), (3) call-result
  register home (`6E834`), (4) register coloring / pseudo-numbering
  (`374E8`). All unreachable from C. (3)+(4) are both register-ASSIGNMENT
  skew, strengthening the case that the single highest-value open lever is
  whether a closer-to-ccpsx cc1 build is obtainable — that would be the
  one-layer-up analog of the maspsx patches and could address the whole set.
  SIX instances now documented (four scheduling, two register-assignment), and THREE are boot-chain functions (`main`,
  `698D4`, `6E834`) — the cc1 question is no longer archaeology for curiosity;
  it directly gates boot-to-black under plan A.
  Do not chase mid-leaf.
- **PARKED-ALLOCATION/SCHEDULING family:** cc1 2.7 register
  allocation/scheduling decisions that natural C cannot steer and `-O` level
  does not change. **FAMILY INVESTIGATED (read-only, accepted): NO SINGLE
  KNOB.** All residuals are present in cc1's **raw** output, pre-maspsx
  (maspsx does only `move`→`addu`, delay-slot nops, the 2.21 indexed-store
  expansion — no reordering/renaming), so a maspsx patch cannot fix any of
  them; the `addiu_at` template does not apply. Pass attribution
  (flag-probed) and current status:
  - `52BCC`: **MATCHED (5EW, leaf 218)** — the `-O1`→`-O2` flip required
    exactly `-fexpensive-optimizations` + `-fschedule-insns2` (regclass +
    post-alloc scheduler; bisection-proven minimal pair). Retried at era
    `-O1 -G0 -fschedule-insns2`: two-const-mode phrasing (u8 head const dies
    at the guard → loop const re-materializes into `$v1`; `int` loop byte →
    mask-free raw `bne`) + sched2 hoisting the head `li` into the `lbu`
    delay = all 15 words exact. First `-fschedule-insns2` leaf.
  - `55724`: pre-reorg RTL emission order (C statement order); NOT
    `dbr_sched` (`-fno-delayed-branch` doesn't move it), `-O`-invariant.
    Retail *sank* the p-load below the guard; 2.7.2-psx has no pass that
    sinks loads past conditional branches. **No lever** — constrained-C or
    acceptance. (Still parked; see entry above.)
  - `6A674`: **MATCHED (5EX, leaf 219)** — the `-O`-sensitive constant
    materialization runs through a hardwired `optimize>1` path (not
    flag-reachable), so `-O1` is the only lever; at `-O1 -G0` the residual
    shrank 45→21 (all pure `li`/`addiu`-before-store order swaps), and
    `-fschedule-insns2` closed them to **0/152** with the 5EP pins intact.
  - **`-fschedule-insns2` is a GENERAL RETAIL FINGERPRINT** (two independent
    leaves, 22 positions): retail's ccpsx ran post-allocation scheduling;
    our default doesn't. Try sched2 early on future scheduling-position
    residuals. Hypothesis to test later (carefully; current leaves match
    without it): sched2 may belong in the era default flag set.
  Evidence: scratch compiles `/tmp/fam_inv` + `/tmp/o1` (session-recorded).
- Complex `$gp` / GTE / BIOS / mult-div / large non-leaves: still open; not
  inventoried here. Path forward is matching real logic, not harvesting
  trivial setters.

## Boot Rung 1 — COMPLETE, climbing `main`'s call chain

```text
main -> func_8006A5BC ✓ exact C (5EZ, leaf 221)   # boot init, VSync waits
     -> func_8006A64C ✓ exact C -> { func_8006A8D4 ✓ exact C,
                                     func_8006A674 ✓ exact C (5EX, leaf 219) }
     -> func_8003E610 ✓ exact C (5EY, leaf 220)   # display/graphics bring-up
     -> func_8003E680 ✓ exact C (5FA, leaf 222)   # subsystem-init dispatcher
```

- `func_8003E680` is **MATCHED (5FA)**: era `-O1 -G0`, all 53 words exact.
  Zero five state globals (Stage-0 reader types: D1C4/D280 unsigned compares,
  D1A0 flags, D250 opaque, CDDC `int` index), 2000-pass poll loop with `i++`
  in the `jal` delay slot (ROM `sltiu` → `unsigned int` counter — the only
  phrasing fix needed), callback registration, ~11 subsystem inits. **First
  fn-ptr-to-asm-callee arg**: `func_80073D24(func_8003E91C)` →
  `lui $a0,%hi` / `addiu $a0,$a0,%lo` with R_MIPS_HI16/LO16 against the
  same-segment text symbol; links exactly. `-O1` predicted by the selection
  rule (five per-store `lui`s, no CSE). Segment-head carve of 2EE80:
  C `0xD4`, resume `2EF54.s` `0x1858`. Next candidates: `func_800698D4`
  (159L, disc mount w/ SDK `DsSearchFile`), `func_8003F3C4` (245L).

- `func_8006A5BC` is **MATCHED (5EZ)**: era `-O1 -G0`, all 36 words exact on
  the first attempt. Four setup calls, two structurally identical
  `while (f() != 1) VSync(0);` loops (VSync = `func_80073A44`, SDK), then
  `func_8007F7A8()`'s return stored to `D_800B0DD4` (`unsigned short`, typed
  by its `lhu` reader). `-O1` reproduces retail's per-use `$s0=1`
  materialization — in `func_80086FF8`'s delay slot AND re-materialized
  between the loops (the 6A674 `-O1` lever, third leaf). Return-use safety:
  both loop conditions compare `$v0` raw (full 32-bit `beq`, no
  mask/sign-extend), so asm callees declared `int(void)` are codegen-safe.
  Identical loop bodies did NOT cross-jump. Mid-55430 carve: prefix
  `0x598C`, C `0x90`, then the three existing boot C leaves — **four
  contiguous C carves, no asm between**. Next candidates up the chain:
  `func_8003E680` (56L, state zeroing + 2000-pass poll + callback
  registration), `func_800698D4` (159L, disc mount w/ SDK `DsSearchFile`).

- `func_8003E610` is **MATCHED (5EY)**: era `-O2 -G0`, all 28 words byte-exact
  on the **first** attempt — no sched2, no pins. Straight-line dispatcher of
  ten calls with immediate args (`0x140`/`0xE0` = 320x224 display res), no
  branches/loops/`$gp`/globals; plain `-O2` reproduces ccpsx's mixed
  arg-load/delay-slot placement exactly. All ten callees are extern-declared
  with call-site-determined signatures (immediate args, no returns used —
  callee bodies don't affect codegen; one already C: `func_80080CC8`).
  Mid-2E7D8 carve: prefix `0x638`, C `0x70`, resume `2EE80.s` `0x192C`.
  Readiness ranking of `main`'s remaining callees (size + callee C/SDK
  coverage) put it first; next candidates in order: `func_8006A5BC` (42L,
  two wait loops + one `sh` global), `func_8003E680` (56L, state zeroing +
  2000-pass poll loop + callback registration).

- `func_8006A674` is **MATCHED (5EX)**: era `-O1 -G0 -fschedule-insns2` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words byte-exact. `-O1` gives
  retail's per-use `-1` materialization (the `-O2` shared hoist is hardwired
  `optimize>1`, not flag-reachable); sched2 places every `li`/`addiu` before
  its adjacent store (21 order swaps). The six semantic pins from the 5EP
  bounded candidate are load-bearing (dropping all six → 46 mismatches).
  Mid-55430 carve fills the 6A64C/6A8D4 gap exactly (0x260); the three boot
  C carves are contiguous.
- `func_8006A64C` matches all 10 words on era `-O2 -G0`: two sequential
  `void(void)` calls, teardown before `jr`, and a nop delay slot. Both
  `R_MIPS_26` relocations resolve at link time; matching a caller requires a
  known callee signature, not that every callee already be C.

## Standing policy

1. **PROBE BEFORE GRIND.** The two biggest unblocks (maspsx stdin hang;
   `expand_load_immediate` forcing `ori`) were short diagnostics, not
   integrations. When a family is blocked, diagnose before more members.
2. **Homogeneous families may be batched.** Risk lives in the first member.
3. **`asm/` is not a source of truth for counts.** Use `configs/USA/disc1.yaml`.
4. **Commit messages are not evidence.** A claim is proven when a gate is green
   and the leaf is objdump-probed (not SHA alone on carves).
5. **No weak-int cheat:** do **not** invent width a narrower store contradicts
   (e.g. `sh`/`sb` → `int`). Distinct from **opaque-word** typing (consistent
   32-bit `sw`/`lw` everywhere) — that is a separate lead ruling, currently
   open under `TYPING-POLICY` in `parked_blockers.json`.
6. **Width-only setters are triaged in `parked_blockers.json`.**
   `READY-FROM-READER` (src reader already *types* it), `BLOCKED-ON-READER`
   (undecompiled reader not yet proven to be a mere use-site),
   `TYPING-POLICY` (opaque 32-bit word; use-site found, no narrowing possible),
   or `DECISION-BLOCKED` (write-only; no reader). A use-site is not a type-site
   (`func_800405A4` lesson). Re-check after every reader phase.
   `5EF-delay-slot` **CLOSED** (14/14 integrated); `sb-sh-five` reclassified
   typing-only.
7. **Register pinning is an evidence-backed fallback, not a shortcut.** Use it
   only after natural C and a retail-order phrasing retry prove that the
   residual is register **allocation**, not statement order. Pins must have
   semantic names and a source comment recording the allocation proof
   (`func_8006A8D4` exact; `func_8006A674` bounded parked example).

## Resolved blockers

- **Phase 5I** delay-slot (`move`/`or` vs `addu`): **SOLVED in 5EC** by era.
- **Maspsx non-TTY hang:** **SOLVED** (`</dev/null` in `era_compile`).
- **`lui;ori` large-literal synthesis:** **CAPABILITY-VERIFIED** (scratch probe;
  both sign cases; no flag change).
- **5EF delay-slot (sw in `j $31` slot):** **CLOSED in 5EF** by the
  vendored maspsx LOCAL PATCH (`MASPSX_FILL_STORE_DELAY_SLOT=1`). Key evidence:
  `func_8003FFAC` vs `func_8007FBC0` — identical C, different ROM scheduling
  (pre-jr+nop vs in-slot) ⇒ original units assembled under different ASPSX
  scheduling; behavior is opt-in per leaf. All 14 members are integrated exact;
  sb/sh never fill (ROM-proven).

## History (append-only, truncated)

| Phase | **224 exact leaves** (tools: maspsx load gate `439c244` on main; parked: 698D4/6E834/374E8) | `scripts/verify_us.sh` summary + exact rebuild |
| --- | --- | --- |
| 4I–4J | 0→1 path | Exact asm rebuild; GCC 14.2 first leaf |
| 5B–5CW | →98 | Empty stubs, getters, store/setter batch |
| 5CX–5DB | →103 | Countdown memset/memcpy (`$2`/`$3` pins) |
| 5DC–5DJ | →156 | `$gp` small-data (`_gp`+`-G 8`); `-fno-tree-ter` |
| 5EA | 157 | Era dual-toolchain; return-0 `addu` |
| 5EB | 161 | Return-0 twins via mid-segment holes |
| 5EC | 163 | sb+ret0; `--dont-expand-li`; 5I dead |
| 5ED | 170 | sb+ret0 batch harvest (family closed) |
| 5EE | 171 | `$at` absolute-`sw` integrated pilot; delay-slot shapes blocked |
| 5EG-readers | 173 | Type-pinning readers `func_8008AB1C` / `func_80042B6C`; `D_800A1870` decl fix |
| 5EG-setter | 174 | `func_80085728` dual-store; first reader-recoverable pre-jr setter |
| 5EH-opaque-word | 182 | u32 opaque-word ruling; 8 A182x setters (`42BD8`…`42C64`) |
| 5EI-ready-from-reader | 185 | READY-FROM-READER setters `42910`/`42B38`/`42B50` |
| 5EJ-d8009d28c-state | 189 | `D_8009D28C` int-state setters `17FDC`/`17FF0`/`192B8`/`192C8` |
| 5EK-d8009d270-bitwise | 191 | `D_8009D270` unsigned flags setters `87198`/`87414` |
| lui-ori probe | 191 | Large-literal `lui;ori` CAPABILITY-VERIFIED (docs only) |
| 5EF-pilot | 192 | Vendored maspsx LOCAL PATCH (sw delay-slot fill); `func_8007FBC0` integrated |
| 5EF | 205 | Remaining 13 delay-slot `sw` members typed and integrated; family closed 14/14 |
| 5EG-first-branch | 206 | First branchy leaf `func_8004F448`; era cc1 `-O1 -G 8` hoists const into `beqz` delay slot word-exact (branch scheduling capability proven) |
| 5EH-arg-return | 207 | First value-returning leaf `func_800438C0` on era path: `-O2 -G8` preserves double store, `addu` return-0, era+gp proven; GCC 14.2 store-merge + `move` documented as $CC-path limits |
| 5EI-first-nonleaf | 208 | First non-leaf `func_800197D0` on era `-O2 -G8`: frame (`addiu $sp,∓0x18`, `sw/lw $ra,0x10($sp)`) + `jal func_800375B4`; teardown `addiu $sp,+0x18` lands **in the `jr` delay slot** word-exact |
| 5EJ-outgoing-arg | 209 | `func_80019484(int **)` on era `-O2 -G0`: double-dereference load schedule sets outgoing `$a0` before `jal func_800438C0`; load-delay nop, jal nop, frame, and teardown-in-`jr`-slot all word-exact |
| 5EK-volume-197f0 | 210 | First post-probe volume leaf: `func_800197F0` on era `-O2 -G0` transfers the proven 197D0 frame + void `jal` + return-1 + teardown-in-`jr`-slot shape word-exact; no new primitive |
| 5EL-return-forwarding | 211 | `func_8007F7A8` on era `-O2 -G0` forwards `func_8007FCAC`'s `$v0` untouched and reproduces retail's opposite epilogue schedule: teardown before `jr`, nop in the delay slot; all eight words exact |
| 5EM-boot-6a8d4 | 212 | First Rung-1 boot leaf: `func_8006A8D4` on era `-O2 -G0` lays out boot memory regions with 19 ordered absolute pointer stores; register-pinned byte cursors reproduce all 68 retail words exactly after two plain-local phrasings fail the retail register allocation/store schedule. Compiler-constrained, target-specific C is documented in source |
| maspsx indexed-store | 212 | Toolchain patch `f0b9155`: default-off `MASPSX_THREE_WORD_SYMBOL_STORE=1` opt-in adds the three-word symbol+register store form; exact 212-leaf regression, 148 tests, and live re-clone durability passed |
| 5EN/5EP-loop-probe | 212 | `func_8006A674` proves five `bnez`/`bgez` loop back-edge delay slots plus store-in-`jr`-slot; L2 and late allocation deltas cleared, but the leaf is parked with a 45-word `$v1` constant-hoist residual and no 213 claim |
| 5EQ-boot-6a64c | 213 | Boot wrapper `func_8006A64C` on era `-O2 -G0`: calls matched-C `func_8006A8D4` then live-asm `func_8006A674`, both proven `void(void)`; both `R_MIPS_26` relocations resolve and teardown-before-`jr` + nop-slot matches all 10 words |
| 5ER-d1c-d48 | 215 | Adjacent byte/word test-and-clear-return twins `func_80038D1C` / `func_80038D48` on era `-O2 -G0`; explicit pointer reuse gives retail `bnez` + j-over delay-slot returns and `sb`/`sw` clears, all 11 words each exact after one natural phrasing retry |
| 5ES-loop-4bf08 | 216 | First loop-as-volume leaf: natural explicit-init pointer walk in `func_8004BF08` clears two parallel `int[8]` arrays; era `-O2 -G0` reproduces all 14 words, including the split pointer advances and backward-`bnez` delay slot, with no pinning or tool flag |
| 5ET-loop-5186c | 217 | Loop-as-volume repeats: pure-register 16-pass bit-serial loop `func_8005186C` on era `-O2 -G0`, all 15 words on the first natural-C try; unconditional `result <<= 1` fills the forward `bnez` skip slot, nop `bgez` back-edge; mid-4204C carve (prefix 0x20, C 0x3C, resume 420A8.s 0x5A0) |
| 5EU/5EV parks | 217 | `func_80055724` (p-load hoist; `-O1`≡`-O2`) and `func_80052BCC` (rotated-loop `$v0`/`$v1` role swap, 13/15) parked as the **PARKED-ALLOCATION/SCHEDULING family** (with `6A674`): cc1 global allocation/scheduling choices natural C can't steer. Banked idioms: rotated loop = explicit first iteration + `while`; signed `char` vs `0xFF` emits the `andi`. Docs only, no carve |
| family diagnosis | 217 | Read-only investigation: **NO SINGLE KNOB**. All three residuals are in cc1 raw output (maspsx can't fix any). `55724` = pre-reorg emission order, no lever; `52BCC` = regclass+sched2 pair (`-fexpensive-optimizations`+`-fschedule-insns2`), `-O1` shows retail loop roles — retry at `-O1`; `6A674` = hardwired `optimize>1`, only lever `-O1` (untested). Toolchain-patch hypothesis closed; per-leaf `-O1` is the route |
| 5EW-52bcc-o1 | 218 | `func_80052BCC` MATCHED: era `-O1 -G0 -fschedule-insns2` (first sched2 leaf) + two-const-mode phrasing (u8 head const dies at guard → loop reload into `$v1`; `int` loop byte → raw `bne`); sched2 hoists head `li` into the `lbu` delay like ccpsx. All 15 words exact; mid-42FC8 carve (prefix 0x404, C 0x3C, resume 43408.s 0x2A8). Also fixed a latent pipefail/SIGPIPE flake in toolchain detection (`grep -q` → `grep … >/dev/null`) |
| 5EX-6a674-o1 | 219 | `func_8006A674` MATCHED after three parked attempts: era `-O1 -G0 -fschedule-insns2` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`, all 152 words + relocs exact with the 5EP semantic pins (load-bearing; dropping → 46 mismatches). `-O1` = per-use `-1` materialization; sched2 = `li`/`addiu`-before-store placement (21 fixes) — **sched2 confirmed as a general retail fingerprint**. Boot Rung 1 complete (`main → 6A64C ✓ → {6A8D4 ✓, 6A674 ✓}`); mid-55430 gap filled exactly (0x260), three contiguous C carves |
| 5EY-boot-3e610 | 220 | Boot display/graphics bring-up `func_8003E610` on era `-O2 -G0` — all 28 words exact on the **first** attempt, no sched2/pins: straight-line dispatcher, ten calls with immediate args (`0x140`/`0xE0` = 320x224), callees extern-declared with call-site-determined signatures. Readiness ranking of `main`'s callees (callee C/SDK coverage, not raw size) picked it; next up the chain: `func_8006A5BC`, `func_8003E680`. Mid-2E7D8 carve (prefix 0x638, C 0x70, resume 2EE80.s 0x192C) |
| 5EZ-boot-6a5bc | 221 | Boot init `func_8006A5BC` on era `-O1 -G0`, all 36 words exact first attempt: four setup calls, two identical `while (f() != 1) VSync(0);` loops (no cross-jump), `7F7A8()` return → `D_800B0DD4` (`unsigned short` via `lhu` reader). `-O1` per-use `$s0=1` materialization (delay-slot + between-loops re-materialization) — third `-O1`-lever leaf; return-use confirmed codegen-safe (raw `$v0` `beq`, no mask). Mid-55430 carve extends the boot block backward: **four contiguous C carves** (prefix 0x598C, C 0x90, then 6A64C/6A674/6A8D4) |
| 5FA-boot-3e680 | 222 | Boot subsystem-init dispatcher `func_8003E680` on era `-O1 -G0`, all 53 words exact: zero 5 globals (Stage-0 reader types), 2000-pass poll loop (`i++` in `jal` slot; `unsigned int` counter for ROM `sltiu` — the only phrasing fix), callback registration + ~11 inits. **First fn-ptr-to-asm-callee arg** (`&func_8003E91C` via R_MIPS_HI16/LO16 against a text symbol). `-O1` predicted by the per-use selection rule (five per-store `lui`s, no CSE). Fingerprint table banks: `-O1` selection rule, return-use readiness, sched2 scope narrowing, fn-ptr arg, unsigned loop compare. Segment-head carve of 2EE80 (C 0xD4, resume 2EF54.s 0x1858) |
| 5FB park | 222 | `func_800698D4` (disc mount, 141 words) PARKED-SCHEDULING at 140/141: nested-if phrasing defeats gcc's range-test collapse (`v!=0 && v!=-1` → `addiu`+`sltiu`+`bnez`), everything exact except one dbr_sched delay-slot steal at search #3's `beqz` — retail declines a `$v0`-setter steal when the branch target is the return block (`$v0` live to `jr`); ours steals. Screening rule + CdlFILE `0x18` frame note banked; candidate stashed; no carve, no leaf claim |
| 5FC park | 222 | `func_8006E834` (post-mount loader + display env, 91 words) PARKED-ALLOCATION at 89/91: five-arg call PROVEN (5th arg `sw $v0,0x10($sp)` in `jal` slot, first try); frame decomposition method banked; retail folds the range test in this unit (per-TU datapoint vs 698D4). Residual: call-result register home (`$v0`+restores vs `$v1`), not source-expressible. Third ccpsx-vs-2.7.2 skew mechanism recorded; candidate stashed; no carve, no claim |
| 5FD-table-2f9cc | 223 | Table clear `func_8002F9CC` (17 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: zero the in-use flag of all 7×220-byte records at `D_800A5D58` (record typed from the `func_8002F7D8` reader; extent `0x604` = 7×220). Key finding: aggregate element type is an addressing-mode lever — `arr[i].field = 0` keeps the symbolic indexed store; flat `arr[i*55] = 0` hoists `la` (flag-invariant). `unsigned char` counter (`andi 0xFF` masks), `sltiu` bound, stride 220B/55W (not 196B/49W). Mid-11718 carve (prefix 0xEAB4, C 0x44, resume 20210.s 0x4010) |
| 5FE-table-2f970 | 224 | Table twin `func_8002F970` (23 words) on era `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: pointer-match search-and-clear over the 2F9CC table (`SlotRecord` typing inherited unchanged); `*p == D_800A5D58[i].body` → clear `inUse`, then `*p = 0` with the `sw` in the `jr` delay slot (5EN pattern). `$a3` body-base hoist = the aggregate lever producing (not preventing) a hoist; back-branch slot FILLED vs 2F9CC's nop — slot fill is per-shape, not per-table. One phrasing fix: operand order in the compare (`body == *p`) for `bne $v0,$v1`. Object-level `%lo` difference on the hoisted base (`D_800A5D58+4` vs `D_800A5D5C`) resolves to identical bytes at link. Contiguous carve with 2F9CC (prefix 0xEA58, C 0x5C, C 0x44, resume 20210.s) |
| 5FF-maspsx-loads | 224 | Toolchain patch `439c244`: `MASPSX_THREE_WORD_SYMBOL_STORE` extended from stores to standalone indexed symbolic LOADS (lb/lbu/lh/lhu/lw/lwl/lwr) under addiu_at — pass-through emits the ASPSX 2.30 three-word lui/addu/op-%lo form; compound lines retain legacy; `lwc2` stays outside (durable negative). Store path untouched; one gate, existing name. Full gate: flag-OFF 224 exact SHA; flag-ON 224 exact SHA (6A674/2F9CC/2F970 unchanged under the extended meaning); 153 vendored tests (was 148, +5 load); re-clone restores all three tracked files byte-identical. `func_800374E8` (which motivated the patch) PARKED — register-coloring residual (structure correct; see Known-open families + parked_blockers.json). 224 unchanged, no carve.
| 5FG-363f4 | 225 | Search-and-clear `func_800363F4` (21 words / 0x54 @ 0x26BF4): 16-entry D_800A7624 scan, clear key on match, break. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 — FIRST leaf exercising the load gate; the probe EXPOSED the 439c244 bug (GNU as uses the DESTINATION reg as temp for lw; ROM uses $at), fixed at 5dac87e. 21/21 words; mid-2422C carve (prefix 0x29C8, C 0x54, resume 26C48.s 0xD5C); full 225 build EXACT SHA.
| 5FH-twin-37548 | 226 | Record-field lookup twin `func_80037548` (27 words / 0x6C @ 0x27D48): scan 4 x 56-byte D_800BCEA8 records for short needle at +0x10, return signed byte0 (+0x00) on match else 0. era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (lh/lbu indexed pair through the 5dac87e $at gate). Twin-hypothesis FALSIFIED: matches 27/27, no coloring skew — live-value-pressure rule refined (see register-coloring-374e8 parked entry). Accumulator phrasing banked (fingerprint table). Mid-27C6C carve: prefix 0xDC, C 0x6C, resume 27DB4 (existing sibling); full 226 build EXACT SHA; packed-span byte-exact.

Detail and leaf-by-leaf narrative: git history + wiki
([Current Status](https://github.com/Blizz127/Parasite-Eve-Decompilation/wiki/Current-Status)).
PC port remains out of scope. Redump.org cross-check still open (non-blocking).
junction.  The provider returns the incoming destination per the BIOS
memset-family convention; `func_80064964` does not consume it.
