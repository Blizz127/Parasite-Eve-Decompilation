# Day-1 completion marker and Day-2 terminal transition

Evidence for the two remaining script/bytecode gaps on the boot→end-of-Day-2
route. Verdicts first:

| Question | Verdict | Key address |
| --- | --- | --- |
| What ends Day 1? | **Proven** — `persist[74]` reaches `0x78`, the exit selector loads `M0036I`, which raises it to `0x80` and returns to the sentinel; the selector then loads `M0351I`, which raises it to `0x88` and transfers to `M0042I`. | `M0036I @ 0x801B82FC` (write `0x80`), `0x801B833C` (transfer) |
| What ends Day 2? | **Proven (two code paths)** — `M0091I` raises `persist[74]` to `0x138` and transfers to `M0351I`; the `M0351I` selector writes `0x140` and transfers to `M0092I`. Separately `M0089I` writes `0x140` and transfers to `M0092I`. | `M0091I @ 0x801C3B38`, `M0351I @ 0x80191158/0x80191168`, `M0089I @ 0x801DE640/0x801DE660` |
| `M0351I -> M0042I` at `g74=0x88` | **Proven** | `M0351I @ 0x80191114` (write), `0x80191124` (transfer) |
| `M0042I -> ... -> M0191I/M0037I/M0374I` as one chain | **Refuted** — `M0374I` is on the `M0042I` static route; `M0191I`/`M0037I` are not. They are entered from `M0195I`/`M0192I`. | see §6 |

This is a static/closed-region result. It is **not** a live traversal proof and
it does not prove frame-by-frame reachability of any individual story value.

## 1. The story word

`persist[]` storage is `D_800A77F0` (`PERSIST_BASE`, 0x200 words). Slot 74 is
therefore `0x800A77F0 + 74*4 = 0x800A7918` — the word the transition overlay and
the `M0351I` script read as the story/progress marker (called `story` by the
existing oracles). `g74` in earlier notes and `persist[74]` here are the same
word.

There are **247** code sites that write slot 74 and **1445** that read it
across the 414 decoded field-table rooms; the value advances in scene-sized
steps (`0x08, 0x18, 0x28, … 0x78, 0x80, 0x88, 0x90, … 0x138, 0x140`). The
transition rooms and the milestone writers are a small subset of the 247.

## 2. Method and tools

New tooling (this lane):

- `pc_port/tools/pe_day1_day2_transitions.py` — decodes every field-table room
  package, inventories every `persist[74]` writer/reader, annotates each room
  transfer with the nearest preceding `persist[74]` write in its module, and
  builds the static room-transfer graph with reachability queries.
  Output: `local/live/day1-day2-transitions.json`.
- `pc_port/tools/pe_story_destination_map.py` — loads the retail transition
  overlay (base `0x8018EFF0`, from `load_u32(EXE,0x80011614)`) and *executes*
  the original selector at `0x80192030` under the project MIPS interpreter for
  each `persist[74]` value, recording the selected room token
  (`0x8009D280`). This is execution of retail instructions, not a script
  re-implementation. Output: `local/live/story-destination-map.json`.

Existing oracles re-run unchanged and passing:

- `pe_day1_route_audit.py` — selector cases + transfer inventory.
- `pe_day2_route_audit.py` — the four shared entry-room scripts.
- `pe_day2_entry_paths.py` — 772 original-handler selector cases,
  96 calculation cases, 256 timer chains, 1050 input/state gate paths.
- `pe_day2_park_progression.py` — closed `M0191I/M0195I/M0037I` predicates.

Commands:

```
python3 pc_port/tools/pe_day1_day2_transitions.py
python3 pc_port/tools/pe_story_destination_map.py
(cd pc_port/tools && python3 pe_day2_entry_paths.py)
```

## 3. Exit selector mapping (retail instructions)

`pe_story_destination_map.py`, one selector invocation per story value:

| `persist[74]` | destination | note |
| --- | --- | --- |
| `0x00..0x77` | `M5000I` | default / no day transition |
| `0x78` | `M0036I` | **Day-1 exit** |
| `0x79..0x7F` | `M5000I` | |
| `0x80` | `M0351I` | **Day-2 entry hop** |
| `0x81..0x15F`, `0x3E6`, `0x7FFFFFFF`, `0xFFFFFFFF` | `M5000I` | every other probed value |

