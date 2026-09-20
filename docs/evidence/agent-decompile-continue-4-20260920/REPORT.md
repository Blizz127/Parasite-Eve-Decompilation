# agent/decompile-continue-4 — 12 new byte-exact matching C leaves (679 -> 691)

Branch `agent/decompile-continue-4`, worktree `/tmp/pe-agent-decomp5`, base
`df538f07`. Every leaf below is proved by the rebuild harness, not by
`try_leaf` alone.

## Cumulative verification

```text
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp5 && bash scripts/build_us.sh'
  RESULT: EXACT MATCH
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  Matching claim: YES (691 registered C leaves)

distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp5 && bash scripts/verify_us.sh'
  VERIFY_US=PASS
  PASS all 691 packed C spans equal retail
```

## Leaves

| function | file | size | shape | era flags |
|---|---:|---:|---|---|
| `func_80015AB8` | 0x62B8 | 0x38 | 3-word reader-call wrapper (int/short/short) -> `func_800677A0`; return 1 | `-O2 -G0` |
| `func_80018B30` | 0x9330 | 0x38 | 3-word reader-call wrapper (short x3) -> `func_800679C4`; return 1 | `-O2 -G0` |
| `func_800193D8` | 0x9BD8 | 0x38 | 3-word reader-call wrapper (int x3) -> `func_80065AD4`; return 1 | `-O2 -G0` |
| `func_8001A43C` | 0xAC3C | 0x38 | `func_8005401C()` result stored through `*a0[0]`; return 1 | `-O2 -G0` |
| `func_8001A474` | 0xAC74 | 0x38 | `func_80052F70()` result stored through `*a0[0]`; return 1 | `-O2 -G0` |
| `func_800504BC` | 0x40CBC | 0x38 | `func_8005E8A4(0,1)` then `func_80064C54(a0+0x33)` | `-O2 -G0` |
| `func_800773D0` | 0x67BD0 | 0x34 | `D_80095888 = VSync(-1)+0xF0; D_8009588C = 0` | `-O2 -G0` |
| `func_8007DE40` | 0x6E640 | 0x38 | `func_80072714` / `func_8007E324` / `func_80073C74(0)` / `func_80072724` | `-O2 -G0` |
| `func_8007DFE0` | 0x6E7E0 | 0x30 | `7E1B4` / `73C74(0)` / `7E204`; return 1 | `-O2 -G0` |
| `func_8007E010` | 0x6E810 | 0x38 | `7E218` / `7E1C4` / `7E0C0`; `D_8009B4AC = 0` | `-O2 -G0` |
| `func_8007E0C0` | 0x6E8C0 | 0x38 | critical section around `7E1F4(1, D_800A34B0)`; return 1 | `-O2 -G0` |
| `func_80080F64` | 0x71764 | 0x34 | mode-2 forwarder `func_80081D74(func_80080F98, -1)` | `-O2 -G0` |

Each leaf has `src/<name>.c` with the matching-authority SHA-1 in its header and
a `c` carve in `configs/USA/disc1.yaml`.

## Honest non-matches (not registered)

All were triaged with `tools/analysis/try_leaf.py` and differ by exactly the
final two words (retail `jr $ra` with `addiu $sp,$sp,N` in the delay slot vs
cc1's `addiu $sp` before `jr`), which no flag rung tried moves:

- `func_800755BC` (0x65DBC, 0x34) — memcpy wrapper to `D_8009575C`
- `func_80075AE8` (0x662E8, 0x34) — memcpy wrapper to `D_800957B8`
- `func_80075B4C` (0x6634C, 0x38) — task-record init
- `func_8007DD74` (0x6E574, 0x34) — `func_8007DDC4` + `func_8007DDB4(a0,0x3F,0)`
- `func_800828F4` (0x730F4, 0x38) — function-pointer global call
- `func_8007DEC0` / `func_8007DF50` (0x6E6C0 / 0x6E750, 0x90) — 4-arg SDK wrappers

Also re-confirmed non-matching: `func_8008594C` (signed/unsigned compare and
load-hoist layout) and `func_80019260` (store/return scheduling).

## Build hygiene notes

- Run the build INSIDE the distrobox (`distrobox enter pe-mipsel -- ...`); a
  host-side run spawns one container per `as` call (~10x slower).
- `scripts/split_us.sh` is the fast YAML-geometry validator; a wrong resume
  offset fails there before any compile.
- `scripts/verify_us.sh` requires the new `src/*.c` to be `git add`ed and
  `docs/generated/DISC1_MATCHING_STATUS.md` refreshed via
  `python3 tools/build/disc1_plan.py --write-status`.
