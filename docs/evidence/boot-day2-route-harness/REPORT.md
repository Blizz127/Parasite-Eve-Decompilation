# End-to-end boot -> Day-2 route harness (control-flow / story-state traversal)

**Current status (2026-09-12):** the fixed cold-boot route wins the
rehearsal encounter and the first sewer battle. The52,000frame /45milestone
regression uses961fixed pad pairs and passes134.33s. It ends in m0027i
atPC80199734, story68/persist1=1A, liveHP30/status-copy36, three defeated
enemies, clubslot2, both keys retained, D1A04080, ambient overlayF2=0.
A connected continuation enters m0028i at52344 and preserves liveHP30.
New original/native checks pass:381model-fade frames,195captured enemy ticks
and426ambient-loader control cases. FullDay2 remains unproved. See
[sewer evidence](../../ai_context/DAY1_SEWER_FADES.md) and
[rehearsal evidence](../../ai_context/DAY2_REHEARSAL_ROUTE.md).
Older updates below are historical.

## Update (2026-09-11): `m0377i` released by a fourth pad stage

The documented next step (feed `m0377i` module 1's op-77 rectangle at
`0x801953C4` a real Aya position) is done, using the same input-traversal
mechanism that released `m0378i` — a fourth scripted-pad stage, no seeding and
no debug probe left in the tree.

- `m0377i` is entered at frame **6989** (`persist[1]=0x17A`).
- Stage 4 (`PE_ROUTE_SWITCH3`, default frame **6989**) holds `0xFFAF` +
  periodic Cross. `0xFFAF = 0xFF9F & 0xFFEF` (adds the `0x10` bit).
- Under stage 3's `0xFF9F` hold the module-1 rectangle at `0x801953C4`
  (x in (-211, 241), z in (-4607, -4114)) never reports a hit; the module
  loops at `0x8019544C`. Under `0xFFAF` the op-77 test returns a hit,
  `local[4]=1`, the guard at `0x80195410` falls through, and the loop is left.
- `m0377i` module 5 then writes `persist[1]=0x179` at `0x80195758` and
  transfers back to `m0378i` at `0x80195768` (`0xA8067448`), frame **7106**.
- `m0378i` module 0 writes `persist[1]=0x17A` at `0x80195374` and transfers to
  `m0004i` at `0x80195384` (`0xA8000248`), frame **7925**.
