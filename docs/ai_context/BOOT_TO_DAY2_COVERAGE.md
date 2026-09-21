# Boot → end of Day 2: function-level coverage map

**Analysis-only deliverable.** This document is a read-only snapshot. It does
not claim that the persistent goal (100% 1:1 retail-accurate decompile + port
from cold boot through end of Day 2) is complete. It records how much of the
retail first-play path is *currently* matched C, what is still assembly, and
what is only a `pc_port/` translation.

## 0. Snapshot state (re-derive, do not trust prose)

| Item | Value | Command |
|---|---|---|
| `configs/USA/disc1.yaml` SHA-256 | `eecb10939ff5b7183c1c9ebcb1e8a8c751966af6bddf906fc98699634c50d87f` | `sha256sum configs/USA/disc1.yaml` |
| Matched-C spans (whole image) | **578** | `python3 tools/build/disc1_plan.py --check` → `867 spans (578 c, 287 asm, 2 rodata)` |
| Plan SHA-256 (truncated print) | `496c44e65323` | `python3 tools/build/disc1_plan.py --check` |
| Retail EXE SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | `docs/generated/DISC1_MATCHING_STATUS.md:7` |
| Image geometry | `0x1EE000` | `python3 tools/build/disc1_plan.py --check` |

`configs/USA/disc1.yaml` and `src/` are being edited by sibling workers; the
counts below are only valid for the YAML hash above. Counting rule used
throughout: **a function is "matched C" iff it is the named symbol of a `c`
span in `configs/USA/disc1.yaml`.** A `src/func_XXXXXXXX.c` file alone is *not*
matching (e.g. `src/func_8001220C.c` is `PARKED`,
`configs/USA/disc1_nonmatching_sources.json`).

---

## 1. Route definition with evidence

### 1.1 Cold boot → entry

| Step | Evidence |
|---|---|
| PS-X EXE header, initial PC | `asm/disc1/header.s:6` — `.word 0x80072534 /* Initial PC */`; text base `0x80010000`, size `0x001EE000` (`asm/disc1/header.s:8-9`) |
| `func_80072534` sets GP/SP (`gp = 0x8009CD70`), calls `func_800726B4`, then `jal func_8001220C` | `asm/disc1/621E4.s:925-934` |
| `func_8001220C` = crt0 `main` | `configs/USA/disc1.yaml:47` — "first MIPS prologue is at 0x8001220C (func_8001220C, crt0 jal target)" |

`func_80072534` is the retail initial PC but the repo splits it as part of a
merged asm span (`configs/USA/disc1.yaml:53` `[0x2A0C, asm]`), so its
individual lane is `asm`.

### 1.2 `func_8001220C` boot spine (proven by direct `jal` extraction)

`asm/disc1/2A0C.s` `glabel func_8001220C` … `endlabel`; the ordered spine is
exactly (`jal` at `asm/disc1/2A0C.s:19,21`, then the loop at `:30,33,37`, then
`:42,44,46,56,64,87,95,114,116,128,130,132,140,198,200`):

```text
func_800725DC, func_8003E610,
loop { func_8006A5BC; while (func_800698D4() != 0) func_80073A44(0); }
func_8006A64C, func_8003E680, func_8006A9E4,
func_80069B08, func_8006AD40, func_8006ECEC, func_8019234C, func_8006F044,
func_801235DC, func_8006E834, func_801909B4, func_8006E9A0,
func_8003F3C4, func_80074D28
```

`func_8006A5BC` is the streaming bring-up (four setup calls, documented at
`pc_port/platform/pe_stream.c:1-20`); `func_800698D4` returning non-zero means
"disc not mounted", which is why the native port bounds that loop
(`pc_port/bootstrap/func_8001220C_port.c:65-103`).

### 1.3 Boot → title/new-game

