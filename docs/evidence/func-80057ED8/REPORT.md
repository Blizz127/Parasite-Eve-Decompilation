# func_80057ED8 — signed-index range-clamped halfword getter

VRAM `0x80057ED8`, size `0x3C` (16 words), file offset `0x486D8`.
Formerly an empty `0x486D8` asm span. The C span closes exactly at the
next span `0x48714`.

Retail: `if (a0 < 0) return 0; if (a0 >= D_8009D078) return 0;
return D_800A1FD4[a0];` — the table holds `short`, indexed by the signed
`a0` (`slt`), with the `0x308($gp)` limit.

## Profile

`era_o2_g8_symbol_at_temp` — era `-O2 -G8` with `MASPSX_SYMBOL_AT_TEMP=1`
(maspsx patch 5). Both knobs are load-bearing: without patch 5 the indexed
symbolic load `lh $v0,%lo(D_800A1FD4)($at)` becomes `lh $v0,0($at)`
(9 mismatches), and `-G0` costs 14.

## Commands

```
env -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  bash -c 'export MASPSX_SYMBOL_AT_TEMP=1; \
    tools/analysis/check_leaf.sh func_80057ED8 0x80057ED8 0x3c -O2 -G8'
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern int D_8009D078;
extern short D_800A1FD4[];
int func_80057ED8(int a0) {
    if (a0 < 0) return 0;
    if (a0 >= D_8009D078) return 0;
    return D_800A1FD4[a0];
}
```
