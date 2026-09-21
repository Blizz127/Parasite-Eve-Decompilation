# func_8008F178 — MATCHED (14 words, LINK_EXACT)

VRAM `0x8008F178`, file `0x7F978`, span `0x38`. Carve: splits `[0x7E044, asm]`;
`func_8008F1B0` starts at `0x7F9B0`.

```c
void func_8008F178(unsigned char *a0, int a1) {
    unsigned char *p = D_800B2900 + (a1 << 6);
    *(short *)(a0 + 0x5A) = (short)a1;
    func_8008F0D0(a0, p, *(int *)p);
}
```

`a1` is stored at `a0+0x5A` **before** the `sll` scales it into a `0x40`-stride
`D_800B2900` record; the record's word 0 becomes the third argument. era
`-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
