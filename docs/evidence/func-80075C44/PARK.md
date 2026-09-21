# `func_80075C44` — PARK (boolean-fold residual)

Status: **PARKED** (not counted). VRAM `0x80075C44`..`0x80075C6C`
(file `[0x66444,0x6646C)`, `0x28` bytes = 10 words).

## Semantics

Third of the rectangle-command flag-word setters (twins `func_80075C6C`
matched, `func_80075C44` here):

```text
a0[3] = 2;
*(int *)(a0 + 4) = (a1 ? 0xE6000002 : 0xE6000000) | (a2 != 0);
*(int *)(a0 + 8) = 0;                 ; store in the jr delay slot
```

Retail bytes:

```text
80075c44  li    v0,2
80075c48  sb    v0,3(a0)
80075c4c  beqz  a1,.L80075C58
80075c50  lui   v1,0xe600
80075c54  ori   v1,v1,2
80075c58  sltu  v0,zero,a2            ; v0 = (a2 != 0)
80075c5c  or    v0,v1,v0
80075c60  sw    v0,4(a0)
80075c64  jr    ra
80075c68  sw    zero,8(a0)
```

## Exact residual (era `-O2 -G0`)

`era_leaf_match.sh src/func_80075C44.c 0x80075C44 0x28 -O2 -G0`
-> `SIZE_MISMATCH C=0x30 ROM=0x28`, `MISMATCHES=7` (the `j` target under the
defsym link makes it `mismatches=1`: `sw zero,8(a0)` vs `jr ra`).

Retail keeps the `(a2 != 0)` boolean in `$v0` via `sltu` and ORs into `$v1`
(the branch-arm constant register). cc1 instead folds the boolean into the
constant select and branches on `a2`:

```text
80075c50  lui   v0,0xe600            (retail: v1)
80075c54  ori   v0,v0,2
80075c58  beqz  a2,.L80075C64        (extra branch, +1 word)
80075c5c  nop
80075c60  ori   v0,v0,1
80075c64  sw    v0,4(a0)
80075c68  jr    ra
80075c6c  sw    zero,8(a0)
```

The extra `beqz` is the observed `C=0x30` vs `ROM=0x28`.

Shapes tried (all `SIZE_MISMATCH`, 7–8 mismatches, none byte-exact):
`(a1 ? K|2 : K) | (a2 != 0)` with `unsigned int`/`int` `a2`, `(a2 != 0) | (...)`
operand swap, `K | (a1 ? 2 : 0) | (a2 ? 1 : 0)`, `((a1 ? 2 : 0) | (a2 != 0)) + K`,
`if (a1) v |= 2;` statement form. Every shape either materializes the `2` bit
inside the `a1` arm (retail does too) but then branches again on `a2`, or emits
`(a2 ? 1 : 0)` as a select. cc1 has no expression that yields
`sltu $v0, $zero, $a2` when the constant already lives in the OR destination.

## Next lever to try

This is the same class as `func_8006F9F0`'s rematerialization residual: a
`cse`/`combine` choice that folds `!= 0` into a select. A maspsx-side rewrite
would have to recognize `beqz a2` + `ori` and fold back into `sltu/or`, which
is not a delay-slot reorder and is not currently warranted by a single leaf.

## Registration

- `src/func_80075C44.c` is **not** present (no unmatched C may live in `src/`).
- Span remains asm in the `0x655C0` region; the matching-C count is unaffected.
