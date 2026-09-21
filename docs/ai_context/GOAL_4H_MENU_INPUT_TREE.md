# Goal: 4-hour native-port continuation — field-menu input tree 0x8004AE1C

Paste into the prompt:

```
/goal Continue the native PC port for ~4 hours: translate the field-menu input
tree rooted at 0x8004AE1C (the page handler func_8004AD9C installs), wire its
sub-page constructors/handlers/draws into the native dispatchers, keep the
plain and pilot disc-1 route runs green, and push the plain-route
unresolved-boundary past PE_MenuInputCallback. No matching-C parks and no MIPS
interpreter count as progress.
```

## Goal kind

code-change

## Objective (one sentence)

The disc-1 field menu's Items/Escape input tree runs natively, the plain boot →
Day-2 route no longer stops on `PE_MenuInputCallback` (raw `0x8004AE1C`), and the
two coverage numbers plus the disc-1 baseline stay honest.

## Where this continues from

The prior 4-hour goal (`GOAL_4H_PORT_CONTINUE.md`) ended with:
- plain route `frames=61623 stop=unresolved-boundary story=0x09 persist1=0x0A
  token=0xA8000148` (m0002i), 45/57, only boundary stub
  `BOOTSTRAP_RET PE_MenuInputCallback x1` — raw guest callback `0x8004AE1C`
  (`[MENU] Unported input callback 8004AE1C`, `route_4ff30.log`);
- `native_c=258/979`, `funcs=372/979` (indirect-subtree leaf, by design —
  `ROUTE_COVERAGE.md` §"Metric scope (2026-09-18)").

## Scope (the tree)

```
func_8004AD9C (already native) stores window+0x2C = 0x8004AE1C, list+0x30 = 0x8004FF30
func_8004AE1C (72 w) input handler, jump table jtbl_80011034 (asm/disc1/data/800.rodata.s:1310)
  case 0 → func_8004AF3C  items   sub-page ctor, window+0x2C=0x8004AFA4, list+0x30=0x8004FF58
  case 1 → func_8004B03C  escape  sub-page ctor, window+0x2C=0x8004B0A4, list+0x30=0x8004FF80
  case 2 → func_8004B13C  equip   sub-page ctor (2 lists 0x2E/0x31), handlers/draws 0x8004B214/
                                  0x8004B394/0x8004B534/0x8004B55C
  case 3 → func_8004B584  modal   sub-page ctor, window+0x2C=0x8004B650, list+0x30=0x8004B5DC
  case 4/5 → func_8005D994(sel-4) close path (needs func_8005247C)
```

## Acceptance criteria

1. `func_8004AE1C` is a native leaf (`pc_port/game/boot/func_8004AE1C_port.c`)
   dispatched from `func_80063E0C_port.c`'s `menu_callback`, and its jump-table
   cases match the retail table exactly (0..3 → the four constructors; 4/5 →
   the close path; ≥6 → the shared `func_800525EC`/return-1 tail, which is
   reached **once** for ≥6 and **twice** for 4/5 — preserve that).
2. The four sub-page constructors and the reachable sub-handlers/draws are
   native: at minimum `func_8004AF3C`, `func_8004B03C`, `func_8004B13C`,
   `func_8004B584`, `func_8004AFA4`, `func_8004B0A4`, `func_80050C70`,
   `func_80050CB4`, and the four draw wrappers `func_8004FF58`, `func_8004FF80`,
   `func_8004B534`, `func_8004B55C` (hand adapters, same class as
   `func_8004FF30`: a bare `func_*` used as a value is a guest address, rules
   E5/E6 of `gen_decomp_ports.py`).
3. Anything still untranslated in the tree is a **named** boundary arm in
   `menu_callback` / `menu_draw_callback` (not the generic default), so a route
   stop names the exact residual.
4. Focused test `DAY2_field_menu_input_4ae1c` passes, plus a full native suite
   run with no new failures.
5. Plain disc-1 route: the boundary moves past `PE_MenuInputCallback`
   (0x8004AE1C); pilot route unchanged (55/57, frame-limit, all HOST_ADAPTED).
6. Two coverage numbers re-printed; `native_c` must not drop. An indirect
   subtree leaf does not move `native_c` — say so, do not fudge the metric.
7. No game data in git; no MIPS interpreter/recompiler; `docs/legal.md` holds.

## Verification plan

1. `source /tmp/pe-tools/env.sh; cmake --build pc_port/build -j8`
2. `PE_TEST_FILTER=DAY2_field_menu ./pc_port/build/pe-native-tests` (new + old
   menu tests PASS) → `{SCRATCH}/native_menu.log`.
