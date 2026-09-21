# GOAL 4H #3 — keep the queue moving and the port checked

Date opened: 2026-09-21. Target: 4-hour box.

Standing constraints (unchanged): the **native PC port is the deliverable**;
matching decomp is the means; recovered assembly is worth zero to the port; no MIPS
interpreter; nothing is committed (new sources are staged only); no game data in the
tree; `{SCRATCH}` = `/tmp/pe-2nd`. The worktree carries a large pre-existing
staged/untracked set from other workstreams — touch only paths this goal names.

## Where the last box left off

`docs/ai_context/GOAL_4H_QUEUE_CONTINUE.md`: the arena push/pop pair
(`func_8005E8C4` / `func_8005E914`) landed and gate-passed, bringing the plan to
**808 c spans**, with the port verification extended to seven functions and the
decomp contributing new facts (arena window `0x800A2270..0x800A22B0`, report codes
2 = push overflow / 3 = pop underflow).

## Objective

1. Work the queue from the top: `func_800C6EF8` and `func_800C6F4C` are adjacent
   21-word leaves (likely a twin pair), then `func_8005B8A8` (29),
   `func_80064C80` (34), `func_80059EC8` (16).
2. Keep using the method that has worked: try **source spelling** first
   (`volatile int *p`, named locals, non-small declarations, post-increment loops),
   flags and maspsx knobs only after that.
3. For every leaf that lands, check the port's counterpart against the new C and
   extend `pc_port/tests/test_port_verify_decomp.h` where one exists.
4. Keep the generated status docs and the handoff self-consistent.

## Acceptance

- landed leaves: `LINK_EXACT`, deep preflight PASS, `EXACT_REBUILD_GATE=PASS` with
  retail SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` unchanged, evidence report;
- port verdicts stated explicitly per function (agrees / named divergence), never
  implied;
- `git diff --check` clean, no unmatched C in `src/`, nothing committed;
- problems outside this goal's paths (other workstreams' in-flight edits) get
  reported, not "fixed".

## Progress log

| item | result |
| --- | --- |
| `func_800C6EF8` (5GB) | **LINK_EXACT + gate PASS** — mesh → `D_800E2370` word transfer; the counter increment must sit in the loop body before the copy |
| `func_800C6F4C` (5GB) | **LINK_EXACT + gate PASS** — the mirror transfer, same fix |
| further queue items | `func_800C6FA0` (0xF8) is the effect-level sibling of this pair — the natural next leaf, not attempted in this pass |
| port verification extension | `func_800C6EF8`/`func_800C6F4C` vs `func_800C71E4_port.c`: **agrees exactly** (source derivation, per-iteration count reload, word granularity; no signed/unsigned trap at `count >= 0x8000`). `PORTVERIFY_matched_leaves` now covers nine functions. |

Box totals: matching c spans 808 → **810**; `funcs` stays **377/980** (these two sit
outside the direct-call closure); `EXACT_REBUILD_GATE=PASS` with the retail SHA-1
unchanged; port suite **1405/1405**.

Method tally — six source-spelling fixes for six codegen puzzles across the two boxes:
`volatile int *p`, `int one = 1;`, the post-increment scan with an explicit step
back, a non-small declaration where an address must come out absolute, source words
read into locals before the stores, and an in-body counter increment.
