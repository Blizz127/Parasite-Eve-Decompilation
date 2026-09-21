# func_80090178 — 0x24 bytes, LINK_EXACT

Retail VRAM `0x80090178` (file offset `0x80978`), in `asm/disc1/806FC.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Twin of `func_80090054` with a different cleared bit and a different zeroed
halfword: clear bit 2 of `+0x38`, OR `0x3` into `+0xF4`, zero `+0xEC`.

```c
void func_80090178(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) &= ~4;
    *(unsigned int *)(a0 + 0xF4) |= 3;
    *(unsigned short *)(a0 + 0xEC) = 0;
}
```

```
tools/analysis/era_link_match.sh src/func_80090178.c 0x80090178 0x24 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80090178.c 0x80090178 0x24 -O2 -G0
```

Object: gas pad only. Link-level `word mismatches=0` → `LINK_EXACT`.
