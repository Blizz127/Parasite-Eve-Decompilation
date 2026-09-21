# func_800198C4 — MATCHED (16 words, LINK_EXACT)

VRAM `0x800198C4`, file `0xA0C4`, span `0x40`. Carve: splits `[0xA010, asm]`;
`func_80019904` starts at `0xA104`.

```c
int func_800198C4(unsigned int *a0) {
    unsigned int b = *(unsigned char *)*a0;
    func_8002FAD8(D_8009D2F0, b, *(unsigned int *)a0[1], *(unsigned int *)a0[2]);
    return 1;
}
```

Four-argument call: the global object, an **`lbu`** byte from `*a0[0]`, and two
words from `a0[1]` / `a0[2]`. era `-O2 -G0`. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
