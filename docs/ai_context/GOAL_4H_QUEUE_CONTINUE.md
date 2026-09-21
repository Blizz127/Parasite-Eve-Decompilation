# GOAL 4H — second box: keep clearing the queue, keep verifying the port

Date opened: 2026-09-19. Target: 4-hour box.

Standing constraints (unchanged): the **native PC port is the deliverable**;
matching decomp is the means; recovered assembly is worth zero to the port; no
MIPS interpreter; nothing is committed (new sources are staged only); no game data
in the tree; `{SCRATCH}` = `/tmp/pe-2nd`. The working tree carries a large
pre-existing staged/untracked set from other workstreams — leave it alone and only
touch paths this goal names.

## Where the previous box left off

`docs/ai_context/GOAL_6H_DECOMP_PORT.md` — three leaves landed
(`func_8005270C`, `func_80052C08`, `func_800773D0`), two gates PASS,
`funcs` 377/980, 806 matching c spans, and the port verified against the new decomp
via `PORTVERIFY_matched_leaves`. Method that worked repeatedly: **fix the source
spelling before reaching for a compiler flag.**

## Objective

1. Continue the queue (`tools/progress/port_priority.py`) from its top —
   `func_8005E8C4` — and take the natural neighbours: `func_80077404` is the poll
   side of the `func_800773D0` timer pair already matched.
2. Keep the port verification honest and growing: every leaf that lands, check the
   port's counterpart against the new C and extend
   `pc_port/tests/test_port_verify_decomp.h` where a counterpart exists.
3. Keep docs/handoff and the generated status docs self-consistent.

## Acceptance

- landed leaves: `LINK_EXACT`, deep preflight PASS, `EXACT_REBUILD_GATE=PASS` with
  retail SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` unchanged, evidence report;
- port verification: an explicit per-function verdict, no silent "looks fine";
- `git diff --check` clean; no unmatched C in `src/`; nothing committed;
- if the port suite fails for a reason outside this goal's paths (e.g. another
  workstream's in-flight test edits), report it rather than "fixing" it.

## Verification plan

```sh
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
python3 tools/analysis/era_link_check.py src/<leaf>.c <vram> <size> [flags]
python3 tools/build/disc1_preflight.py --deep
bash scripts/split_us.sh && bash scripts/exact_rebuild.sh
./pc_port/build/pe-native-tests
```

## Progress log

| item | result |
| --- | --- |
| `func_8005E8C4` push (5GA) | **LINK_EXACT + gate PASS** — arena push; locals-before-stores, plus a non-small declaration for the arena bound so its address is absolute |
| `func_8005E914` pop (5GA) | **LINK_EXACT + gate PASS** — the pop counterpart, same two devices |
| `func_80077404` | not attempted in this box (poll side of the timer pair) |
| port verification extension | `func_8005E8C4`/`func_8005E914` vs `func_8005EED4_port.c`: **agrees**. The only difference is the port eliding calls to the empty `func_800527C0` stub — behaviourally nil, and consistent with the port's own note elsewhere. The decomp establishes what was previously unrecorded: the arena window `0x800A2270..0x800A22B0` and the report codes **2 = push overflow, 3 = pop underflow**. |

Box totals: matching c spans 806 → **808**; `funcs` stays **377/980** (these two sit
outside the direct-call closure, so the plan count and the gate are the evidence);
`EXACT_REBUILD_GATE=PASS` with the retail SHA-1 unchanged; port suite **1405/1405**
with `PORTVERIFY_matched_leaves` now covering seven functions.

Method notes worth carrying forward: five of this session's six codegen puzzles were
solved by **source spelling** rather than compiler flags — `volatile int *p`,
`int one = 1;`, the post-increment scan with an explicit step back, a non-small
declaration for an address that must come out absolute, and reading source words
into locals before the stores.
