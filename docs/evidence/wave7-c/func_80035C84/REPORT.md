# wave7-c — func_80035C84 (executed-path C leaf)

**Result: LANDED.** Commit `4b6f0394` on `agent/wave7-c`. Count transition
**903 -> 904** (the shared commit also lands `func_80066800`, 904 -> 905).

## Identity

| field | value |
| --- | --- |
| function | `func_80035C84` |
| file offset | `0x26484` |
| size | `0x180` (96 words) |
| VRAM | `0x80035C84` |
| splat source | `asm/disc1/25838.s` |
| profile | **`era_o2_g8_aspsx_230`** (`-O2 -G8` + `ERA_ASPSX_VER=2.30`); register in `configs/USA/disc1_build_profiles.json` |

## Semantics

Per-actor pose snapshot, view-code publish and motion integrate; returns the
updated `+0x2C`. Snapshots the pose (`+0x28/+0x2C/+0x30` and the `+0x38`
halfwords into `+0x40..`/`+0x50..`), republishes via `func_800361F4`, and while
`D_8009D2E8` bit 0 is clear derives the view code from
`(*D_8009D254)->0x4C` (`& 0xC0 == 0x80` -> `0x11`) into a stack local and calls
`func_8003999C(a, D_800943C0, &code)`. Then integrates the
`+0x88/+0x8C/+0x90` delta (when `+0x98` bit 1 is set), the `+0x78/+0x7C/+0x80`
delta and the `+0x58/+0x5C/+0x60` delta into the pose.

Immediate predecessor of the already-matched `func_80035E04`, which shares the
identical struct layout and tail.

## Fresh-build authority

```
EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (905 registered C leaves; 904 with only this leaf)
VERIFY_US=PASS
```

`try_leaf.py src/func_80035C84.c 0x26484 0x180 --flags "-O2 -G8"
--env ERA_ASPSX_VER=2.30`: `WORDS MATCH`.

## Levers

- `D_8009D2E8` is the gp+0x578 scalar; `D_8009D254` is the gp+0x4E4
  pointer-to-pointer, so it is declared `int **` and read as
  `*(int *)((char *)*D_8009D254 + 0x4C)` (retail does the extra
  `lw $v0,0($v0)`).
- `D_800943C0` must stay **absolute** via an incomplete array (`extern int
  D_800943C0[];`) — it is larger than the `-G8` small-data window.
- The view-code mask has to be staged in a **local** before the unconditional
  `sp10 = a->f0E`:
  `int v = *(int *)((char *)*D_8009D254 + 0x4C); sp10 = a->f0E; if ((v & 0xC0)
  == 0x80) sp10 = 0x11;`. Referring to `a->f0E` first (or adding a `rec` local)
  reorders the pointer chain and costs 8-63 words.
- The final `+0x2C` update result must be kept in a local and returned:
  `r = a->f2C + a->f5C; a->f2C = r; a->f30 += a->f60; return r;`. Plain
  `return a->f2C;` makes cc1 emit a fresh `lw` instead of reusing `$v0`
  (8-word diff, +16 bytes).

## Divergence history

Best of a 3x3 guard/tail matrix: `G4_T1` = 8 diffs (only the trailing reload).
The returned-local tail (`G4_T11`) closed it.
