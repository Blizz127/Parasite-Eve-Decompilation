# func_80083790 — MATCHED (14 words, LINK_EXACT)

VRAM `0x80083790`, file `0x73F90`, span `0x38`. Carve: splits `[0x73DC0, asm]`;
`func_800837C8` starts exactly at `0x73FC8`.

```c
int func_80083790(unsigned char *a0) {
    unsigned int v = ((*(unsigned char *)(a0 + 0xE3) + 1) >> 1) << 2;
    unsigned int b = *(unsigned char *)(a0 + 0xE9);
    unsigned int off = ((b * 5 + 3) & 0xFFC) + 4;
    return (int)((unsigned char *)(v + off) + *(unsigned int *)(a0 + 0xEC));
}
```

Two packed record offsets summed against the base at `a0+0xEC`: the divisor-2
count `((hE3+1)>>1)<<2` plus the `0x11C`-stride block offset
`((5*hE9+3) & 0xFFC) + 4`. **Lever — the addition is split so the low half
folds**: writing the two terms as separate `unsigned int` locals and adding them
gives retail's `addu $v0,$v0,$v1`. era `-O2 -G0`. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
