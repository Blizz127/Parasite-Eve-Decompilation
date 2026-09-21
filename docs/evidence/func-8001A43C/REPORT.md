# func-8001A43C — MATCHED (13 words, LINK_EXACT)

VRAM `0x8001A43C`, file `0xAC3C`, span `0x38`. Carve: splits `[0xAB74, asm]`;
`func_8001A474` starts exactly at `0xAC74`.

```c
int func_8001A43C(unsigned char *a0) {
    int r = func_8005401C();
    *(unsigned int *)*(unsigned int *)a0 = r;
    return 1;
}
```

`a0` is copied to `$s0` in the `jal` delay slot (a callee-saved pin: the
pointer must survive the call), then the result is stored through the reloaded
`*a0`. era `-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
