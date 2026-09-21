# func_80015AB8 — MATCHED (14 words, LINK_EXACT)

VRAM `0x80015AB8`, file `0x62B8`, span `0x38`. Carve: splits `[0x3420, asm]`;
`func_80015AF0` starts exactly at `0x62F0`.

```c
int func_80015AB8(unsigned char *a0) {
    unsigned int v0 = *(unsigned int *)(a0 + 0);
    unsigned int v1 = *(unsigned int *)(a0 + 4);
    unsigned int a2 = *(unsigned int *)(a0 + 8);
    func_800677A0(*(unsigned int *)v0, *(short *)v1, *(short *)a2);
    return 1;
}
```

Three descriptor words are loaded first, then the call arguments: a **word** at
`*a0[0]` and **halfwords** at `*a0[1]` / `*a0[2]` (`lh`). era `-O2 -G0`.
Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
