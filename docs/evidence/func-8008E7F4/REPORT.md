# func_8008E7F4 — MATCHED (19 words, LINK_EXACT)

VRAM `0x8008E7F4`, file `0x7F004`, span `0x4C` (19 words). Carve: splits
`[0x7E044, asm]`.

```c
void func_8008E7F4(unsigned char *a0, int a1) {
    int v = *(short *)(a0 + 0xE2);
    if ((unsigned)v < (unsigned)a1) func_8008E4E8(a0, a1);
    else if ((unsigned)a1 < (unsigned)v) func_8008E664(a0, a1);
}
```

**Lever — unsigned comparison selects `sltu` over `slt`.** Both compares are
**unsigned** (`sltu $v0,$v1,$a1` and the swapped `sltu $v0,$a1,$v1`); written
signed the pair becomes `slt` and 2 words differ. The second compare is the
swapped-operand form, so writing `v > a1` instead of `a1 < v` is *not*
equivalent here — it changes the emitted block layout (`greater-than` form
gives 2 mismatches after re-testing the unsigned spelling).

The function body is 19 words; the declared span `0x4C` is the true end
(`func_8008E840` starts at `0x7F050`).

Gate: `check_leaf.sh func_8008E7F4 0x8008E7F4 0x4C -O2 -G0` → `LINK_EXACT`,
deep span exact.