- New frontier: `m0004i` type-4 task `0x8009D368`, parked at
  **`pc=0x801B6CC8` opcode 0x00** (module 4's final 1-frame wait loop). Its
  module-4 op-77 volumes (`0x801B6940`, `0x801B6A08`, `0x801B6B74`) are the
  next input-gated rectangles; none of them is entered under the current pad.

Command and verbatim output:

```
$ ./pc_port/build/pe-route-boot-day2-tests
route: frames=8000 stop=frame-limit story=0x00000018 persist1=0x0000017A token=0xA8000248
route: 14/14 ordered milestones reached
PASS: boot -> m0377i bounce -> m0004i route (14 milestones, frontier=m0004i mod4 pc=0x801B6CC8)
```

`PE_ROUTE_PAD4=FFAF` reproduces the same run explicitly; the `0xFFAF` value is
now the harness default (no env var needed). `0xFF9F` reproduces the old
`m0377i` stop exactly. Probe sweep (all with the rest of the defaults, 8000
frames):

| stage-4 mask | result |
|---|---|
| `0xFFBF` / `0xFFDF` / `0xFF9F` / `0xFF7F` | stop in `m0377i` mod1 at `pc=0x8019544C` (old three-stage behaviour) |
| `0xFFAF` | **14/14**, bounce `m0377i -> m0378i -> m0004i`, frontier `pc=0x801B6CC8` |
| `0xFF2F` / `0xFF4F` / `0xFF8F` / `0xFFCF` | `m0377i` released; stops in `m0378i` at `pc=0x80195784` / `0x8019582C` |

## What this is

- Harness: `pc_port/tests/test_route_boot_day2.c`
- ctest name: `route-boot-day2-control-flow`
  (`pc_port/CMakeLists.txt`, ~`add_executable(pe-route-boot-day2-tests ...)`)
- It drives `func_8001220C` (the translated retail main loop) against the
  user's Disc 1 image with a deterministic scripted pad, and asserts the
  address-exact `persist[]`/token transitions documented in
  `docs/ai_context/DAY1_DAY2_TRANSITIONS.md` and re-derived by
  `pc_port/tools/pe_day1_day2_transitions.py`.

This is a **control-flow / story-state traversal proof**. It does NOT prove
audio fidelity (XA is not decoded), pixel/timing fidelity (no hardware
rasterizer), or that the pad sequence is the unique player solution.

## Route actually executed (step by step)

Observed trace from the default run (frame numbers from the harness):

| frame | `persist[74]` | `persist[1]` | token | transition |
|---:|---:|---:|---|---|
| 1 | 0x00 | 0x00 | 0x00000000 | cold boot |
| 2 | 0x00 | 0x00 | 0xA9400048 | `m5000i` |
| 6 | 0x00 | 0x00 | 0xA80830C8 | `m0431i` |
| 13 | 0x01 | 0x00 | 0xA8001048 | **`m0010i`** (profile/name entry) |
| 827 | 0x08 | 0x0A | 0xA8001048 | name-entry story step |
| 857 | 0x08 | 0x0A | 0xA8000148 | **`m0002i`** |
| 939 | 0x09 | 0x0A | 0xA8000148 | `m0002i` story write (`801A9BAC`) |
| 1207 | 0x09 | 0x02 | 0xA80001C8 | **`m0003i`** |
| 2040 | 0x09 | 0x03 | 0xA8067148 | **`m0372i`** (FMV003 prefix) |
| 4884 | 0x11 | 0x03 | 0xA8067148 | `m0372i` story write |
| 4920 | 0x12 | 0x03 | 0xA8000248 | **`m0004i`** |
| 5039 | 0x18 | 0x03 | 0xA8000248 | `m0004i` story write (`801B5FD4`) |
| 6007 | 0x18 | 0x04 | 0xA8067448 | **`m0378i`** (`m0004i @801B69D0`) |
| 6989 | 0x18 | 0x17A | 0xA80673C8 | **`m0377i`** (`m0378i @80195728`) |
| 7106 | 0x18 | 0x179 | 0xA8067448 | `m0377i` mod5 @80195758 -> back to **`m0378i`** |
| 7925 | 0x18 | 0x17A | 0xA8000248 | `m0378i` mod0 @80195384 -> **`m0004i`** |
| 8000 | 0x18 | 0x17A | 0xA8000248 | frame-limit stop, frontier PC below |

Tokens are packed room names (`decode_packed_name` from
`tools/research/pe_pst0_scan.py`): `0xA8001048`=`m0010i`,
`0xA8000148`=`m0002i`, `0xA80001C8`=`m0003i`, `0xA8067148`=`m0372i`,
`0xA8000248`=`m0004i`, `0xA8067448`=`m0378i`, `0xA80673C8`=`m0377i`.

## The exact stop

`route: frames=8000 stop=frame-limit story=0x00000018 persist1=0x0000017A
token=0xA8000248`

```
route: frontier actors (D_8009D20C list):
  actor=800BEA90 type=1 id=0 task=00000000 pc=00000000 op=00 delay=0
  actor=800BF210 type=3 id=0 task=00000000 pc=00000000 op=00 delay=0
  actor=800BEF90 type=4 id=0 task=8009D368 pc=801B6CC8 op=00 delay=1
    locals: [0]=F8E80000 [1]=FE4BE426 [2]=1F860000 [3]=00000001 ...
  actor=800BED10 type=0 id=0 task=00000000 pc=00000000 op=00 delay=0
```

- Room: **`m0004i`** (re-entered from `m0378i` after the `m0377i` bounce).
- Frontier task: `0x8009D368` (actor `0x800BEF90`, type 4), stuck at
  **`pc=0x801B6CC8` opcode 0x00** (`jump` back into module 4) with `delay=1`.
- That PC is the tail of `m0004i` module 4:

  | PC | opcode | meaning |
  |---|---|---|
  | `0x801B691C` | 0x5E | copy Aya pos into locals |
  | `0x801B6940` | 0x77 | op-77 rect (int XZ `FAED,2156` / `FAED,2480` / `F656,2480` / `F656,2156`) -> `0x801B69D0` transfer |
  | `0x801B69E4`/`0x801B6A08` | 0x5E/0x77 | second op-77 volume (`0DE8,0744` .. `0B5A,0A94`) |
  | `0x801B6B50`/`0x801B6B74` | 0x5E/0x77 | third op-77 volume (`F812,0875` .. `F6A9,0B50`) |
  | `0x801B6CC8` | 0x00 | `jump` back into module 4 -> loops |

  The three `m0004i` module-4 rectangles are large outdoor volumes (tens of
  thousands of 16.16 units across); the deterministic pad walks Aya past all
  three without entering any hit band, so module 4 loops on its own `jump`.
  This is the next input-gated stop, not a port gap (zero UNSUPPORTED stubs).

## How `m0378i` was passed (input, not seeding)

A dedicated diagnostic build printed every `func_80014DA0` call's inputs,
result, and copied polygon (`PE_ROUTE_OP77DBG`); that instrumentation has been
reverted and is not part of the committed harness. It showed:

- `m0378i` module 4 tests **two** rectangles. Rectangle #2 (at `0x801957A8`,
  x in (-830, 350), z in (3542, 4642)) and rectangle #1 (at `0x80195670`,
  x in (-1067, 533), z in (-560, -300)) both feed the same module.