`persist[1]` becomes `0x3E7` in every case. `flags = 0x2000` is invariant for
`0x78/0x80/0x88/0x138/0x140` (the same destinations as `flags = 0`). The probe
sweeps every value in `0..0x15F` plus `0x3E6`, `0x7FFFFFFF`, `0xFFFFFFFF`
(355 cases).

So the map-exit selector has exactly two non-default exits, at `0x78` and
`0x80`. `M5000I` is the "stay / world-map" fallback and is not a day boundary.

## 4. Day-1 completion marker — proven

`M0036I` (token `0xA8003348`, script base `0x801B6C90`, script sha256
`5eb6d60bf94c794b57039f2fd30a3164594dd1a11a0ba2ed3d77d99d69801479`):

| PC | Command | Effect |
| --- | --- | --- |
| `0x801B7120` | `assign [2,0] [74,120]` | `persist[74] = 0x78` (module 1 entry, idempotent) |
| `0x801B82FC` | `assign [2,0] [74,128]` | `persist[74] = 0x80` — **the Day-1 completion marker** |
| `0x801B830C` | `assign [2,0] [1,36]` | `persist[1] = 0x24` |
| `0x801B833C` | `room_transfer` `0xA8000048` | transfer to `M0000I` (dispatcher sentinel) |
| `0x801B8348` | `yield` | |

The `0x80` write at `0x801B82FC` is unconditional: the preceding
`branch_if_zero [3,0] [0,2068]` at `0x801B82C4` targets `0x801B82FC` itself, so
both the taken and fall-through arms reach it.

Chain, with only retail-proven or retail-statically-decoded hops:

```
persist[74]=0x78 --exit selector--> M0036I
M0036I @0x801B82FC: persist[74]=0x80 ; @0x801B833C: -> M0000I sentinel
persist[74]=0x80 --exit selector--> M0351I
M0351I @0x80191114: persist[74]=0x88 ; @0x80191124: -> M0042I   (Day 2 begins)
```

`M0042I @ 0x801C70E4` immediately raises the marker to `0x90` (unique writer),
consistent with `M0042I` being the first Day-2 room.

The `M0351I` write/transfer pair is guarded by the `M0351I` internal selector at
`0x801910DC`/`0x80191130`; `pe_day2_entry_paths.py` proves, with the original
MIPS handlers, `story <= 0x80 -> (0x80191124, 0x88, persist1=0)` across 772
cases.

Other writers of `0x78`/`0x88` (`M0359I @0x801B8550/0x801B8758`,
`M0360I`) are transition stubs / a story-select room, not the Day-1 gameplay
trigger: only `M0036I` writes the unique `0x80` marker.

## 5. Day-2 terminal transition — proven

Two retail paths reach `M0092I` at marker `0x140`.

**Canonical path.** `M0091I` (module 0):

| PC | Command | Effect |
| --- | --- | --- |
| `0x801C3B38` | `assign [2,0] [74,312]` | `persist[74] = 0x138` — end-of-Day-2 marker (unique writer of `0x138`) |
| `0x801C3B48` | `assign [2,0] [1,91]` | `persist[1] = 0x5B` |
| `0x801C3B58` | `room_transfer` `0xA80650C8` | transfer to `M0351I` |

Then `M0351I`'s internal selector:

| PC | Command | Effect |
| --- | --- | --- |
| `0x80191158` | `assign [2,0] [74,320]` | `persist[74] = 0x140` |
| `0x80191168` | `room_transfer` `0xA80654C8` | transfer to `M0092I` |

`pe_day2_entry_paths.py` proves `story == 0x138 -> (0x80191168, 0x140,
persist1=0x3E6)` with the original handlers.

**Alternate path.** `M0089I` writes `persist[74]=0x140` at `0x801DE640` and
transfers to `M0092I` at `0x801DE660` (`M0089I` also writes `0x11B` before its
other transfer to `M0367I`).

Writer census for the terminal markers:

```
0x138 -> only M0091I @ 0x801C3B38
0x140 -> M0089I @ 0x801DE640, M0351I @ 0x80191158
```