| Step | Evidence |
|---|---|
| Opening menu / title loop = `func_801909B4` (overlay body `[0x801909B4,0x801918F8)`, 977 words) | `pc_port/game/boot/func_801909B4_port.c:1-14` |
| Title-screen 480-frame loop `func_80190660` completes | `pc_port/game/boot/func_801909B4_port.c:8-9` |
| Saved-bit-zero arm is a hard stop at `0x80191120` | `pc_port/game/boot/func_801909B4_port.c:10,166-168` |
| New-game arm enters `func_80192CE8(1)` (opening-FMV path) | `pc_port/game/boot/func_801909B4_port.c:173`, `pc_port/game/boot/func_80192CE8_port.c:27` |
| Native "skip movie → new game" host adaptation | `pc_port/tests/test_native.c:34782-34790` (`func_801909B4_skip_movie_new_game`), `PE_Port_SetSkipMovie` at `pc_port/bootstrap/game_port.c:39-46` |

### 1.4 Opening FMV → first field

- **File identity is `RESEARCH_REQUIRED`**: `docs/acceptance/PE_DAY1_ACCEPTANCE_CONTRACT.md:182`
  ("Opening FMV … file not opcode-mapped"). The proven, *different* FMV on the
  field path is FMV003 = ISO `FMV1/FMV003.STR;1` via opcode `0x35 3`
  (`PE_DAY1_ACCEPTANCE_CONTRACT.md:187`).
- Curb/limo material was observed in PE-RD0; `m0001i` as predecessor is
  **TENTATIVE** (`PE_DAY1_ACCEPTANCE_CONTRACT.md:155,183`).

### 1.5 Proven first-play field prefix (Day 1)

```text
m0002i -> m0003i -> m0372i -> FMV003 -> m0004i -> m0378i -> m0377i
```

Evidence: `docs/acceptance/PE_DAY1_ACCEPTANCE_CONTRACT.md:125,128-143`
(rungs RD5-C2 `1e7f0df`, RD6-A `65446c8`, RD6-B `a0baebc`); `m0377i` identity
proven (`0xA80673C8`, table index 376) but its destination contract is
`RESEARCH_REQUIRED` (`PE_DAY1_ACCEPTANCE_CONTRACT.md:134-143,190`).

### 1.6 Rehearsal / sewer scripts → `M0000I` dispatcher

Script transfer inventory (`docs/ai_context/DAY1_ROUTE_AUDIT.md:11-27`), verified
by `python3 pc_port/tools/pe_day1_route_audit.py` (35 immediate transfers / 15
chunks, `DAY1_ROUTE_AUDIT.md:5-8`):

```text
M0319I -> M0023I, M0026I, M0012I
M0023I -> M0319I, M0367I, M0026I
M0367I -> M0319I            (Day-1 branch inspected)
M0026I -> M0027I -> M0028I -> M0029I -> M0030I -> M0031I -> M0032I
       -> M0033I -> M0034I -> M0359I -> M0036I -> M0000I (dispatcher sentinel)
```

`M0319I` module 7 supplies the sewer-entry interaction (opcode 77 rectangle
`X693..1252, Z-999..-360` at `801A359C`; `DAY1_ROUTE_AUDIT.md:94-99`).
`M0036I` conditionally sets `g74=0x80` and `g1=0x24` then requests `M0000I`
(`DAY1_ROUTE_AUDIT.md:40-42`). `func_8001220C` handles token `A8000048` by
calling `func_8006ECEC` and then the scratchpad entry `func_8019234C`
(`DAY1_ROUTE_AUDIT.md:41-44`). **Do not index the room table with `M0000I`**
(index-1) — `DAY1_ROUTE_AUDIT.md:48`.

### 1.7 Day 1 exit selector → Day 2 entry

Evidence: 12 executions of the original exit selector `func_80192030` prove
`g74=0x78 → M0036I` and `g74=0x80 → M0351I` (`DAY1_ROUTE_AUDIT.md:60-63`).
`M0351I` module 1 writes `0x88` at `80191114` before requesting `M0042I` at
`80191124` (`DAY2_ROUTE_AUDIT.md:41-44`). Final selector semantics were proven
by `pe_day2_entry_paths.py`: signed `g74 <= 0x80` selects `M0042I` after
clearing `persist[1]`; otherwise `g74 == 0x8A` selects `M0092I`
(`DAY2_ROUTE_AUDIT.md:45-47,83-88`). **Day 1 completion marker remains
`RESEARCH_REQUIRED`** (`PE_DAY1_ACCEPTANCE_CONTRACT.md:193`).

