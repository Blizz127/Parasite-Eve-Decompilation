# PE-BTL139 REPORT — func_800293F4 matching C (124 words)

```text
PE-BTL139 MATCHING_C — func_800293F4 era -O2 -G8, 124/124 words
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x800293F4..0x800295E4 exclusive (0x1F0 / 124 words)
file        0x19BF4
flags       gcc-2.7.2-psx -O2 -G8 + maspsx 2.21 --dont-expand-li
            MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2E8
```

The BTL137 21-word named cut is the HP prefix of this full leaf
(`0x800293F4..0x80029448`, reached from `29810` with `a0=0`). The
full function continues through the `a0==1` arm (`2AA98` JT[7]), the
record `+0x4C` flag storm, `D_8009D2E8 &= ~0x10`, `21D4C` / `374E8`,
and the D298/9A/9B/9C zeros.

## Addressing

| Symbol | ROM form | C declaration |
|---|---|---|
| D_8009D278 | `lw/sw ?,0x508($gp)` | `extern unsigned char *` (-G8 sdata) |
| D_8009D1D0 | `sw $0,0x460($gp)` | `extern unsigned int` |
| D_8009D244 | `sb ?,0x4D4($gp)` | `extern unsigned char` |
| D_8009D234 | `lui $at` / `sb %lo` | `extern unsigned char []` |
| D_8009D2E8 | 2-word `lui/lw` + `lui $at` / `sw` | `extern unsigned int` + force-absolute |
| D_8009D298/9A/9B/9C | `lui $at` / `sh/sb/sw $0` | arrays |

D_8009D2E8 is a 4-byte scalar. `-G8` makes cc1 emit `.extern D_8009D2E8, 4`,
and GNU as then turns the RMW into gp-relative `lw/sw`. ROM wants the
non-sdata 2-word expansion. `MASPSX_FORCE_ABSOLUTE_SYMBOLS` strips that
one `.extern` line; every other `-G8` symbol stays gp-relative.

## Compiler constraints

- `register unsigned char *r2 asm("$5")` / `r3 asm("$4")` / `flags asm("$2")`
  reproduce ROM's a1 / a0 / v0 coloring (same class as `func_8006A8D4`).
- `asm volatile("" ::: "memory")` after the join `+0x4C` store keeps it
  before the mask hoist; without it gcc -O2 delays the store 19 words.

## Reloc-equivalent .text

Unlinked object vs ROM: 26 reloc-only diffs (gp offsets, `%hi/%lo`,
`j`/`jal` targets). Zero hard mismatches.

```text
scripts/build_us.sh
# RESULT: EXACT MATCH
# SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
# yaml C entries: 228
# func_800293F4.c.o .text: 0x1F0→0x1F0
```

## Not this cut

`func_800292EC` is the tail of `func_80028E94` (prologue at `0x80028E94`,
epilogue at `0x80029368`), not a standalone leaf.
`func_8001F814` emits a jump table whose `.rodata` lives in the `0x800`
prefix pool at `0x800106E4` — splat placement still open.

## Files

```text
src/func_800293F4.c
configs/USA/disc1.yaml          [0x19BF4, c, func_800293F4]
scripts/build_us.sh             era -O2 -G8 + FORCE_ABSOLUTE D_8009D2E8
docs/evidence/pe-btl139-func-800293F4/REPORT.md
```
