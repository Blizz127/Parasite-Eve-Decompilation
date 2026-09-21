# func_8008FCE4 — 0x2C bytes, LINK_EXACT

Retail VRAM `0x8008FCE4` (file offset `0x804E4`), in `asm/disc1/804E4.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Signed-byte cursor accumulator into `+0xE0` — the exact twin of
`func_8008FBFC`'s `+0xDE`, and the accumulator partner of
`func_8008FCBC`'s raw store.

```c
void func_8008FCE4(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    {
        unsigned short v = *(unsigned short *)(a0 + 0xE0);
        v += (signed char)c;
        *(unsigned short *)(a0 + 0xE0) = v;
    }
}
```

```
tools/analysis/era_link_match.sh src/func_8008FCE4.c 0x8008FCE4 0x2C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008FCE4.c 0x8008FCE4 0x2C -O2 -G0
```

Object: gas pad only. Link-level `word mismatches=0` → `LINK_EXACT`.