### 1.8 Day 2

```text
M0351I (g74=0x88) -> M0042I (g74=0x90) -> M0043I / M0041I ...
                  -> station / shared rooms
                  -> park scripts M0191I / M0037I / M0374I + interiors
```

Evidence: `DAY2_ROUTE_AUDIT.md:52-69` (transfer + story-assignment table,
`M0042I` base `801C5224`). Station expansion `DAY2_STATION_ROUTES.md`,
`DAY2_WORLD_MAP_ROUTES.md`, `DAY2_PARK_PROGRESSION.md`,
`DAY2_STATION_RETURN.md`, park interiors
`DAY2_PARK_INTERIOR_ROUTES.md` / `DAY2_PARK_PASSAGE_ROUTES.md`, late park and
movies `DAY2_LATE_PARK_AND_MOVIES.md` (all indexed from
`DAY2_ROUTE_AUDIT.md:17-37,188-211`). **Adjacency does not prove Day 2
membership** — `DAY2_ROUTE_AUDIT.md:5-6,67-69`.

### 1.9 The engine the route runs on (load-bearing, and *not* in the direct closure)

The in-game task VM `func_80017018` dispatches via the pointer table
`D_800910A0[op]` (`asm/disc1/7640.s:302-307`, `jalr $v0` at `:306`). `D_800910A0`
is the head of the mid-image data island `rodata` span
`configs/USA/disc1.yaml:1555-1558` (`D_800910A0` is a func-pointer table).
`func_80017018` is reached from `func_80036224` (recorded at
`pc_port/game/boot/func_80017018_port.c:5-6`), not from the `func_8001220C`
tail — which is why §3 counts a second, script-driven tier.

Field rooms load through `func_8006B4F8` (the `m0431i` destination loader in
the native test `pc_port/tests/test_native.c:34750-34766`), which *is* in the
direct closure.

---

## 2. Counting method (reproducible)

Two functions were used; both are read-only and were run against the snapshot
hash in §0.

```bash
# 2.1 authority + totals
python3 tools/build/disc1_plan.py --check
sha256sum configs/USA/disc1.yaml

# 2.2 lane lookup: map a VMA to its span kind from disc1.yaml
#     span[0]=file offset, span[1]=kind, span[2]=symbol name
#     VRAM = 0x80010000 + file_offset - 0x800
grep -nE '^ *- \[0x[0-9A-F]+, (c|asm|rodata)' configs/USA/disc1.yaml

# 2.3 call graph: glabel/nonmatching/jal from asm/disc1/*.s; func_* identifiers from src/*.c
#     (script used in §3; no build products consumed)
```

- **Tier A** = transitive closure of direct `jal` targets from the retail
  initial PC `0x80072534`, resolved in `asm/disc1/*.s`, treating each `src/*.c`
  file as its own caller of the `func_*` identifiers it names, and keeping only
  targets inside the loadable image `[0x80010000, 0x80010000+0x1EE000)`.
- **Tier B** = text-pointer targets of the field-VM dispatch table
  `D_800910A0`, read from `build/disc1.main.bin` at file offset
  `0x800910A0 - 0x80010000` over the opcode space `0x2000` words
  (`op = word & 0x1FFF`, `func_80017018_port.c:8`).
- A function is **matched C** iff it is the named symbol of a `c` span.
  Otherwise it is counted under **asm** (including `S`-merged spans and the
  second `rodata` blob).

---

## 3. Honest totals

### 3.1 Whole image (authority)

`python3 tools/build/disc1_plan.py --check` → `867 spans (578 c, 287 asm, 2 rodata)`,
geometry `0x1EE000`. (Note: "578" counts `c` spans, i.e. named leaves; the
`asm` count of 287 counts *spans* which may contain many functions.)

### 3.2 Tier A — direct boot closure

| Metric | Value |
|---|---|
| functions in closure | 733 |
| **matched C** | **168** (23%) |
| asm / unsplit | 565 |
| matched-C span bytes | 13,748 B = **3,437 words** |
| asm bytes where a `nonmatching NAME, 0xN` hint exists (560 of 565 funcs) | 257,764 B = **64,441 words** |
| asm functions with no size hint | 5 |

