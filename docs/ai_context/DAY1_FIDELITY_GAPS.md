# Day-1 fidelity gaps (cold boot → `persist[74]=0x80`)

**Status:** Day 1 is **not** complete. This is a prioritized blocker list for
100% 1:1 retail accuracy through end of Day 1 (sounds, movies, all systems).
Day 1 ends when `M0036I @ 0x801B82FC` writes `persist[74]=0x80`, after which
`M0351I` starts Day 2 (`DAY1_DAY2_TRANSITIONS.md`).

**Live counts (re-derive):** `python3 tools/build/disc1_plan.py --check` →
`1145 spans (796 c, 347 asm, 2 rodata)`, plan `b50a30290d4d…`.
Route coverage: `371/979` matched C on Tier A∪B; `532` real remaining asm
funcs / `65774` words (`tools/analysis/route_coverage.py --no-history`).

---

## Ranked Day-1 blockers

### 1. Opening FMV — HOST_ADAPTED skip is still the only working New-Game path

| Item | Status |
| --- | --- |
| File identity | **Strong native evidence for `FMV1/FMV001.STR;1` at LBA 189742** (movie index 1 / `func_80192CE8(1)`); acceptance contract still labels critical-path opcode/file mapping `RESEARCH_REQUIRED` (`PE_DAY1_ACCEPTANCE_CONTRACT.md:182`) — do not treat as closed for acceptance |
| Harness / default run | **Partial:** `OPEN1` proves New Game enters real FMV without `skip-movie` (budgeted). Connected route harness still forces `PE_Port_SetSkipMovie(1)`. |
| `got_frame` / C89C | **PASS on real FMV001:** live C89C→7C394→EC. |
| `92CE8` media loop | **Partial:** continue-frame live; default unlimited; B54KY/OPEN1 budget=1 → `func_80192CE8_media_loop`. FMV001 ≈2077 frames. |
| Title join after FMV | **Partial:** pad-seeded body runs `90064`+`8F468`+merge+present; frontier **`0x801911C0`** (loop continue). Canary without pad still `0x801911F8`. |
| Real playback on connected route | **Not yet.** Full STR + title + drop route skip-movie still open |

**Code that must change for real playback:**

1. ~~Deliver a complete last-chunk frame into `func_801924F8` E0~~ **done**
2. ~~`func_80192CE8` past `80192E08` into the media loop~~ **done**
3. ~~Translate `func_80192934` + `3EB04` + DMA1 `80191DC8`~~ **done**
4. ~~Production media loop (until `B0DBA==0`)~~ **done** (host budget for tests)
5. ~~`func_801909B4` enters `92CE8` without skip-movie~~ **done** (`OPEN1` PASS)
6. ~~Title present `8018F2F4` + freelist + `8018FBC0`~~ **done** (frontier
   `0x801911C0` title main loop)
7. Finish full FMV001 (no continue budget) on New Game; translate title
   main loop (repeat `801911C0` until Circle/timeout, then New Game); remove route harness
   `PE_Port_SetSkipMovie(1)` only when that path stays stop-free.
8. Field path `func_80121C04` / `func_80122040` without skip + XA decode.

**Exact next concrete edit:** Translate the title main-loop body past
`0x801911C0`, or run opening STR with unlimited continue budget through
`B0DBA==0` on the New-Game path.
### 2. In-route movies (FMV003 etc.) — proven identity; playback still skipped

| Layer | Proven? | Evidence |
| --- | --- | --- |
| FMV003 identity | **Yes** | ISO `FMV1/FMV003.STR;1` via field opcode `0x35 3` on `m0372i` (`PE_DAY1_ACCEPTANCE_CONTRACT.md:187`) |
| Autonomous stream (first assembled slice) | **Yes (FMV001 fixture + disc)** | `docs/evidence/fmv-autonomous-stream/REPORT.md` — ReadS `0x1E0` served; `HostFB_StreamTick`; `B0DBA=4` / slice bytes match disc |
| Multi-frame updater | **Partial** | `DAY2_MOVIE_UPDATER.md` — `func_80122040` ported; retry/Setloc / cycle-accurate waits incomplete |
| MDEC complete frames | **Partial** | `DAY2_MOVIE_COMPLETE_FRAMES.md` — FMV001 frames 1–3 RLE/MDEC DMA checked; **no hardware pixel golden** |
| XA audio interleaved with STR | **No** | Explicit non-claim in `fmv-autonomous-stream/REPORT.md:136-139` |
| Connected-route FMV003 | **Skipped** | `func_80017018_port.c:1310-1321` — `PE_Port_SkipMovie()` → `func_80014E30_skip_movie`; else unresolved boundary |

Field movie without skip: port/finish `func_80014E30` (106 words, still `asm`) and stop returning early under skip.

---

### 3. Sound / music / SPU / XA on Day-1 path

| Area | Implemented | Stubbed / incomplete | Evidence |
| --- | --- | --- | --- |
| Event BGM handles `0xC8`/`0xC9` (seq 26/27) | Command IDs proven on Day-1 rooms | Full audible score sequencing | `PE_DAY1_ACCEPTANCE_CONTRACT.md:223-238` |
| Audio command queue / EA dispatch | Control/data graphs | Not audible synthesis | `DAY2_AUDIO_DISPATCH.md`; `pc_port/README.md:127-128` |
| SPU mode / DMA / registers | Register + DMA event model | Synthesis collapsed | `DAY2_SPU_MODE_*.md`; `pc_port/platform/pe_stream.c:12-36` |
| Ambient reload after sewer battle | Loader transitions | Provider internals / audible | `DAY1_SEWER_FADES.md:53-72` |
| XA / CD-DA decode + mix | Read modes accepted | **Undecoded** | `fmv-autonomous-stream/REPORT.md`; `DAY2_CD_SECTOR_DEVICE.md` |
| Spatial sound `func_8006DE80` | Native port helpers | Still **asm** matching leaf (fan-in 15) | `route_coverage.py` top list |

