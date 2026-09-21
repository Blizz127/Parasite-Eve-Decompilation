# ROUTE_COVERAGE — boot → end of Day 2, measured

**Analysis-only deliverable.** This records how much of the retail *first-play*
path (cold boot → end of the in-game Day 2) is lifted to matching C, produced by
a reproducible tool rather than a hand-written snapshot.

- Tool: `python3 tools/analysis/route_coverage.py`
- History: `python3 tools/analysis/route_coverage.py --history` appends an
  idempotent entry to `docs/generated/ROUTE_COVERAGE_HISTORY.md` (append-only;
  prior entries are never rewritten; repeated runs at the same plan hash are
  byte-identical).
- Supersedes the manual snapshot in
  [`BOOT_TO_DAY2_COVERAGE.md`](BOOT_TO_DAY2_COVERAGE.md), which is now a
  historical reference (retained read-only; a sibling owns that file).
- Optional line in the exact-rebuild gate:
  `scripts/exact_rebuild.sh` prints `ROUTE_COVERAGE=…` alongside its PASS line.
  This is **informational only** and never changes the gate's PASS semantics.

## Native traversal update (2026-09-12)

The fixed cold-boot route now wins the rehearsal encounter and the first
sewer fight. The52,000frame /45milestone /961pad-pair regression passes134.33s,
ending in m0027i atPC80199734 with30HP, all three enemies defeated and field
control/music state restored. A connected continuation enters m0028i at52344.
Original/native comparison covers381model-fade frames,195captured enemy
model ticks and426ambient-loader control cases. See
[sewer evidence](DAY1_SEWER_FADES.md) and
[rehearsal evidence](DAY2_REHEARSAL_ROUTE.md).
Matching-C coverage below is unchanged. This route remains inDay1; fullDay2
and whole-game fidelity are unproved.

## Three buckets

The on-path working set is split three ways. The middle bucket is **consumed
read-only** from the matching worker's artifact
`docs/evidence/non-c-matchable/MANIFEST.json` (built by
`tools/analysis/nonmatchable_spans.py`) — this tool never edits or invents
classifications.

- **matched C** — the named symbol of a `c` span in `configs/USA/disc1.yaml`
  (PS1 rebuild byte-parity). This is the *total coverage* number.
- **executed-path native C** — the same route union intersected with
  `pc_port/game/{boot,decomp}/func_*_port.c` (and bootstrap ports). This is
  the C-only subset that predicts a playable, restylable build. Matching
  YAML `c` that has no native translation does not run on x86.
- **provably non-C-matchable** — a span the artifact positively classifies
  (`handwritten-jr-t2` BIOS `jr $t2` dispatchers, `handwritten-syscall`,
  `handwritten-cop` GTE primitives). `alignment-filler` spans are layout
  padding, not functions, and are excluded.
- **real remaining asm** — the rest, i.e. genuine outstanding work.

If the artifact is absent, the tool reports `nonmatchable=unknown` and folds
everything into *real remaining asm* (never guesses).

## Method (reproducible, evidence-based)

The route definition is taken from the evidence in
`BOOT_TO_DAY2_COVERAGE.md` §1–§2 and the Day 1 / Day 2 route audits — no path
data is invented.

- **Root** — `func_80072534`, the retail PS-X EXE initial PC
  (`asm/disc1/header.s`), reached as `func_80072534 → func_800726B4 →
  func_8001220C` (crt0 `main`).
- **Tier A — direct boot closure.** Transitive closure of direct `jal` targets
  starting at the root. The call graph is parsed from the split
  (`asm/disc1/*.s`: `glabel`/`endlabel` boundaries + static `jal` operands); a
  matched leaf contributes the `func_*` identifiers named in its
  `src/func_*.c` body.
- **Tier B — field/script VM handlers.** `func_80017018` dispatches through the
  pointer table `D_800910A0[word & 0x1FFF]` (`pc_port/game/boot/func_80017018_port.c`).
  The tool reads the first `0x2000` words of that table from the retail image
  and keeps in-image text targets.
