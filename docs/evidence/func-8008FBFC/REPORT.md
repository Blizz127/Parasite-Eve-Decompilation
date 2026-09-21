# func_8008FBFC — 0x2C bytes, LINK_EXACT

Retail VRAM `0x8008FBFC` (file offset `0x803FC`), in `asm/disc1/803FC.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Signed-byte cursor accumulator into the halfword at `+0xDE` (the sibling of
`func_8008FBD4`, which stores the raw sign-extended byte in the same slot).

```c
void func_8008FBFC(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    {
        unsigned short v = *(unsigned short *)(a0 + 0xDE);
        v += (signed char)c;
        *(unsigned short *)(a0 + 0xDE) = v;
    }
}
```

The intermediate `unsigned char c` is required for retail's `lbu` + `sll 24` /
`sra 24` pair (an `int` folds to `lb`); the named `unsigned short v` keeps the
`lhu` / `addu` / `sh` order.

```
tools/analysis/era_link_match.sh src/func_8008FBFC.c 0x8008FBFC 0x2C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008FBFC.c 0x8008FBFC 0x2C -O2 -G0
```

Object: gas pad only. Link-level `word mismatches=0` → `LINK_EXACT`.
