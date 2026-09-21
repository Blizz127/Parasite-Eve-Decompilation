# func_800524D0 — guarded bit-9 extractor through D_8009D254

VRAM `0x800524D0`, size `0x44` (17 words), file offset `0x42CD0`.
Carved from the tail of the former `0x42664` asm span (prefix `0x66C`),
closing exactly at the next span `0x42D14` (`func_80052514`).

Retail: `p = D_8009D254; if (!p) return 0; p = *p; if (!p) return 0;
return (p->f+0x4C >> 9) & 1;`

## Profile

Default `era_o2_g0` (`-O2 -G0`).

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_800524D0 0x800524D0 0x44 -O2 -G0
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern unsigned char *D_8009D254;
int func_800524D0(void) {
    unsigned char *p = D_8009D254;
    if (p == 0) return 0;
    p = *(unsigned char **)p;
    if (p == 0) return 0;
    return (*(unsigned int *)(p + 0x4C) >> 9) & 1;
}
```

Writing the two null guards as early `return 0;` (rather than nesting)
reproduces retail's shared `.L80052508` tail and the `j`/`andi` delay slot.