The Tier-A C spans are 168 of the 578 image-wide matched leaves (**29.1%**).

### 3.3 Tier B — field/script-VM dispatch targets

| Metric | Value |
|---|---|
| text targets in `D_800910A0[0..2047]` | 229 |
| **matched C** | **102** (44.5%) |
| asm / unsplit | 127 |
| overlap with Tier A | **0** |

### 3.4 Union (the boot→Day 2 working set)

| Metric | Value |
|---|---|
| Tier A ∪ Tier B | **962 functions** |
| **matched C** | **270** (28.1%) |
| asm / unsplit | 692 |

Distribution by code region (from the YAML, not the closure):

| Region | spans | C spans | C bytes | asm spans |
|---|---|---|---|---|
| `0x8001220C`+ (whole EXE text) | 866 | 578 | 28,380 | 287 |
| boot entry cluster `0x8001220C..0x80013000` | 9 | 6 | 1,724 | 3 |
| field VM + handlers `0x80017000..0x8001C000` | 135 | 99 | 3,752 | 36 |
| field/scene subsystem `0x80028000..0x80030000` | 11 | 9 | 1,512 | 2 |
| CD/boot helpers `0x8006A000..0x80070000` | 28 | 24 | 7,084 | 4 |
| SDK/graphics/audio `0x80070000..0x80082000` | 146 | 92 | 3,440 | 54 |
| tail `0x800C2AF8..0x800D4850` | 109 | 71 | 1,660 | 38 |

### 3.5 Existing non-matching drafts on the path (not counted as C)

Five Tier-A asm functions already have a `src/*.c` draft, all dispositioned in
`configs/USA/disc1_nonmatching_sources.json`:

| function | disposition |
|---|---|
| `func_8001220C` | `accepted-residual` |
| `func_800725DC` | `accepted-residual` |
| `func_800698D4` | `accepted-residual` |
| `func_8006A9E4` | `in-progress-checkpoint` |
| `func_8001F814` | `nonmatching-native-cut` (jump table in the `0x800` rodata pool) |

Per `configs/USA/disc1_nonmatching_sources.json`, these are **not** matching and
are excluded from the 578.

---

## 4. Gap list, prioritized

Fan-in below is within Tier A ∪ Tier B (distinct caller functions), so it
measures how much of the *path* transitively depends on the leaf. Size is the
`nonmatching NAME, 0xN` hint.

### 4.1 Boot spine (highest priority — these gate everything)

| function | VMA | size | lane | fan-in | why it blocks |
|---|---|---|---|---|---|
| `func_800698D4` | `0x800698D4` | `0x234` | asm (draft `accepted-residual`) | 29 | disc-mount wait; `func_8001220C` spins on it (`asm/disc1/2A0C.s:33`) |
| `func_80074D28` | `0x80074D28` | `0x98` | asm | 8 | `func_8001220C` tail (`asm/disc1/2A0C.s`) |
| `func_80069B08` | `0x80069B08` | `0x5E0` | asm | 6 | `func_8001220C` tail |
| `func_8006AD40` | `0x8006AD40` | `0x61C` | asm | 5 | `func_8001220C` tail |
| `func_801909B4` | `0x801909B4` | (overlay span) | asm | — | title/new-game selector; overlay body 977 words, only 246 implemented (`func_801909B4_port.c:1-11`) |
| `func_8019234C` | `0x8019234C` | `0x1297A0` (merged) | asm | — | scratchpad overlay frame loop; 253 words, 30 static direct-call sites; native is `Bootstrap_ReturnVoid` (`pc_port/include/psx_compat.h:125`, `func_8001220C_port.c:177`) |
| `func_801235DC` | `0x801235DC` | merged | asm | — | `func_8001220C` tail |
| `func_8003F3C4` | `0x8003F3C4` | `0x394` | asm | 6 | `func_8001220C` tail (`func_8003F3C4_port.c` exists) |

### 4.2 Field/script VM (highest fan-in among the gap targets)

First 24 table-relative rows of `D_800910A0` are asm
(`func_80017294`, `func_800172BC`, `func_800172E0`, `func_800172FC`, …), and the
VM entry `func_80017018` itself has **no `c` span**
(`func_80017018_port.c` is a translation, its header cites 159 words
`0x80017018..0x80017294`).

