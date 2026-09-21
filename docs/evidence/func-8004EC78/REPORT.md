# func_8004EC78 — MATCHED (14 words, LINK_EXACT)

VRAM `0x8004EC78`, file `0x3F478`, span `0x38`. Carve: splits `[0x3F17C, asm]`;
`func_8004EC74` is the 4-byte `nop` filler before it.

```c
void func_8004EC78(unsigned char *a0) {
    func_80052E30(1);
    func_800638D8(a0, func_800506E8);
}
```

Slot-1 twin of `func_8004EC3C`. era `-O2 -G0`. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
