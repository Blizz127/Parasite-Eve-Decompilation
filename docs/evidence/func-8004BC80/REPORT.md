# func_8004BC80 — MATCHED (13 words, LINK_EXACT)

VRAM `0x8004BC80`, file `0x3C480`, span `0x34`. Carve: splits the former
`[0x3BD84, asm]` span; `func_8004BCB4` starts exactly at `0x3C4B4`.

```c
void func_8004BC80(void) {
    unsigned char *p = (unsigned char *)func_80062D2C(0x16, 0, 0, 0);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004BCB4;
}
```

Four zero/low arguments are materialized before the `jal`; the returned handle
gets `func_8004BCB4` installed at `+0x30` as a **function-address** store
(HI16/LO16 reloc). era `-O2 -G0`, default profile. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
