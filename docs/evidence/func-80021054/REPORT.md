# func_80021054 — MATCHED (11 words, LINK_EXACT)

VRAM `0x80021054`, file `0x11854`, span `0x2C`. Carve: splits the former
`[0x11718, asm]` span; `func_80021080` starts exactly at `0x11880`.

```c
int func_80021054(void) {
    unsigned int *obj = (unsigned int *)D_8009D278[0];
    if (*(int *)((unsigned char *)obj + 0x4C) & 0x10000) return -1;
    return D_8009CE3C;
}
```

The signed byte `D_8009CE3C` is **`$gp`-relative** (`lb 0xCC($gp)`), while the
object pointer and the `0x10000` mask are absolute. That mixed addressing is
exactly the **`-O2 -G8`** profile (registered under `era_o2_g8`); at `-O2 -G0`
the byte is absolute and 4 words differ.

Gate: `check_leaf.sh func_80021054 0x80021054 0x2C -O2 -G8` → `LINK_EXACT`,
deep span exact.
