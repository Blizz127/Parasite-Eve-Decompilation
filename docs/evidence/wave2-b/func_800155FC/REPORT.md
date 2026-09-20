# func_800155FC — wave2-b

- Leaf: `func_800155FC` (VRAM 0x800155FC)
- File: 0x5DFC, size 0x4C (19 words), unit 38B4
- Source: `src/func_800155FC.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: `[0x5630, asm]`, `[0x5DFC, c, func_800155FC]`, `[0x5E48, asm]`

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 815 → 817
with the sibling leaf landed in the same batch. `scripts/verify_us.sh`
= PASS at 817.

## Divergence / lever

Zero divergences. Two symbol-declaration choices carry the match:

1. `0x90($gp)` is `D_8009CDF8 + 8` and `0x590($gp)` is the pointer global
   `D_8009D300`. Both are declared as scalars so the accesses stay
   gp-relative under `-G8`; `char *D_8009D300` makes the load-then-store
   through the pointer match `lw $a0,0x590($gp); sw $v1,0x10($a0)`.
2. `D_800BCF88` (0x800BCF88) is outside the $gp window and must stay
   absolute: `extern int D_800BCF88[];` (incomplete array) emits no
   `.extern D_800BCF88, N`, so maspsx leaves the `lui`/`lw` pair absolute.