Ranked by Tier A∪B fan-in:

| rank | function | VMA | size | fan-in | callers observed |
|---:|---|---|---:|---:|---|
| 1 | `func_8001A680` | `0x8001A680` | `0x104` | 27 | field handlers `func_8001D340`, `func_8001F814`, `func_8001F9C4`, `func_80020C74`, `func_80021F38`, … |
| 2 | `func_80073A44` | `0x80073A44` | `0x178` | 16 | `func_8001220C`, `func_80014E30`, `func_800698D4`, `func_80069B08`, `func_8006AD40`, … |
| 3 | `func_80077AC4` | `0x80077AC4` | `0x3C` | 16 | `func_8002B29C`, `func_800314E4`, `func_80031760`, … |
| 4 | `func_8006DE80` | `0x8006DE80` | `0x54` | 15 | `func_8001D340`, `func_8001F814`, `func_80022210`, `func_800236E8`, … |
| 5 | `func_80072714` | `0x80072714` | `0xC` | 14 | BIOS `syscall 1` wrapper (handwritten); 22 callers |
| 6 | `func_80071A74` | `0x80071A74` | `0xC` | 14 | `syscall 0xA0` / fn-`0x3F` wrapper; 45 callers |
| 7 | `func_8008CBA8` | `0x8008CBA8` | `0x3C8` | 12 | streaming command producer; 42 callers |
| 8 | `func_80074DC0` | `0x80074DC0` | `0x68` | 11 | `func_80014E30`, `func_8003F3C4`, `func_80069B08`, `func_8006AD40`, … |
| 9 | `func_80052F70` | `0x80052F70` | `0x5C` | 11 | 61 callers; calls `func_80051E58` |
| 10 | `func_80074D28` | `0x80074D28` | `0x98` | 9 | `func_8001220C`, `func_80014E30`, `func_80069B08`, `func_8006AD40` |
| 11 | `func_80073C5C` | `0x80073C5C` | `0xC` | 9 | `syscall 0xB0` wrapper |
| 12 | `func_8006DF50` | `0x8006DF50` | `0x58` | 9 | `func_80015DAC`, `func_80025EE8`, `func_80026824`, … |

Highest fan-in in Tier A∪B (supermassive merged spans, **not** good first
targets — listed so the queue is honest): `func_80062D2C` (`0x80062D2C`,
`0x1F0`, fan-in 8), `func_80079FB4` (`0x79FB4`, `0x174`, fan-in 7),
`func_80065400` (merged `0x51BC`, port exists), `func_800438EC`,
`func_80033A40`.

### 4.3 Field-VM handler leaves still asm (Tier B-only, largest first)

These are all merged-`S` spans, so their true size is smaller than shown. They
are the field-engine handler surface the day-1/day-2 scripts actually drive.

| function | span base | merged span size |
|---|---|---:|
| `func_8001A374` | `0x8001A374` | `0x6B88` |
| `func_8001A390` | `0x8001A374` | `0x6B88` |
| `func_8001A3FC` | `0x8001A374` | `0x6B88` |
| `func_8001A43C` | `0x8001A374` | `0x6B88` |
| `func_8001A474` | `0x8001A374` | `0x6B88` |
| `func_80012C20` | `0x80012C20` | `0x41D8` |
| `func_80014228` | `0x80012C20` | `0x41D8` |
| `func_80014E30` | `0x80012C20` | `0x41D8` |
| `func_80013C34` | `0x80012C20` | `0x41D8` |

`func_80014E30` is load-bearing for movies: it is the opening-movie branch that
the native port host-adapts at `pc_port/game/boot/func_80017018_port.c:1179`
(`func_80014E30_skip_movie`).

---

## 5. `pc_port/` parity per route step

Legend: **implemented** = real host equivalent; **partial** = translated with a
named stop/boundary; **stub** = `Bootstrap_ReturnVoid`-class
(`pc_port/bootstrap/stub_registry.h:8-11`); **absent** = no port.

