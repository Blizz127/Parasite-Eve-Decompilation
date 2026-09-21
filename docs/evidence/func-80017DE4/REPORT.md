# func_80017DE4 — MATCHED (15 words, LINK_EXACT)

VRAM `0x80017DE4`, file `0x85E4`, span `0x3C`. `func_80017E20` starts at
`0x8620`, immediately after.

```c
int func_80017DE4(unsigned char *a0) {
    signed char b = (signed char)func_80037864();
    *(unsigned int *)*(unsigned int *)a0 = b;
    return 1;
}
```

The byte result is sign-extended with the retail `sll $v1,24` / `sra $v1,24`
pair (the intermediate is never narrowed in the register), then stored through
the pointer at `*a0`. era `-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`,
deep span exact.
