# func_80046334 — wave2-b

- Leaf: `func_80046334` (VRAM 0x80046334)
- File: 0x36B34, size 0x44 (17 words), unit 35698
- Source: `src/func_80046334.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: `[0x35C2C, asm]`, `[0x36B34, c, func_80046334]`, `[0x36B78, asm]`

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 810 → 815
with the sibling leaves landed in the same wave. `scripts/verify_us.sh`
= PASS at 815.

## Divergence / levers

Two small-data decisions:

1. `0x240($gp)` is `D_8009CFA0 + 0x10`. Declared as a scalar
   (`extern int D_8009CFA0;`) so the flag access stays gp-relative.
2. `D_800B0CE6` is an absolute `lbu`/`lui` (0x800B0CE6 lies outside the
   $gp window). Declared as `extern unsigned char D_800B0CE6[]` — an
   incomplete array emits no `.extern D_800B0CE6, 1`, so maspsx leaves the
   load absolute. The naive `extern unsigned char D_800B0CE6;` produced
   `lbu $v0, 0(gp)`.
