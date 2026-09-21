# func_800528C4 — 60x scaled store into D_800A76A4[a0*3]

VRAM `0x800528C4`, size `0x2C` (12 words), file offset `0x430C4`.
Carved from the tail of the former `0x43094` asm span (prefix `0x30`),
closing exactly at the next span `0x433CC`:

```
- [0x430C4, c, func_800528C4]
- [0x430F0, asm]
```

Retail: `D_800A76A4[a0 * 3] = a1 * 60;` where the table stride is 12 bytes
(`sll`/`addu`/`sll` on `a0`, and `a1*60` via `sll 4`, `subu`, `sll 2`).

## Profile

`era_o2_g0_symbol_at_temp` — era `-O2 -G0` with `MASPSX_SYMBOL_AT_TEMP=1`
(maspsx patch 5). Retail's indexed symbolic store keeps the `%lo`
displacement: `lui $at,%hi(D_800A76A4)` / `addu $at,$at,$v1` /
`sw $v0,%lo(D_800A76A4)($at)`. Plain `-O2 -G0` emits `sw $v0,0($at)` and
mismatches 4 words (2 shifted).

## Commands

```
env -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  bash -c 'export MASPSX_SYMBOL_AT_TEMP=1; \
    tools/analysis/check_leaf.sh func_800528C4 0x800528C4 0x2c -O2 -G0'
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern int D_800A76A4[];
void func_800528C4(int a0, int a1) {
    D_800A76A4[a0 * 3] = a1 * 60;
}
```