## 6. The park-script chain — partly proven, partly refuted

The hypothesis "`M0351I -> M0042I` at `0x88` -> station/shared rooms -> park
scripts `M0191I`/`M0037I`/`M0374I`" is **not one chain**.

Static room-transfer graph reachability from `M0042I` (independent of story
routing) reaches **205** field rooms. Within that set:

- `M0091I` (Day-2 terminal writer) is reached:
  `M0042I@801C58C4 -> M0043I@801BEDBC -> M0051I@801BE1EC ->
  M0055I@801CA504(0x128) -> M0090I@801C2E4C(0x134) -> M0136I ->
  M0126I@801A3670 -> M0125I@801A60A0(0x136) -> M0091I`.
- `M0092I` is then reached via `M0091I -> M0351I@80191168 -> M0092I`.
- `M0374I` is reached on a different branch:
  `M0042I -> M0043I -> M0051I -> M0055I(0x128) -> M0090I(0x134) -> M0136I ->
  M0126I@801A29C8 -> M0146I@801CB378 -> M0147I@801C2BD8 ->
  M0367I@801AAC8C(0x11C) -> M0058I@801B5908 -> M0059I@8019FF08 -> M0374I`.
- **`M0191I` and `M0037I` are NOT reachable from `M0042I`.** Their only
  incoming transfers are:

```
M0191I <- M0195I @0x801AFA94, @0x801AFAAC
M0191I <- M0360I @0x8019046C (0xC0), @0x80190740 (0x160)
M0037I <- M0192I @0x801B457C, @0x801B5CC4, @0x801B683C
M0037I <- M0360I @0x80191404 (0x178), @0x8019150C (0x298)
```

`M0191I` itself transfers onward to `M0195I` (`0x801B68D0`, `0xD0`) and
`M0239I`, and `M0037I` transfers to `M0271I` (`0x801CA660`, `0x282`),
`M0367I` (`0x801CA820`, `0x29E`) and the sentinel. So the park rooms are a
separate branch entered from `M0195I`/`M0192I`, not continuations of the
`M0042I` route.

`M0360I` is a story-select/debug room: it writes a milestone and transfers
directly to the matching room for ~30 values (`0xC0 -> M0191I`,
`0xE4 -> M0374I`, `0x128 -> M0090I`, `0x130 -> M0091I`, `0x160 -> M0191I`, …).

## 7. Limits and hand-off items

- **Static vs feasible.** Reachability here is over the decoded transfer graph.
  Edges into the `M0000I` sentinel terminate the graph; the real successor is
  chosen by the exit selector from live `persist[74]`. The `story_value`
  annotation is "nearest preceding `persist[74]` write in the same module", a
  heuristic, not a guard proof. `pe_day2_entry_paths.py` supplies the
  handler-executed guard proofs for the `M0351I` regions.
- **Not proven:** that a retail player reaches `persist[74]=0x78` before any
  other route (each earlier milestone writer is inventoried in
  `local/live/day1-day2-transitions.json` but not ordered into a single
  walkthrough).
- **Hand-off (would need `src/` matching, not done here):** the field
  dispatcher/SYS0 path that reads slot 74 and drives the exit selector is only
  exercised through the transition overlay + `M0351I` oracles; a matching-C
  leaf for the dispatcher entry and for `M0036I`/`M0091I` module 2/0 would let
  the chain be asserted from matched code instead of decoded bytecode.
- **Not a live run.** No emulator frame loop is executed; the selectors run one
  invocation per case.

## 8. Reproducibility

```
python3 pc_port/tools/pe_day1_day2_transitions.py     # 414 rooms, 247 writers, 1011 transfers
python3 pc_port/tools/pe_story_destination_map.py     # 0x78->M0036I, 0x80->M0351I, else M5000I
(cd pc_port/tools && python3 pe_day1_route_audit.py)
(cd pc_port/tools && python3 pe_day2_route_audit.py)
(cd pc_port/tools && python3 pe_day2_entry_paths.py)   # 772 + 96 + 256 + 1050 + 256 PASS
(cd pc_port/tools && python3 pe_day2_park_progression.py)
```

Requires the user's Disc 1 image via `local/pe_disc1.path` and
`build/disc1.candidate.exe` (sha1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).
