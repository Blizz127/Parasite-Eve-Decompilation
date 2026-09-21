# func_80017E68 — MATCHED (13 words, `LINK_EXACT`)

## Target

- VRAM `0x80017E68`, file `0x8668`, span `0x34` (13 words).
- Carve: splits the former `[0x85E4, asm]` span; the function runs `0x8668`..
  `0x869C` and `func_80017E9C` starts exactly at `0x869C`.

## Semantics

```c
o = D_8009D2F0;
v = o->w98;
w = *(unsigned int *)*(unsigned int *)a0;    /* pointer-to-pointer argument */
*(unsigned int *)a0[1] = ((v & w) ^ w) < 1;  /* all bits of w already set in v */
return 1;
```

`(v & w) ^ w` is zero exactly when every bit of `w` is already set in `v`, so
the stored word is the "all requested bits present" boolean (`sltiu` after the
`xor`).

## Notes

- `a0` is a two-word descriptor: `a0[0]` is a pointer to the word read, and
  `a0[1]` is the destination pointer. Both are reached through the descriptor
  directly (no extra base register), which is what cc1 emits here.
- The AND/XOR/`sltiu` chain and the `return 1` land exactly; the single `nop`
  after the first load is retail's own load-delay fill.

## Verification

```
tools/analysis/check_leaf.sh func_80017E68 0x80017E68 0x34 -O2 -G0
  LINK_EXACT (0 word mismatches)
  disc1_preflight: PASS (deep, 754 c / 331 asm / 2 rodata)
```

- `configs/USA/disc1.yaml`: `[0x8668, c, func_80017E68]`.
- Profile: default (`era_o2_g0`).
