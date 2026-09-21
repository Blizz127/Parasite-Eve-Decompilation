# func_80067CBC — MATCHED (23 words, `LINK_EXACT`)

## Target

- VRAM `0x80067CBC`, file `0x584BC`, span `0x5C` (23 words).
- On-path fan-in **11** (`route_coverage.py`) — one of the highest-fan-in
  remaining asm leaves in the `0x67xxx` state cluster.
- Carve: `0x584BC`..`0x58518`; previous span `[0x58374, asm]` (from
  `func_80067B40` at `0x58340`) shortens to `0x58374`..`0x584BC`, and a new
  `[0x58518, asm]` resumes before `func_80067D18` (`0x58518`, `0x104`).
  No neighbor is swallowed — `func_80067D18` starts exactly at `0x58518`.

## Semantics

Status-word flag setter on `D_800BCF88`:

```c
v = D_800BCF88;
n = v | 0x1000;
D_800BCF88 = n;
D_800BCF88 = (n & 0x2000) ? (n & ~0x2000) : (v | 0x3000);
D_800BCF88 = (D_800BCF88 & 0xFFFF3FFF) | 0x4000;   /* clear 0x8000/0x4000, set 0x4000 */
return 0;
```

Same global and bit vocabulary as the already-matched `func_80067B40`
(`D_800BCF88 = (D_800BCF88 & ~0xC00) | 0x400`), which is the neighboring
C leaf.

## The lever: dual pinned pointer bases

A bare-symbol form makes cc1 keep the address absolute (`lui $at`/`lw %lo`),
and a single pointer local gives the **right shape but the wrong base
registers**: retail keeps the head base in `$a1` and rematerializes a *fresh*
base in `$v1` for the tail. One shared local keeps the same register for both
halves.

Declaring **two distinct pointer locals pinned to the retail registers**

```c
register unsigned int *p asm("$5");
register unsigned int *q asm("$3");
...
p = &D_800BCF88;   /* head: load, or, two stores */
...
q = &D_800BCF88;   /* tail: load, and/or, store */
```

reproduces both bases exactly and closes all 7 remaining mismatches.

### Mismatch ladder (all `-O2 -G0`)

| form | mismatches |
|---|---|
| bare symbol | 23 |
| single unpinned pointer local | 14 |
| single pointer pinned `$5` | 7 |
| **two pointers pinned `$5` / `$3`** | **0** |

Note the pinned pair must be `($5 head, $3 tail)`; `($5, $2)` stays at 7 — the
tail base is `$v1`, and `$2` is the value/`and` result register.

## Verification

```
tools/analysis/check_leaf.sh func_80067CBC 0x80067CBC 0x5C -O2 -G0
  == link-level check ==
  linked .text 96 bytes, target 0x5c, word mismatches=0, nonzero_pad=0
  LINK_EXACT
  == deep span-size check ==
  disc1_preflight: PASS (deep, 752 c / 331 asm / 2 rodata)
  == [func_80067CBC] OK (link exact + span size exact) ==
```

- `configs/USA/disc1.yaml`: registered as `[0x584BC, c, func_80067CBC]`
  with a resumed `[0x58518, asm]`.
- Profile: default (`era_o2_g0`) — no profile entry needed.
