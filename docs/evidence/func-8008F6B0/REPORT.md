# func_8008F6B0 — 0x44 bytes, LINK_EXACT

Retail VRAM `0x8008F6B0` (file offset `0x7FEB0`), in `asm/disc1/7FEB0.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Cursor read that seeds two fields and conditionally raises a flag:

```c
void func_8008F6B0(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned char c;
    unsigned int f;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    f = *(unsigned int *)(a0 + 0x38);
    *(unsigned short *)(a0 + 0x74) = 0;
    *(unsigned short *)(a0 + 0xD8) = (unsigned short)(c << 8);
    if (f & 0x100)
        *(unsigned int *)(a0 + 0xF4) |= 3;
}
```

**Load-bearing:** hoisting the `+0x38` load into `f` and issuing it *before*
the `+0x74` zero store is what puts retail's `lw $v1,0x38` / `sh $zero,0x74`
pair ahead of the `sll`, and the `andi 0x100` before the `beqz` with the
`sll`+`sh` pair in the delay slot.

```
tools/analysis/era_link_match.sh src/func_8008F6B0.c 0x8008F6B0 0x44 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008F6B0.c 0x8008F6B0 0x44 -O2 -G0
```

Object: `C .text 80 vs ROM 68` (gas pad). Link-level `word mismatches=0` →
`LINK_EXACT`.
