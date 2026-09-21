# func_800192DC — MATCHED (12 words, LINK_EXACT)

VRAM `0x800192DC`, file `0x9ADC`, span `0x30`. Carve: splits the former
`[0x9ADC, asm]` span; `func_8001930C` starts exactly at `0x9B0C`.

```c
int func_800192DC(unsigned int *a0) {
    unsigned int x = *(unsigned int *)*a0;
    func_8006FC18(x, D_8009D2F0, 0);
    return 1;
}
```

`a0[0]` is dereferenced to the call argument; `D_8009D2F0` is the second
argument; `0` is materialized in the `jal` delay slot. era `-O2 -G0`, default
profile. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