| Route step | Status | Evidence / file |
|---|---|---|
| EXE load / entry | implemented (host loader) | `pc_port/platform/pe_guest_image.c`, `pe_guest_ram.c` |
| Boot spine (`func_8001220C`) | partial — translated from the PARKED candidate, host-bounded disc wait, scratchpad call collapsed | `pc_port/bootstrap/func_8001220C_port.c:1-8,55-103,177` |
| Overlay frame loop `func_8019234C` | **stub** | `pc_port/include/psx_compat.h:125` (`Bootstrap_ReturnVoid`) |
| CD device / disc image | implemented (device model + mounted-disc delivery) | `pc_port/platform/pe_cdreg.c`, `pe_disc.c`, `pe_libcd.c`; stages 143-149 in `DAY2_ROUTE_AUDIT.md:279-333` |
| CD sector transfer (DMA3) | partial — sectors reach stream records; physical FIFO boundary | `DAY2_ROUTE_AUDIT.md:335-341`; `pe_stream.c:1-20` |
| Movie overlay buffers / display | implemented | `pc_port/game/boot/movie_overlay_port.c:18,69,115,123,158,184` |
| Movie slice callback `func_801214D4` | partial — explicit stops at `func_8010C01C` and stream `func_8007C564` | `pc_port/game/boot/movie_overlay_port.c:43-45` |
| Movie player `func_80121C04` | partial — setup/search/decode graphs translated; search/retry waits stop | `pc_port/game/boot/movie_overlay_port.c:363-364,402-403`; `pc_port/tests/test_movie_player.h:1-11` |
| MDEC decode | partial — RLE/IDCT/pixel output; hardware rounding and libpress integration unproven | `pc_port/platform/pe_mdec.c:1-30`; `DAY2_ROUTE_AUDIT.md:343-364` |
| Title/new-game selector `func_801909B4` | partial — 246/977 words through the positive arm; saved-bit-zero arm is a named cut at `0x80191120` | `pc_port/game/boot/func_801909B4_port.c:1-14,166-168` |
| Opening-FMV skip | implemented (host policy, not retail path) | `pc_port/bootstrap/game_port.c:39-46`; `pc_port/tests/test_native.c:34782-34790` |
| Field room load (`func_8006B4F8`) | implemented (disc-backed, chunk destinations verified) | `pc_port/tests/test_native.c:34750-34766` (`FLD1_6B4F8_m0431i_load`) |
| Field/script VM (`func_80017018`) | partial — many opcodes ported; unported table slots are explicit boundaries that request `PE_PORT_STOP_UNRESOLVED_BOUNDARY` | `pc_port/game/boot/func_80017018_port.c:8-45` |
| Field script VM opcode coverage | partial — ~100 handlers listed | `pc_port/game/boot/func_80017018_port.c:35-45` |
| Field graph reach = closure | implemented | 733 functions in the Tier-A closure; 385 of the 565 asm functions have a `pc_port` definition |
| Battle | partial — BTL progress exists but retail handoff is `RESEARCH_REQUIRED` | `PE_DAY1_ACCEPTANCE_CONTRACT.md:366-379` |
| Audio/SPU | partial — guest-state transcribed; SPU/DMA hardware effects collapsed | `pc_port/platform/pe_stream.c:1-30` |
| Memory card | partial | `pc_port/platform/pe_save.c`, `pc_libcard.c` |
| Day 1 acceptance | **not met** — field prefix only through `m0378i`, text IDs only, FMV playback absent | `PE_DAY1_ACCEPTANCE_CONTRACT.md:145,204-238` |
| Day 2 terminal transition | **absent/unproven** | `DAY2_ROUTE_AUDIT.md:26-27,196-197,201-202` |

Per-step port-file mapping for the boot spine:
`func_800725DC`→`bootstrap/func_800725DC_port.c`;
`func_800698D4`→`bootstrap/func_800698D4_port.c`;
`func_8006E834`→`bootstrap/func_8006E834_port.c`;
`func_8006E9A0`→`bootstrap/func_8006E9A0_port.c`;
`func_8006ECEC`→`bootstrap/func_8006ECEC_port.c`;
`func_8003F3C4`→`game/boot/func_8003F3C4_port.c`;
movie-overlay members (`func_80073A44`, `func_80069B08`, `func_8006AD40`,
`func_801214D4`, `func_8019234C`, `func_801235DC`) live in
`game/boot/movie_overlay_port.c`.