- With the previous default (stage 3 = hold `0xFFBF`, held bits `0x20`) Aya's
  world position drifted only along a single `z` axis in `m0378i`-world
  coordinates and never entered either rectangle band, so the module looped at
  `0x8019582C`.
- Holding `0xFF9F` (held bits `0x30`, i.e. the `0xFFBF` and `0xFFDF` bits
  together) walks Aya into the `m0378i` module-4 rectangle #1 band; the op77
  test then returns a hit, sets local[4]=1, and fires the module-4
  room_transfer at `0x80195728` (`0xA80673C8`).

So the `m0378i` gate was released by **driving the pad**, not by seeding a
position. The route needs three distinct held directions across its stages:
`0xFFEF` (held `0x08`, prefix), `0xFFBF` (held `0x20`, walk-to-bench), then
`0xFF9F` (held `0x30`); the previous two-stage pad could not produce the third.

## How `m0377i` was passed (input, not seeding)

Same mechanism, a fourth stage rather than diagnostic seeding:

- `m0377i` module 1's op-77 at `0x801953C4` (int XZ `FF2D,EFEE` /
  `FF2D,EE01` / `00F1,EE01` / `00F1,EFEE`, rectangle x (-211, 241),
  z (-4607, -4114)) is a **small interior** trigger.
- Under stage 3's `0xFF9F` hold Aya is parked outside it, so the gate returns 0
  and the guard at `0x80195410` jumps to the `0x8019544C` loop.
- Stage 4 (`PE_ROUTE_SWITCH3`, default frame 6989) holds `0xFFAF`
  (`= 0xFF9F & 0xFFEF`); Aya then enters the band, `local[4]=1`, the guard
  falls through, and the module completes.
- `m0377i` module 5 writes `persist[1]=0x179` and transfers to `m0378i`;
  `m0378i` module 0 writes `persist[1]=0x17A` and transfers to `m0004i`.

So `m0377i` is a two-room bounce (`m0378i <-> m0377i`) that, once released,
hands the route back to `m0004i` — consistent with the static contract in
`docs/evidence/m0377i-destination-contract/REPORT.md` (only outbound edge
`m0377i -> m0378i`; `m0378i` module 4 arms to `m0377i` or `m0001i`/`m0004i`).
No `M0000I` / Day-1-exit hop happens here; `m0004i` module 4's three op-77
volumes are the next gate.

## How `m0378i` was passed (input, not seeding)

A dedicated diagnostic build printed every `func_80014DA0` call's inputs,
result, and copied polygon (`PE_ROUTE_OP77DBG`); that instrumentation has been
reverted and is not part of the committed harness. It showed:

- `m0378i` module 4 tests **two** rectangles. Rectangle #2 (at `0x801957A8`,
  x in (-830, 350), z in (3542, 4642)) and rectangle #1 (at `0x80195670`,
  x in (-1067, 533), z in (-560, -300)) both feed the same module.
- With the previous default (stage 3 = hold `0xFFBF`, held bits `0x20`) Aya's
  world position drifted only along a single `z` axis in `m0378i`-world
  coordinates and never entered either rectangle band, so the module looped at
  `0x8019582C`.
- Holding `0xFF9F` (held bits `0x30`, i.e. the `0xFFBF` and `0xFFDF` bits
  together) walks Aya into the `m0378i` module-4 rectangle #1 band; the op77
  test then returns a hit, sets local[4]=1, and fires the module-4
  room_transfer at `0x80195728` (`0xA80673C8`).

So the `m0378i` gate was released by **driving the pad**, not by seeding a
position. The route needs three distinct held directions across its stages:
`0xFFEF` (held `0x08`, prefix), `0xFFBF` (held `0x20`, walk-to-bench), then
`0xFF9F` (held `0x30`); the previous two-stage pad could not produce the third.

## Asserted transitions (proved by this run)

The harness fails unless every one of these is observed in order:

- `persist[74]=0x01` then `0x08` (name-entry / first field-prefix step)
- tokens `m0010i` -> `m0002i` -> `m0003i` -> `m0372i` -> `m0004i` -> `m0378i`
  -> `m0377i` -> `m0378i` -> `m0004i`
- `persist[74]=0x09` (`m0002i @801A9BAC`), `0x11` / `0x12` (`m0372i`),
  `0x18` (`m0004i @801B5FD4`)
- `persist[1]` advances `0x0A -> 0x02 -> 0x03 -> 0x04 -> 0x17A -> 0x179 ->
  0x17A` along the chain
- the frontier task at `pc=0x801B6CC8` (so a regression that stalls earlier or
  advances further is both visible)
