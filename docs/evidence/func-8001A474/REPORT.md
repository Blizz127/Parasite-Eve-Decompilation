# func_8001A474 — MATCHED (13 words, LINK_EXACT)

VRAM `0x8001A474`, file `0xAC74`, span `0x38`. Carve: splits `[0xAB74, asm]`;
`func_8001A4AC` starts exactly at `0xACAC`.

```c
extern int func_80052F70(void);
int func_8001A474(unsigned char *a0) {
    int r = func_80052F70();
    *(unsigned int *)*(unsigned int *)a0 = r;
    return 1;
}
```

The `func_8005401C` twin of `func_8001A43C`; same `$s0` pointer pin and same
store-through-`*a0`. era `-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep
span exact.