- **Counting rule** — a function is *matched C* iff it is the named symbol of a
  `c` span in `configs/USA/disc1.yaml`. Everything else is *asm* (including
  merged `asm` spans and the `rodata` island). A path function with no
  `nonmatching NAME, 0xN` size hint is reported in an explicit
  `asm_words_unknown` bucket rather than dropped.

**Both tiers are lower bounds.** Tier A is direct-call only (it excludes
indirect dispatch); Tier B reads only the `0x1FFF`-masked opcode window even
though the table's `rodata` span is 50,326 words. A function that cannot be
resolved is reported, not silently dropped: `tierB_unresolved` counts in-image
table targets that carry no resolvable function label.

## Current output

Snapshot state — re-derive, do not trust prose. The plan SHA-256 changes with
every YAML edit, so this record is only valid at the hash below. (Sibling matching
workers keep adding leaves; re-run the tool for the live number.)

```
$ python3 tools/analysis/route_coverage.py --quiet
plan=74acf19a28ffd98c4018b9cd3cc9eee70441ad3054eaccc27b1281b19491c096 \
  funcs=349/979 c_words=6130 nonc_funcs=76 nonc_words=5414 \
  asm_funcs=554 asm_words=66149 asm_words_unknown=0 nonmatchable=present \
  tierA=734 tierB=248 tierB_unresolved=0
```

At this plan the tree is `751 c, 330 asm, 2 rodata` spans, YAML
`a1a1b23b60b3583c188fde621da45856f7ec37ed5d11e89cdc72008687cc911b`; the
on-path working set is 979 functions (Tier A 734, Tier B 248).

- **matched C**: 349 / 979 functions (35.6%), 6,130 words (**7.9%** of on-path words)
- **provably non-C-matchable**: 76 functions, 5,414 words (7.0%)
- **real remaining asm**: 554 functions, 66,149 words (85.1%)

The append-only record of this run is in
`docs/generated/ROUTE_COVERAGE_HISTORY.md`. (This block supersedes the older
`plan=7ede9199b817…` snapshot; the plan-hash-derived history file is the authority
for the full series.)

## Remaining-asm top targets (the matching queue)

Ordered by on-path fan-in (distinct in-path callers), then by size. The full
ordered list is in the `--json` report's `remaining_asm` array.

| rank | function | words | fan-in | note |
|---:|---|---:|---:|---|
| 1 | `func_80079FB4` | 93 | 19 | highest fan-in on-path leaf; PARKED (duplicate-`slt` + inline div guards) |
| 2 | `func_80073A44` | 94 | 17 | boot-tail VSync poll; `func_8001220C` calls it |
| 3 | `func_8003708C` | 7 | 16 | small, high fan-in — cheap win |
| 4 | `func_8006DE80` | 21 | 15 | field handlers call it; PARKED (arg-promotion/schedule) |
| 5 | `func_8008CBA8` | 242 | 12 | streaming command producer; PARKED (allocation/schedule) |
| 6 | `func_80067CBC` | 23 | 11 | |
| 7 | `func_80062D2C` | 124 | 8 | pool allocator |
| 8 | `func_800755F0` | 318 | 7 | cluster gate state machine; PARKED |
| 9 | `func_8003BCE0` | 245 | 7 | large body |
| 10 | `func_800374E8` | 24 | 7 | |

The largest on-path asm bodies (not necessarily highest fan-in) are
`func_80032B0C` (532 w), `func_8006D60C` (335 w), `func_800755F0` (318 w),
`func_8003BCE0` (245 w), `func_8008CBA8` (242 w) — these are the block-level gaps.

## What this proves / does not prove

- **Proves** the current on-path matched-C vs asm split, at a named plan hash,
  with the method and inputs recorded, and an explicit queue ordered by
  dependency weight.
- **Does not prove** the full goal. The boot → end-of-Day-2 objective is **not
  complete**: most on-path code words are still reassembled from the split, the
  `pc_port/` runtime is incomplete where its evidence says so, and several route
  identities remain `RESEARCH_REQUIRED` (see `BOOT_TO_DAY2_COVERAGE.md` §6).
  An exact rebuild (`scripts/exact_rebuild.sh`) is necessary but not sufficient.

