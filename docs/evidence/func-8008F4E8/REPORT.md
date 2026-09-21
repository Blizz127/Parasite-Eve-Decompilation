# func_8008F4E8 — 0x2C bytes, LINK_EXACT

Retail VRAM `0x8008F4E8` (file offset `0x7FCE8`), in `asm/disc1/7E044.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Cursor reader: advance `*a0`, OR `0x3` into the `+0xF4` flag word, then store
the fetched byte `<<8` at `+0x6C`.

```c
void func_8008F4E8(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned int *)(a0 + 0xF4) |= 3;
    *(unsigned short *)(a0 + 0x6C) = (unsigned short)(c << 8);
}
```

**Load-bearing:** the fetched byte must live in an `unsigned char c` (not
`*p` at the store). Retail keeps `lbu` **after** the `+0xF4` RMW and widens
only at the shift; reading `*p` at the store makes cc1 reschedule the `lbu`
next to the pointer increment and the order changes (10-word residual).

```
tools/analysis/era_link_match.sh src/func_8008F4E8.c 0x8008F4E8 0x2C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008F4E8.c 0x8008F4E8 0x2C -O2 -G0
```

Object: gas pad only. Link-level `word mismatches=0` → `LINK_EXACT`.