---

### 4. Connected route — does **not** reach `M0036I` / `persist[74]=0x80`

| Claim | Verdict |
| --- | --- |
| Static Day-1 exit | **Proven** — `0x78→M0036I`, `M0036I@0x801B82FC→0x80`, `0x80→M0351I` (`DAY1_DAY2_TRANSITIONS.md`) |
| Live harness reaches `0x80` | **No** |

**Current frontier (connected cold boot + reward pilot):**

- Live stop is **m0034i** / `persist[74]=0x6C` (M34 alligator). Prior fixed-pad
  52 000-frame / m0027i notes are historical.
- **F434 cleared** on heal-crit; Aya dies at **f=61469** (close-range PE menu).
- Supply7/11/12/13 item7 Use failed in m32 battle menu; supply14/15 idled at
  m31 door after premature heal arm. Supply16 wedged at door spawn on
  HEAL_WAIT (fixed: center X then `(0,1000)`). Supply17 reached clear + HEAL
  arm but mode=9 Items dismiss ate the menu (no HEAL_MENU). Fix: hold dismiss
  while heal/equip armed; Triangle re-pulse. Supply18: focus stayed 0 after
  HEAL arm (dismiss-hold insufficient). Supply19: heal owns pad through
  `d1a0&4` + Circle/Triangle open pulses; heal WORKS (DONE@58044). Supply20:
  post-DONE Down+Cross on id0 opened id=8; Cross never exits. Supply21:
  call-site/`DFFF` exit → **`A8003148`**@58560; then m32 PE-heal
  (item7=0) starved by cancel-only-in-mask → `A8001048` mode=-1. Fix:
  cancel-only only at HEAL_DONE call site; restore mask PE nav.
  Supply22: exit + **`A8003248` M34**; PE heal 8→38; died re-hug
  (no 2nd heal) mode=-1@64000. Supply23/24: two PE heals then west-pin
  soft-fail (`supply24` x≈-2700). Supply25: east bias held x≈-2316 but
  ATB Up-freeze mode=1 FFEF→frame budget. Supply26: pure-east into boss
  → soft-fail@64000. Supply27 live (lateral-then-east + emergency heal).
  Oracle Use on **m31** `A80030C8`: Down id1→row2 → Cross → id2@row0 → Cross.
- **Never observed live:** `0x78`, `0x80`, `M0036I`, `M0351I`.

Still ahead on Day 1 only: win M34 → `M0359I` → `M0036I` → `persist[74]=0x80`
then Day 2 selector (`DAY1_ROUTE_AUDIT.md` / `BOOT_TO_DAY2_COVERAGE.md`).

HOST_ADAPTED skips remain on every successful connected run (`SetSkipMovie(1)` /
`--skip-opening-menu`).

---

### 5. Matching-C leaves still `asm` that gate Day-1 movie / audio / boot

Live top remaining-asm by on-path fan-in (`route_coverage.py --top`):

| Rank | Function | Fan-in | Words | Day-1 relevance |
| ---: | --- | ---: | ---: | --- |
| 1 | `func_80079FB4` | 19 | 93 | Top remaining; PARKED (`docs/evidence/func-80079FB4/PARK.md`) |
| 2 | `func_80073A44` | 17 | 94 | VSync; movie/boot waits; PARKED |
| 3 | `func_8003708C` | 16 | 7 | Wide fan-in mul residual; PARKED |
| 4 | `func_8006DE80` | 15 | 21 | Spatial sound; PARKED |
| 5 | `func_8008CBA8` | 12 | 242 | Streaming / SPU command producer |

Also still **unnamed `asm`** and load-bearing for movies despite lower/merged fan-in reporting: `func_80014E30` (opcode 35), `func_80017018` (VM), `func_8007C564` / `func_80081314` / `func_8007C214` (stream/CdReadS/DMA3), `func_8001220C` (crt0 main), `func_800698D4` / `func_80069B08` / `func_8006AD40` / `func_8003F3C4` (boot spine). Overlay title/FMV bodies (`func_801909B4`, `func_80192CE8`) are overlay/`pc_port` translations, not disc1 `c` leaves.

---

## What must not be claimed

- Day 1 complete / `persist[74]=0x80` reached on a connected run.
- Opening or field FMV “working” because `--skip-movie` advances story.
- XA audio or hardware-exact pixels from autonomous/complete-frame tests.
- Matching progress from `pc_port/` translations alone.

## Key paths

| Concern | Path |
| --- | --- |
| Skip API | `pc_port/bootstrap/game_port.c` (`PE_Port_SetSkipMovie`) |
| Opening skip | `pc_port/game/boot/func_801909B4_port.c` |
| Field movie skip | `pc_port/game/boot/func_80017018_port.c` (`0x80014E30`) |
| Movie player / ticks | `pc_port/game/boot/movie_overlay_port.c`, `pc_port/platform/host_framebuffer.c` (`HostFB_StreamTick`) |
| Route harness | `pc_port/tests/test_route_boot_day2.c` |
| Day-1 exit proof | `docs/ai_context/DAY1_DAY2_TRANSITIONS.md` |
