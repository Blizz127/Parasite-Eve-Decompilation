# func_800900E4 — 0x24 bytes, LINK_EXACT

Retail VRAM `0x800900E4` (file offset `0x808E4`), in `asm/disc1/806FC.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Cursor reader, byte `<< 7` into the halfword at `+0xB4`:

```c
void func_800900E4(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    *(unsigned short *)(a0 + 0xB4) = (unsigned short)(*p << 7);
}
```

```
tools/analysis/era_link_match.sh src/func_800900E4.c 0x800900E4 0x24 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_800900E4.c 0x800900E4 0x24 -O2 -G0
```

Object: gas pad only. Link-level `word mismatches=0` → `LINK_EXACT`.
