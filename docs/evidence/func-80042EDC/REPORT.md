# func_80042EDC — D_8009CEE0 group seed + D_800BD024 clamp

VRAM `0x80042EDC`, size `0x44` (17 words), file offset `0x336DC`.
Formerly an empty `0x336DC` asm span (the C starts exactly there). The C
span now closes exactly at the next span `0x33720` (`func_80042F20`).

Retail: `D_8009CED8 = 1; D_8009CEE4 = 1; D_8009CEE8 = 0; v = D_800BD024;
D_8009CEDC = v;` then clamp: `v < 0` => 1, `v >= 0x21` => 0x20.

## Profile

`era_o2_g8_force_d800bd024_absolute` — era `-O2 -G8` with
`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800BD024`. The int destinations
(`D_8009CE*`) are `$gp`-relative (`0x16C`/`0x174`/`0x178`/`0x168($gp)`),
but the separate `.data` byte `D_800BD024` is outside ±32K of `$gp`, so
`-G8` without the force knob fails to link
(`relocation truncated to fit: R_MIPS_GPREL16`).

## Commands

```
env -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  bash -c 'export MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800BD024; \
    tools/analysis/check_leaf.sh func_80042EDC 0x80042EDC 0x44 -O2 -G8'
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern int D_8009CED8, D_8009CEE4, D_8009CEE8, D_8009CEDC;
extern unsigned char D_800BD024;
void func_80042EDC(void) {
    int v;
    D_8009CED8 = 1;
    D_8009CEE4 = 1;
    D_8009CEE8 = 0;
    v = D_800BD024;
    D_8009CEDC = v;
    if (v < 0) {
        D_8009CEDC = 1;
    } else if (v >= 0x21) {
        D_8009CEDC = 0x20;
    }
}
```

## Durable lever

A gp-relative leaf whose *word* destinations are in small-data but whose
*byte* source is out of range needs `MASPSX_FORCE_ABSOLUTE_SYMBOLS=<byte>`
alongside `-G8`; the alternative `-G0` loses every word store's
gp-relative encoding. `-O1 -G8` degrades badly (17 mismatches).
