# func_80090054 — 0x24 bytes, LINK_EXACT

Retail VRAM `0x80090054` (file offset `0x80854`), in `asm/disc1/806FC.s`. Era
`-O2 -G0` (default profile), LINK_EXACT.

Flag teardown: clear bit 1 of the `+0x38` word, OR `0x3` into `+0xF4`, zero the
halfword at `+0xEA`.

```c
void func_80090054(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) &= ~2;
    *(unsigned int *)(a0 + 0xF4) |= 3;
    *(unsigned short *)(a0 + 0xEA) = 0;
}
```

**Load-bearing:** the mask is `~2`, not `~3`. Retail clears only bit 1
(`andi` with `0xFFFD`); `&= ~3` gives `0xFFFC` (`addiu $a1,-4`).

```
tools/analysis/era_link_match.sh src/func_80090054.c 0x80090054 0x24 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80090054.c 0x80090054 0x24 -O2 -G0
```

Object: gas pad only. Link-level `word mismatches=0` → `LINK_EXACT`.
