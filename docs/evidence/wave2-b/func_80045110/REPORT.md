# func_80045110 — wave2-b

- Leaf: `func_80045110` (VRAM 0x80045110)
- File: 0x35910, size 0xC0 (48 words), unit 35698
- Source: `src/func_80045110.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: `[0x3578C, asm]`, `[0x35910, c, func_80045110]`,
  `[0x359D0, c, func_800451D0]` (adjacent, no gap)

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 810 → 815
with the sibling leaves landed in the same wave. `scripts/verify_us.sh`
= PASS at 815.

## Divergence / lever

Zero divergences after the m2c draft once the three gp slots are declared
as scalars inside the containing data symbol `D_8009CE54`:
`0x1A0($gp)` = +0xBC, `0x194($gp)` = +0xB0, `0x19C($gp)` = +0xB8.
`extern int D_8009CE54;` emits `.extern D_8009CE54, 4`, keeping the loads
gp-relative under `-G8`; the array form would go absolute. The
`arg0->unk4->unk4` forwarding is spelled
`*(int *)(*(int *)(arg0 + 4) + 4)`.
