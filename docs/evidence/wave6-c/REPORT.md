# wave-6 slice C — executed-path functions

Branch `agent/wave6-c`, worktree `/tmp/pe-agent-w13`, base `abbf2838`.

Baseline gate re-run first: `scripts/split_us.sh` (host) + `build_us.sh` +
`verify_us.sh` (pe-mipsel) reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (884
registered C leaves)`, `VERIFY_US=PASS`.

## Landed — 884 -> 890 (+6)

All six confirmed by a fresh complete build (commit `1603a5fc`):
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (890 registered C leaves)`, plan
`1295 spans = 890 c + 403 asm + 2 rodata`, `VERIFY_US=PASS`.

| leaf | file | size | profile |
| --- | --- | ---: | --- |
| `func_80017588` | 0x7D88 | 0x130 | `era_o2_g8` |
| `func_80050878` | 0x41078 | 0x130 | `era_o2_g8` |
| `func_8005FA3C` | 0x5023C | 0x138 | `era_o2_g8_expand_div` (new) |
| `func_8005FB74` | 0x50374 | 0x138 | `era_o2_g8_expand_div` |
| `func_8005FDF0` | 0x505F0 | 0x138 | `era_o2_g8_expand_div` |
| `func_8005FF28` | 0x50728 | 0x144 | `era_o2_g8_expand_div` |

**New profile `era_o2_g8_expand_div`**: `-O2 -G8` +
`MASPSX_EXPAND_DIV=1`. The 0x5023C decimal-emitter family performs a signed
`value / divisor` and retail contains the full checked-div sequence
(`bnez s1; break 7; ...; break 6; mflo`). Under the default maspsx the `div`
stays unchecked; `--expand-div` reproduces retail exactly. This is the `-G8`
sibling of the existing `era_o2_g0_expand_div` (`func_8003C5D8`).

**Reusable levers confirmed this slice**

- The inlined `func_8005332C` record lookup in `func_80050878` needs the row
  table as a struct (`Row { int v[3]; }`) plus an explicit `Row *table =
  D_80092234;` local so the 12-byte stride is computed and the base
  materialised before the `arg0*4` add. A flat `int[]` gave the wrong
  association; no table local gave 22 diffs.
- The four decimal emitters need the suppressed-leading-zero test written as
  the **ternary** `func_8005F874(i < digits - 1 && digit == 0 ? -1 : digit)`
  (port form), not an `if`/nested `if`: only the ternary makes the scheduler
  hoist the `slt i < digits-1` ahead of the `div` and fill the loop-back delay
  slot. An explicit `lt` local, the m2c do/while shape, and a separate
  `if (lt)` all left 14 diffs.
- `func_8005FF28`'s two prefix arms must be written as an `if/else if` chain
  (`value < 0` / `value > 0`), which cc1 then merges into the single
  `func_8005EB64` call site; `value != 0` or a computed glyph variable gives a
  different test tree.
- The `int *p = &D_8009D128; *p = D_8009D128;` idiom (documented on
  `func_800605F8`) is required in all four emitters to keep the redundant
  gp+0x3B8 self-store.

## Attempted and parked (see `parked_blockers.json` `wave6c-*`)

- `func_800542A0` (0x44AA0, 0x12C) — the inlined `func_8005332C` branch needs
  `entry` live in both `$v1` and `$a1`; cc1 uses `$v1` only. Best 38 diffs.
- `func_800C2EAC` (0xB36AC, 0x144) — cc1 cross-jumps the common case suffixes
  and balances the switch tree; retail keeps a linear chain with full per-case
  bodies. Best 32 diffs.
- `func_8006CC68` (0x5D468, 0x13C) — retail materialises the overlay base into
  `$s0` at entry and keeps the frame at 0x20; cc1 grows it to 0x28. Best 35.
- `func_80020F18` (0x11718, 0x13C) — the D_800BE830/34 clear loop's mixed
  base+offset and three-word indexed-symbol addressing. Best 38 diffs.

Stop condition reached (4 consecutive parks) with 6 of the 11 listed targets
landed. `func_800C9EA8` was not attempted (stop condition); it is a large
stack-struct assembly with four 0x10-byte frame objects.

## Final gate

Fresh `scripts/split_us.sh` + `build_us.sh` + `verify_us.sh` after the carve:
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (890 registered C leaves)`, `VERIFY_US=PASS`.
