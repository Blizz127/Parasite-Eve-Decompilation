# func_8008FFC0 — 0x24 bytes, LINK_EXACT

Retail VRAM `0x8008FFC0` (file offset `0x807C0`), in `asm/disc1/806FC.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Serial-cursor reader on the stream object at `a0`: `+0x00` is the cursor, the
fetched byte is pre-shifted `<<8` into the halfword at `+0xA6`.

```c
void func_8008FFC0(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    *(unsigned short *)(a0 + 0xA6) = (unsigned short)(*p << 8);
}
```

The two `(unsigned char **)` casts (rather than a `struct`) are what the
frontend needs to emit retail's `lw + addiu + sw` then `lbu + sll + sh`; a
struct field access reorders them.

```
tools/analysis/era_link_match.sh src/func_8008FFC0.c 0x8008FFC0 0x24 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008FFC0.c 0x8008FFC0 0x24 -O2 -G0
```

Object: `C .text 48 vs ROM 36` (gas pad). Link-level `word mismatches=0` →
`LINK_EXACT`.