---

## 6. Open unknowns (do not guess)

1. **Opening-FMV file identity** — `RESEARCH_REQUIRED`
   (`PE_DAY1_ACCEPTANCE_CONTRACT.md:182,153`). No opcode/file mapping proven;
   cannot be substituted by FMV003.
2. **`m0001i` as opening predecessor** — TENTATIVE, not proven
   (`PE_DAY1_ACCEPTANCE_CONTRACT.md:155,183`).
3. **First combat encounter identity** — none of the proven prefix rooms contain
   battle evidence (`PE_DAY1_ACCEPTANCE_CONTRACT.md:191`).
4. **Day 1 boss identity** — `RESEARCH_REQUIRED`
   (`PE_DAY1_ACCEPTANCE_CONTRACT.md:166,192,379`).
5. **Day 1 completion marker** — no provenance-backed persist/script/map end
   (`PE_DAY1_ACCEPTANCE_CONTRACT.md:193`). The `g74` progression
   `0x6C→0x70→0x78→0x80` is inventoried (`DAY1_ROUTE_AUDIT.md:34-38`) but is not
   an end marker.
6. **`m0377i` destination contract** — identity proven, forward contract unknown
   (`PE_DAY1_ACCEPTANCE_CONTRACT.md:163,190`).
7. **Day 2 terminal transition** — unproven; late-park adjacency extends only to
   story `0x89` with candidate endpoints `0x90`/`0x91`
   (`DAY2_ROUTE_AUDIT.md:199-202`). The `story 0x120` connection to those
   endpoints is unproved.
8. **Async scene execution** — the Day 2 entry/station "middle" is verified only
   as closed predicates, not live traversal
   (`DAY2_ROUTE_AUDIT.md:26-27,63-64,113-114`).
9. **Tier-B closure is not complete** — `D_800910A0` has 50,326 words in the
   rodata span; only the first 2,048 (the `0x1FFF`-masked opcode space) are
   counted. Scripts can reach additional virtual handler sets whose owners are
   resolved at runtime, so Tier B is a **lower bound**.
10. **Tier A is direct-CALL only** — it excludes indirect dispatch and indirect
    function-pointer arrays outside `D_800910A0`; treat 733 as a lower bound for
    the boot field runtime.
11. **Merged spans obscure per-function size** — 560/565 Tier-A asm functions
    have a hint; the 5 without one and every Tier-B-only leaf are inside merged
    `asm` spans, so the §3.2 asm byte total is approximate for those.
12. **Counts are snapshot-bound** — sibling edits to `configs/USA/disc1.yaml` /
    `src/` invalidate §0-§4; re-run §2.

---

## 7. What "done" would require (gap closure, from the evidence in this file)

1. Match the Tier-A boot spine leaves in §4.1 (they gate *every* later step).
2. Match `func_80017018` and the `D_800910A0` handlers in §4.2-4.3 upward from
   the highest fan-in, so the field/script engine is C rather than `pc_port`
   translation.
3. Resolve the six `RESEARCH_REQUIRED` route identities in §6 before any
   Day 1 / Day 2 acceptance claim can be made.
4. Finish movie playback (player `func_80121C04`, stream, MDEC, overlay) —
   currently partial with named hardware boundaries.
5. Prove the Day 2 terminal transition, then extend the coverage set beyond the
   Tier A∪B lower bound.

This document is analysis only; it makes no claim that any of the above is
complete. As of the snapshot in §0 the boot→Day 2 path is **270/962 functions
(28.1%) matched C**, with the remaining **692** still assembly or unsplit.

**Why that ratio is not surprising, and why it understates the work.** The 270
matched-C leaves are the small, self-contained functions completed so far: the
median Tier-A `c` span is well under 100 bytes (largest on path:
`func_8006ECEC`, `0x358`, `configs/USA/disc1.yaml`). The 692 unmatched
functions carry the bulk of the code — the 560 sized Tier-A asm functions
alone are 64,441 words versus 3,437 words of matched C, so **matched C is
~5.1% of the sized words on the boot path**. Progress toward the goal must be
judged by both numbers, never by the function count alone.