- the four documented HOST_ADAPTED skips were actually taken
  (`func_801909B4_skip_movie_new_game`, `func_8006E9A0_skip_movie_fade`,
  `func_80014E30_skip_movie` x2, `func_80016F10_skip_opening_menu`)
- **zero UNSUPPORTED boundary stubs invoked** (loud-boundary contract)

## Retail quirks pinned

- `m0004i @801B5FD4` writes `persist[74]=0x18` via the `assign [2,0]` opcode —
  assert is on the value, not a re-derived formula.
- `m0378i @80195728` transfers to `0xA80673C8` when its module-4 rectangle #1
  gate is satisfied; the exact PC and token are in the milestone evidence
  string.
- `m0377i` module 5 writes `persist[1]=0x179` (`assign [2,0]` at `0x80195758`)
  and immediately transfers back to `m0378i` at `0x80195768`; `m0378i` module
  0 writes `persist[1]=0x17A` at `0x80195374` and transfers to `m0004i` at
  `0x80195384`. The `0x179 -> 0x17A` pair is the observable fingerprint of the
  bounce.
- The op77 gate reads the actor script-local block at `actor+0xAC` (mode-1
  operand base); the frontier dump prints `[0..7]` directly so the narrowing /
  sign behaviour of `func_8001CAB0` is observable.
- The harness uses the same `D_8009D20C` actor list walk and `D_8009D300`
  task-pointer convention the retail VM uses (`func_80017018`), so the
  frontier report is retail-shaped, not a host artifact.

## Boundaries / host adaptations relied on

The boot path needs the two documented `HOST_ADAPTED` skips
(`PE_Port_SetSkipMovie(1)`, `PE_Port_SetSkipOpeningMenu(1)`) because the FMV
playback (`func_801909B4`, `func_8006E9A0`) and the opening menu
(`func_80016F10`) are not rendered/decoded. These still *record* their
invocation; the harness asserts they were taken and never treats an
UNSUPPORTED stub as success.

## Gates

```
cmake -S pc_port -B pc_port/build
cmake --build pc_port/build
./pc_port/build/pe-native-tests          # 1374/1374 pass
cd pc_port/build && ctest                # 10/10 pass (route-boot-day2 is #7)
```

`route-boot-day2-control-flow` is registered in ctest with `TIMEOUT 300` and
`WORKING_DIRECTORY` at the repo root (so it can read `local/pe_disc1.path`).
It SKIPs (exit 0) when no disc image is configured.

## Reproduction knobs (harness only)

All optional, default deterministic values are what the assertions run with:

| env | meaning |
|---|---|
| `PE_DISC1_BIN` | explicit disc image path (else `local/pe_disc1.path`) |
| `PE_ROUTE_FRAMES` | override the frame bound |
| `PE_ROUTE_PAD1/2/3/4` | active-low held mask per stage (defaults `FFEF` / `FFBF` / `FF9F` / `FFAF`) |
| `PE_ROUTE_SWITCH`, `PE_ROUTE_SWITCH2`, `PE_ROUTE_SWITCH3` | stage boundaries (defaults `5400` / `6040` / `6989`) |
| `PE_ROUTE_PULSE`, `PE_ROUTE_PERIOD` | pulsed button + period |
| `PE_ROUTE_PCTRACE=1` | print the frontier opcode-PC histogram |
| `PE_ROUTE_AYA_DUMP=1` | print Aya's world position/pad + frontier actor locals |

## What is NOT proved

- The route past `m0004i` module 4 (rehearsal -> sewer -> `M0000I` -> Day-1
  exit -> `M0351I` -> `M0042I` -> Day-2 terminus `M0091I`/`M0092I`).
- Any `persist[74]` value in {`0x78`, `0x80`, `0x88`, `0x90`, `0x138`,
  `0x140`}. Those writers are proved statically (see
  `DAY1_DAY2_TRANSITIONS.md`) but **not** executed by this harness.
- The `M0089I` alternate.
- XA audio, pixel/timing fidelity.
- The `m0004i` module-4 op-77 volumes' hit bands under the scripted pad (none
  of `0x801B6940`, `0x801B6A08`, `0x801B6B74` returned a hit in this run).
- The `m0377i` module 0/2/3/4 setup blocks (camera/slot/effect) are not
  asserted; only the module-1 gate and module-5 transfer are.

## Next concrete step

Extend the traversal past `m0004i` module 4 by walking Aya into one of its
three op-77 volumes (`0x801B6940` leads to the `m0004i -> m0378i` north
transfer at `0x801B69D0` again; `0x801B6A08`/`0x801B6B74` are the mailbox
volumes that lead to `m0005i`). The same diagnostic probe used for `m0378i`
(print every `func_80014DA0` call's inputs/result, then add a fifth pad stage)
is the tractable move. The static target remains the Day-1 exit writer
`M0036I @0x801B82FC (persist[74]=0x80)`.
