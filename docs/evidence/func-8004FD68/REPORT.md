# func_8004FD68 — MATCHED (14 words, LINK_EXACT)

VRAM `0x8004FD68`, file `0x40568`, span `0x38`. Carve: splits `[0x401A0, asm]`.

```c
void func_8004FD68(unsigned char *a0) {
    func_80052E30(1);
    func_800638D8(a0, func_80050BE8);
}
```

Same shape as `func_8004EC78` with the `func_80050BE8` handler. era
`-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
