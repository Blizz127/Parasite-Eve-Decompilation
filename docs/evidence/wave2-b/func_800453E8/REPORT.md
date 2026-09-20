# func_800453E8 — wave2-b

- Leaf: `func_800453E8` (VRAM 0x800453E8)
- File: 0x35BE8, size 0x44 (17 words), unit 35698
- Source: `src/func_800453E8.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: carved the `[0x35698, asm]` run into `[0x35698, asm]`,
  `[0x35BE8, c, func_800453E8]`, `[0x35C2C, asm]`

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 810 → 815
with the sibling leaves landed in the same wave. `scripts/verify_us.sh`
= PASS at 815.

## Divergence / lever

The only non-zero-edit issue is the gp-relative load. Retail emits
`lw $a0, 0x23C($gp)` (D_8009CFA0 + 0x0C). Declaring the containing data
object as a scalar — `extern int D_8009CFA0;` plus
`*(int *)((char *)&D_8009CFA0 + 12)` — makes the emitted `.extern
D_8009CFA0, 4` small-data, so maspsx keeps the load gp-relative. Declaring
it `extern int D_8009CFA0[]` (incomplete) or a sized array makes cc1 emit
`lui`/`lw %lo` absolute and adds two words. `-G0` cannot produce the gp
access at all.
