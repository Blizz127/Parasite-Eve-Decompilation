# func_8004EC3C — MATCHED (14 words, LINK_EXACT)

VRAM `0x8004EC3C`, file `0x3F43C`, span `0x38`. Carve: splits `[0x3F17C, asm]`.

```c
void func_8004EC3C(unsigned char *a0) {
    func_80052E30(0);
    func_800638D8(a0, func_80050690);
}
```

`a0` is pinned in `$s0` across the two calls; `func_80050690` is passed as a
**function address** (HI16/LO16). era `-O2 -G0`. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