3. Full suite `./pc_port/build/pe-native-tests` → `{SCRATCH}/native_full.log`
   (baseline 1403 run / 1403 passed).
4. `python3 tools/analysis/route_coverage.py --quiet --no-history` →
   `{SCRATCH}/route_coverage.txt`; keys `disc=1 native_c= funcs=`.
5. Plain route `./pc_port/build/pe-route-boot-day2-tests` →
   `{SCRATCH}/route_menu.log`; pilot
   `PE_ROUTE_REWARD_PILOT=1 ./pc_port/build/pe-route-boot-day2-tests` →
   `{SCRATCH}/route_menu_pilot.log`.
6. `git diff --check`; no ISO/BIN/CUE/CHD/`SLUS_*` staged.

## Non-goals

- 100% matching decomp; matching parks stay parked.
- Translating `func_8005D994`/`func_8005247C`, `func_8004B214`,
  `func_8004B394`, `func_8004B650`, `func_8004B5DC`, `func_80050438` — these are
  named boundary arms unless time allows.
- Day-2 terminus, M34, pixel-perfect FMV.
- MIPS interpreter or dynarec.
- Committing game data.

## Time box

About four hours. Prefer a smaller green increment over an unfinished megaleaf.
When the box is exhausted, ship what is green, update
`docs/ai_context/ACTIVE_HANDOFF.md`, and leave one next command.

## Outcome (2026-09-18, closed)

All acceptance criteria met; `{SCRATCH}` = `/tmp/pe-2nd`.

| Item | Result |
| --- | --- |
| `func_8004AE1C` | native (`func_8004AE1C_port.c`), jump table matched to `jtbl_80011034`, dispatched from `menu_callback` |
| Sub-page tree | `AF3C`/`B03C`/`B13C`/`B584`, `AFA4`/`B0A4`, `4FF58`/`4FF80`/`4B534`/`4B55C`, `50C70`/`50CB4` native |
| Untranslated leaves | named arms: `PE_MenuInputCallback_8004B214`/`_8004B394`/`_8004B650`, `PE_MenuDrawCallback_8004B5DC`, `PE_MenuDrawCell_80050438`, `PE_MenuInputClose` |
| Focused test | `DAY2_field_menu_input_4ae1c` PASS (all 6 jump-table cases, both confirms, cancel, per-cell dispatch, named boundaries) |
| Full suite | 1404 run / 1404 passed / 0 failed / 0 skipped (`native_full2.log`) |
| Plain route | `frames=62000 stop=frame-limit story=0x09 persist1=0x0A token=0xA8000148`, 45/57, **no unresolved boundary** (`route_menu2.log`) |
| Pilot route | `frames=62000 stop=frame-limit story=0x68 persist1=0x20 token=0xA80031C8`, 55/57, all HOST_ADAPTED (`route_menu_pilot.log`) |
| Coverage | `disc=1 funcs=372/979 native_c=258/979` unchanged (indirect subtree, honest) |
| Generator | `gen_decomp_ports.py --verify`: 269 TUs, 0 drift |

The boundary history is the evidence: `PE_MenuInputCallback` (61623) →
`PE_MenuDrawCallback`/`0x80050C70` (61623, one level deeper) → none. Next
command: drive milestone 55 ("normal item use restores 45 HP") — the Items
confirm `func_8004AFA4` is native, so the gap is downstream of it.

## Follow-up (2026-09-18): the named residual is closed

The "next command" above was run. The gap was **not** downstream of
`func_8004AFA4` — the port already performed the whole Items/Use chain natively
(`func_80044B0C → func_80057834 → func_800516B4 → func_80023E14` +
`func_80057D30`, traced at frame 58044: item7 consumed, HP 33 → max 53). The
harness could not see it because two milestone pins and the endpoint pin encoded
route-specific values from an older m0031i run (absolute HP 45 = that run's max
HP; gun-slot sampled on the wrong frame; endpoint token/PC/persist[1]).

Re-expressed the pins against their documented invariants and the endpoint
against the durable state; the pilot now reports **57/57 milestones, PASS**
(`{SCRATCH}/route_m55c.log`), plain unchanged at 45/57. See
`docs/generated/DISC1_GAMEPLAY_BASELINE.md` and the 2026-09-18 CURRENT section of
ACTIVE_HANDOFF for the exact substitutions and evidence. This changed the test
oracle only — no port code, no new native leaf, `native_c` unchanged.
