# func_80014DA0 — wave2-b

- Leaf: `func_80014DA0` (VRAM 0x80014DA0)
- File: 0x55A0, size 0x90 (36 words), unit 38B4
- Source: `src/func_80014DA0.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: carved the `[0x38B4, asm]` run to `[0x38B4, asm]`,
  `[0x55A0, c, func_80014DA0]`, `[0x5630, asm]`

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 815 → 817
with the sibling leaf landed in the same batch. `scripts/verify_us.sh`
= PASS at 817.

## Divergence / lever

Zero divergences after one ordering fix: the destination pointer must be
assigned before the source pointer (`n = 0; dst = buf; src = arg0;`). With
`src = arg0;` first, cc1 homes the stack buffer in `$v1` and the source in
`$a0` and every load in the vertex-copy loop swaps. The port spec
(`pc_port/game/boot/func_8001735C_port.c` `func_80014DA0`) confirms the
double-dereference on both vertex components.
