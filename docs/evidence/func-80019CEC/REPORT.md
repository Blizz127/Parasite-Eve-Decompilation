# func_80019CEC — MATCHED (14 words, `LINK_EXACT`)

## Target

- VRAM `0x80019CEC`, file `0xA4EC`, span `0x38` (14 words).
- Carve: splits the former `[0xA44C, asm]` span; the function runs `0xA4EC`..
  `0xA524` and `func_80019D24` starts exactly at `0xA524`.

## Semantics

```c
o = D_8009D2F0;
o->w28 = o->h21C << 16;   /* promote halfwords to the high half */
o->w2C = o->h21E << 16;
o->w30 = o->h220 << 16;
return 1;
```

Same global and halfword-promotion vocabulary as the neighboring matched
`func_80019C04` / `func_80019C28`.

## The lever: source statement order

The natural grouping (word 0x28, then 0x2C, then 0x30) is **not** what matches;
the retail ordering of the three loads/stores is reproduced by writing
**0x28, then 0x30, then 0x2C**. cc1 interleaves the `lh`/`sll`/`sw` chains, so
the written order directly determines the emitted interleave.

| statement order | mismatches |
|---|---:|
| `28, 30, 2C` | 4 |
| `28, 2C, 30` (locals) | 4 |
| `30, 2C, 28` | 4 |
| **`28, 2C, 30`** | **0** |

Note this is a plain permutation of independent stores — a load-bearing
ordering with no aliasing or side effects involved.

## Verification

```
tools/analysis/check_leaf.sh func_80019CEC 0x80019CEC 0x38 -O2 -G0
  LINK_EXACT (0 word mismatches)
  disc1_preflight: PASS (deep, 754 c / 331 asm / 2 rodata)
```

- `configs/USA/disc1.yaml`: `[0xA4EC, c, func_80019CEC]`.
- Profile: default (`era_o2_g0`).