## Executed-route frontier (runtime evidence, not a coverage count)

`pc_port/tests/test_route_boot_day2.c` (ctest `route-boot-day2-control-flow`,
~23 s with the disc image present, `TIMEOUT 300`) executes the ported retail
main loop end to end from cold boot against the real Disc-1 image with a
deterministic scripted pad, and asserts **14/14 ordered milestones**:

- cold boot → name entry (`persist[74]=0x01`), then `0x08`, `0x09`, `0x11`,
  `0x12`, `0x18`;
- room tokens `m0010i` → `m0002i` → `m0003i` → `m0372i` → `m0004i` → `m0378i`
  → `m0377i` → `m0378i` → `m0004i`;
- `persist[1]` `0x0A → 0x02 → 0x03 → 0x04 → 0x17A → 0x179 → 0x17A`.

The `m0377i` hop is a two-room bounce: `m0377i` module 1's op-77 rectangle at
`0x801953C4` returns a hit once the fourth pad stage (`PE_ROUTE_PAD4=0xFFAF`,
default since 2026-09-11) walks Aya into its interior band; module 5 then
writes `persist[1]=0x179` and transfers back to `m0378i`, whose module 0 writes
`persist[1]=0x17A` and transfers to `m0004i`.

It stops at a **named, evidenced frontier**: `m0004i` module 4 (actor
`0x800BEF90`, task `0x8009D368`) parked at `pc=0x801B6CC8`, the type-4 task's
1-frame wait loop after its three opcode-`0x77` volumes (`func_80014DA0` →
`func_8001CAB0`). The gates **execute**; none is hit only because the scripted
pad never walks Aya into a trigger rectangle — an **input-traversal limit, not
a port gap** (no unported opcode, no `UNRESOLVED_BOUNDARY`, zero `UNSUPPORTED`
stubs invoked).

**Not proved (stated plainly, do not infer past this):** everything after
`m0004i` — the rehearsal/sewer scripts, `M0000I`, the Day-1 exit writer
(`persist[74]=0x80`), `M0351I`/`M0042I`, the Day-2 terminus (`0x138`/`0x140`,
`M0092I`), and `M0089I`. XA audio is undecoded and pixels are not
hardware-exact. Full detail: `docs/evidence/boot-day2-route-harness/REPORT.md`.

## History

`docs/generated/ROUTE_COVERAGE_HISTORY.md` is the append-only record. Each
entry carries the deterministic timestamp, plan SHA-256, YAML SHA-256, span
counts, the three buckets (function and word counts plus percent), and the
non-C class breakdown. The writer is idempotent: re-running at the same plan
hash leaves the file byte-identical, and prior entries are never rewritten
(a changed number for the same plan — e.g. the classification artifact
appearing — rewrites only that one entry in place).

## Metric scope (2026-09-18)

`route_coverage.py` is a **lower bound** on executed-path native C, and its
`--top` "remaining asm" list must not be read as a to-do list of holes:

- `native_c` counts functions that have a translation unit whose *filename* is
  `func_<name>_port.c`. A leaf translated inside a differently-named TU is not
  counted, and `--top` still lists it as remaining. Verified examples:
  `func_80079FB4` (defined in `pc_port/game/boot/func_8001A15C_port.c`),
  `func_8003708C` (`func_80012850_port.c`), `func_80080B44` (the static
  `PE_Cd_IntToPos` in `pc_port/platform/pe_libcd.c`), `func_80073A44` (VSync).
- The 979-function union is a **direct-call** closure (Tier A) plus the
  `D_800910A0` VM table (Tier B), so indirect-callback subtrees are outside it.
  The field-menu dispatch tree is one: the plain route provably reaches
  `func_8004AD9C` (see the recorded boundary stub) yet the function is absent
  from the union, so translating it did not move `native_c`.
