# func_8008594C — bounded clear of D_8009B7D0[a0 & 0xFFFF]

VRAM `0x8008594C`, size `0x34` (13 words), file offset `0x7614C`.
Formerly the `0x7614C` asm span (whose `0x34` head was this function);
closes exactly at the next span `0x76180`.

Retail: `u = a0 & 0xFFFF; if (u >= 3) return 0;
*(unsigned short *)(D_8009B7D0 + u*16) = 0; return 1;`

## Profile

Default `era_o2_g0` (`-O2 -G0`).

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_8008594C 0x8008594C 0x34 -O2 -G0
```

Result: `LINK_EXACT`; deep span-size exact; sequence diff ratio=1.000.

## Source

```c
extern unsigned char *D_8009B7D0;
int func_8008594C(int a0) {
    int u = a0 & 0xFFFF;
    if (u >= 3) return 0;
    *(unsigned short *)((unsigned char *)D_8009B7D0 + u * 16) = 0;
    return 1;
}
```

## Durable lever

The mask must live in a **separate `int u`** local. Testing `a0` directly
after a `unsigned short a0` parameter, or folding the mask into the
condition, produces a *signed* `slti` (1 mismatch) or extra words; keeping
`u` as `int` and comparing `u >= 3` gives retail's unsigned `slti` on the
masked value with `$v1`.
