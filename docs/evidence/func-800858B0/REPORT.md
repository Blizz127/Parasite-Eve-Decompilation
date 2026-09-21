# func_800858B0 — bounded halfword getter through D_8009B7D0

VRAM `0x800858B0`, size `0x38` (14 words), file offset `0x760B0`.
Carved from the head of the former `0x75F44` asm span (prefix `0x16C`),
closing exactly at the next span `0x760E8` (`func_800858E8`).

Retail: `u = a0 & 0xFFFF; if (u >= 3) return 0;
return *(unsigned short *)(D_8009B7D0 + u*16);` — the 16-byte-stride
twin of `func_8008594C`.

## Profile

Default `era_o2_g0` (`-O2 -G0`).

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_800858B0 0x800858B0 0x38 -O2 -G0
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern unsigned char *D_8009B7D0;
int func_800858B0(int a0) {
    register int u asm("$3");
    register unsigned char *base asm("$2");
    u = a0 & 0xFFFF;
    if (u >= 3) return 0;
    base = D_8009B7D0;
    return *(unsigned short *)(base + u * 16);
}
```

## Durable lever

Retail's `$v1`/`$v0` allocation here is the *inverse* of cc1's natural
choice: retail keeps the masked index in `$v1` and the loaded table base
in `$v0`, whereas cc1 wants the index in `$a0`/`$v0`. Pinning `u` to `$3`
and `base` to `$2` is load-bearing (without the pins the leaf mismatches
7 words; the pins give 0). The same shape as `func_8008594C`, but the
getter needs the pins while the setter does not.