- Current live numbers (`--quiet --no-history`):
  `disc=1 funcs=377/980 c_words=6834 native_c=258/980 native_c_words=27622`.
  (`funcs`/`c_words` moved 372→373 and 6734→6752 on 2026-09-18 when the
  matching-C leaf `func_800525EC` was registered, 373→374 and 6752→6776 with
  `func_80062F3C` (Phase 5FX, +0x60 exactly), then 374→376 and 6776→6809 with
  `func_8005270C` (Phase 5FY) — that one also grew the union itself, 979→980, as
  its neighbours became reachable — and 376→377 / 6809→6834 with Phase 5FZ. The
  Phase 5FW cluster moved neither — see the blindness note below. `native_c` is a
  different metric — the *port's* hand TUs — and has not moved. The earlier
  CD/dispatcher + `func_8004AD9C` work moved neither, which is the honest outcome;
  do not change the metric to make it move without deciding that explicitly.)
- **The metric is blind to leaves outside the 979-function union.** The Phase 5FW
  cluster (`func_80052634`/`func_8005267C`/`func_800526C4`/`func_80052764`) took the
  plan from 798 to **802** c spans and passed the exact-rebuild gate, yet
  `funcs`/`c_words`/`asm_funcs`/`asm_words` are all unchanged — those four are not
  reachable in the direct-call closure, so neither their `c` nor their former `asm`
  count is represented. Compare Phase 5FV, where `func_800525EC` *is* in the union
  and did move `funcs` 372→373. Read `<plan> c spans` and the gate for the real
  count; treat `funcs=373/979` as "union coverage", not "matching progress".

The harness description above is historical (14 milestones, `m0004i`). The
current `pe-route-boot-day2-tests` has a 57-milestone table, a 62000-frame cap
and needs `PE_ROUTE_REWARD_PILOT=1` to reach the sewer milestones; see
`docs/generated/DISC1_GAMEPLAY_BASELINE.md` for the recorded runs.

- Leaf 4 (`func_8004FF30`, 2026-09-18) is the same indirect-subtree class as
  leaf 3: a hand adapter in an existing-TU style (`game/boot/`) for a function
  outside the 979-function union, so `native_c` stays 258/979 by design.
- Leaf 5 (the field-menu input tree `func_8004AE1C` + its four sub-page
  constructors, two sub-handlers, four list-draw adapters and two per-cell
  draws, 2026-09-18) is the same class again — it is the indirect callback
  subtree the plain route provably reaches but the direct-call union excludes.
  `native_c` therefore stays 258/979. What moved is runtime evidence: the plain
  route's last unresolved-boundary stub (`PE_MenuInputCallback` raw `0x8004AE1C`
  at `frames=61623`) is gone and the run now ends at `frames=62000
  stop=frame-limit` with only the four documented HOST_ADAPTED skips. Do not
  read `native_c` as a measure of this work.
- Leaf 6 (the remaining field-menu leaves — Equipment draw/input
  `func_8004B214`/`func_8004B394`, modal input `func_8004B650`, Equipment per-cell
  draw `func_80050438`, and the close page `func_8005D994` + `func_8005247C`,
  2026-09-18) is the same class a third time: asm-derived hand TUs for indirect
  callbacks. `native_c` again stays 258/979. The tree then closed completely —
  `func_8004B5DC` plus its leaves `func_8005FCAC` (signed number printer) and
  `func_8005ED18` (icon/sprite packet builder) landed the same day — so **no named
  boundary remains anywhere in the field-menu tree**. Neither route reaches these
  arms, so no route milestone count moves.
- Leaf 7 (`func_800525EC`, 2026-09-18) is a **different** class: not a port TU but
  a matching-C leaf registered into `configs/USA/disc1.yaml`
  (`[0x42DEC, c, func_800525EC]`, mid-42D94 carve). This one *does* move the
  matching metrics — `funcs` 372→373, `c_words` 6734→6752, `asm_funcs` 531→530 —
  while leaving the port's `native_c` at 258/979, and it is proven by the
  exact-rebuild gate: `EXACT_REBUILD_GATE=PASS`, `sha1_cand =
  sha1_orig = 452fb033…`, `VERIFY_SWEEP=PASS leaves=798`.
