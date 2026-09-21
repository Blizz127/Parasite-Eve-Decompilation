# func_80019450 — MATCHED (13 words, LINK_EXACT)

VRAM `0x80019450`, file `0x9C50`, span `0x34`. Carve: splits `[0x9C50, asm]`
(`func_80019484` follows at `0x9C84`).

```c
int func_80019450(unsigned int **a0) {
    unsigned char *p = D_8009D2F0;
    int n = *(short *)(p + 0x224) << 1;
    int v = *(int *)*a0;
    unsigned char *q = *(unsigned char **)(p + 0x1B4);
    *(short *)(q + 0x14) = (short)((n * v) >> 16);
    return 1;
}
```

**Lever — the parameter's element type controls `lw` vs `lbu`.** Declaring the
parameter `unsigned char *a0` makes cc1 read `**a0` as a byte (`lbu $v1,0($v1)`)
because the pointee is `char`; the retail `lw $v1,0($v1)` needs `int **` (or
`unsigned int **`) so the doubly-dereferenced element is word-sized. One word
mismatch either way.

Gate: `check_leaf.sh func_80019450 0x80019450 0x34 -O2 -G0` → `LINK_EXACT`,
deep span exact.
