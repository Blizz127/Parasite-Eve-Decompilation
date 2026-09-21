# func_8008FC28 — 0x50 bytes, LINK_EXACT

Retail VRAM `0x8008FC28` (file offset `0x80428`), in `asm/disc1/80428.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Two-field cursor read: a byte stored to `+0x7E` but replaced by `0x100` when it
is zero, then a second cursor walk whose sign-extended byte lands in `+0xE4`.

```c
void func_8008FC28(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x7E) = c;
    if (c == 0)
        *(unsigned short *)(a0 + 0x7E) = 0x100;
    {
        unsigned char *q = *(unsigned char **)a0;
        unsigned char d;
        *(unsigned char **)a0 = q + 1;
        d = *q;
        *(unsigned short *)(a0 + 0xE4) = (signed char)d;
    }
}
```

**Load-bearing:** the guard must be the `if (c == 0)` early form written as a
separate statement *after* the `+0x7E` store, which produces retail's
`bnez` with the first `sh` in the delay slot; and the second walk must use a
**fresh pointer local `q`**, since re-reading `*(unsigned char **)a0` in one
expression makes cc1 CSE the two cursor loads.

```
tools/analysis/era_link_match.sh src/func_8008FC28.c 0x8008FC28 0x50 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008FC28.c 0x8008FC28 0x50 -O2 -G0
```

Object: `C .text 80 vs ROM 80`, no pad. Link-level `word mismatches=0` →
`LINK_EXACT`.