- Leaf 8 (the `42E34` cluster — `func_80052634`/`func_8005267C`/`func_800526C4`,
  three 0x48 sound twins, plus the gp-relative `func_80052764`; `func_8005270C`
  left as `asm`, 2026-09-18) is matching-C again and **moves no metric the union
  tracks**: plan 798→**802** c spans, `port_priority` 60→56 active candidates,
  gate `EXACT_REBUILD_GATE=PASS` `sha1_orig == sha1_cand = 452fb033…`,
  `VERIFY_SWEEP=PASS leaves=802`, while `funcs` stays 373/979 and `asm_funcs` 530.
  The four are outside the direct-call closure, which is exactly the blindness
  described above.
- Leaf 9 (`func_80062F3C`, the node-list scan that feeds `func_8006269C`, 24 words,
  2026-09-18) **is** in the union, so it moves the metric: `funcs` 373→**374/979**,
  `c_words` 6752→**6776** (+0x60 exactly), `asm_funcs` 530→**529**; plan 802→**803**
  c spans, gate `EXACT_REBUILD_GATE=PASS` `sha1_orig == sha1_cand = 452fb033…`,
  `VERIFY_SWEEP=PASS leaves=803`. It is also the second leaf this session whose match
  turned on *source spelling* rather than a compiler flag (`int one = 1;` holds the
  loop constant so GCC hoists it into the entry block the way retail does) — see
  `docs/evidence/phase5fx-80062f3c/REPORT.md`.
- Leaf 10 (GOAL 6H: the last 42E34 member `func_8005270C`, the string append
  `func_80052C08` and the GPU timer `func_800773D0`, 2026-09-19) moved the metric
  again — `funcs` 374/979→**377/980**, `c_words` 6776→**6834**, `asm_funcs`
  529→**527**, plan 803→**806** c spans, two gates `EXACT_REBUILD_GATE=PASS` with
  `sha1_orig == sha1_cand = 452fb033…`. This is the batch that also grew the union
  itself (**979→980**, `tierA` 734→735), because `func_8005270C`'s neighbours
  became reachable once the cluster was C. The same batch is the first where the
  decomp was used to *verify the port*: `PORTVERIFY_matched_leaves` in
  `pc_port/tests/test_port_verify_decomp.h` pins five port implementations against
  their now-authoritative C, with no semantic drift found. See
  `docs/evidence/phase5fy-5fz-sixhour/REPORT.md`.
- Leaf 11 (GOAL 4H: the arena push/pop pair `func_8005E8C4` / `func_8005E914`,
  2026-09-19) is the **blind** class again: plan 806 → **808** c spans and
  `EXACT_REBUILD_GATE=PASS` (`sha1_orig == sha1_cand = 452fb033…`,
  `VERIFY_SWEEP=PASS leaves=808`), while `funcs` stays 377/980 and `c_words` 6834 —
  both are reached through the menu draw path rather than the direct-call closure.
  The same batch extended the port verification to seven functions, and is the first
  time the decomp supplied information the port never had: the arena window
  `0x800A2270..0x800A22B0` and the report codes 2 (push overflow) / 3 (pop
  underflow). See `docs/evidence/phase5ga-arena-pair/REPORT.md`.
- Leaf 12 (GOAL 4H #3: the mesh colour transfer pair `func_800C6EF8` /
  `func_800C6F4C`, 21 words each, 2026-09-21) — the blind class a third time: plan
  808 → **810** c spans with `EXACT_REBUILD_GATE=PASS`
  (`sha1_orig == sha1_cand = 452fb033…`, `VERIFY_SWEEP=PASS leaves=810`), `funcs`
  still 377/980. The port counterpart in `func_800C71E4_port.c` **agrees exactly**,
  and the decomp settled a potential trap: retail's `lhu` + `blez` + `slt` sequence
  behaves as unsigned for a 16-bit count, so `count >= 0x8000` is not a signedness
  divergence between the two. See `docs/evidence/phase5gb-mesh-colour-pair/REPORT.md`.
