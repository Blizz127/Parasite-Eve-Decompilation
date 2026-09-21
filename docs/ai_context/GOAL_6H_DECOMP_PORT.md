# GOAL 6H — decompile + port as much as possible, and verify the port with the new decomp

Date opened: 2026-09-18. Target: 2026-09-19 (6-hour box).

Standing constraints (unchanged): the **native PC port is the deliverable**;
matching decomp is the means; recovered assembly is worth zero to the port; no
MIPS interpreter; nothing is committed (new sources are staged only); no game data
in the tree; `{SCRATCH}` = `/tmp/pe-2nd`.

## Objective

Two interleaved tracks, in priority order:

1. **Matching decomp** — keep clearing the repaired candidate queue
   (`tools/progress/port_priority.py`, currently topped by `func_80052C08` 25 w,
   `func_800773D0` 13 w), plus the two parked near-misses whose fix is now known
   (`func_8005270C` 0x58). Every leaf must pass `LINK_EXACT`, then the deep
   preflight, then `scripts/exact_rebuild.sh` (SHA-1
   `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).
2. **Verify the port with the newly decompiled information** — as leaves match,
   use the now-authoritative C to check the port's hand-written translations of the
   same functions (`pc_port/**/*_port.c`) for semantic drift, and lock the finding
   with a native test. Priority: functions the port already implements by hand.

## Scope

- in: `src/*.c` matching leaves, `configs/USA/disc1.yaml` spans,
  `configs/USA/disc1_build_profiles.json` assignments, evidence reports,
  `pc_port/` comparison + tests, docs/handoff.
- out: new pad capture (routes are bounded by the synthesized program), MIPS
  interpretation, anything requiring game data in the tree.

## Acceptance

- each landed leaf: `LINK_EXACT`, deep preflight PASS, gate PASS with an unchanged
  retail SHA-1, and an evidence report under `docs/evidence/<phase>/REPORT.md`;
- port verification: an explicit statement per checked function — "port agrees" or
  a named divergence with the test that pins it;
- docs (`ACTIVE_HANDOFF.md`, `ROUTE_COVERAGE.md`, generated status docs)
  regenerated and self-consistent (`--check-status` green);
- `git diff --check` clean, no unmatched C left in `src/`, nothing committed.

## Verification plan

```sh
export LD_LIBRARY_PATH="$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
python3 tools/analysis/era_link_check.py src/<leaf>.c <vram> <size> [flags]   # iterate
python3 tools/build/disc1_preflight.py --deep                                 # all leaves
bash scripts/split_us.sh && bash scripts/exact_rebuild.sh                     # gate
./pc_port/build/pe-native-tests                                               # port suite
```

## Non-goals

- Not a full-decompilation claim; the target is honest incremental movement.
- Not chasing compiler-flag archaeology where a source spelling is untested.

## Progress log

| leaf | size | result |
| --- | --- | --- |
| `func_8005270C` (retry, 5FY) | 0x58 | **LINK_EXACT + gate PASS** — fixed by declaring the package pointer non-small so cc1 emits the absolute address retail keeps in `$a0` |
| `func_80052C08` (5FZ) | 0x64 | **LINK_EXACT + gate PASS** — post-increment terminator scan (`while (*dst++ != 0xFF){} dst--;`) |
| `func_800773D0` (5FZ) | 0x34 | **LINK_EXACT + gate PASS** — first try at the default profile |

Totals for the box: matching c spans 804 → **806**; `funcs` 374/979 → **377/980**;
`c_words` 6776 → **6834**; `asm_funcs` 529 → **527**. Every leaf passed
`EXACT_REBUILD_GATE=PASS` with
`sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Port verification (track 2) — `pc_port/tests/test_port_verify_decomp.h`,
`TEST PORTVERIFY_matched_leaves... PASS`, full suite **1405**:

| decompiled leaf | port counterpart | verdict |
| --- | --- | --- |
| `func_80062F3C` | `func_80062D2C_port.c` (`func_80062A34`) | agrees — list head, `+0x20`/`+0x24`, NULL-on-miss |
| `func_800773D0` | `func_80076C34_port.c` | agrees — deadline/poll words |
| `func_8005270C` | `field_message_port.c` | agrees — package, sound id, args, target word |
| `func_80052764` | `battle_reward_port.c` | agrees — stop + clear |
| `func_80052C08` | `func_8004F910_port.c` | agrees (copy factored into `func_80052BCC`; retail inlines it) |

Reusable finding: three of this session's four codegen puzzles were solved by
**source spelling**, not compiler flags — `volatile int *p` (two loads through one
address), `int one = 1;` (loop constant hoisted into the entry block), the
post-increment scan with an explicit step back (loop rotation), and a non-small
declaration (absolute vs forced-absolute addressing).

## Next

- continue the queue (`port_priority.py` top) with the same spelling-first method;
- `func_80077404` is the poll side of the `func_800773D0` pair and the natural
  follow-up in that region.
