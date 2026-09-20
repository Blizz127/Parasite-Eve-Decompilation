# func_8006E1C0 — packed colour/vertex emit (LANDED)

- **Carve**: VRAM `0x8006E1C0`, file `0x5E9C0`, size `0x110` (68 words). Split
  out of the `[0x5E7A8, asm]` run; next span is the existing
  `[0x5EAD0, c, func_8006E2D0]`.
- **Profile**: default `era_o2_g0`.
- **Method / lever**: the record's packed word at `+0x0C` **overlaps** the byte
  at `+0x0F` (and `+0x04` overlaps `+0x07`), so a struct declaration produces
  the wrong offsets. Addressing the record as a `char *` with explicit offset
  casts (`*(unsigned int *)(a0 + 0x10)`, `*(unsigned char *)(a0 + 0xF)`, …)
  reproduces retail exactly.
- The first emit's 4th component is the conditional
  `v1 = 0x100; if (v0 != 0) v1 = v0 & 0xFF;` (the `addiu v1,zero,0x100` sits in
  the `beqz` delay slot).
- **Evidence**: `try_leaf.py src/func_8006E1C0.c 0x5E9C0 0x110 --flags "-O2 -G0"`
  → `WORDS MATCH`.
- **Authority**: fresh build → `EXACT SHA-1
  452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (888
  registered C leaves)`, `VERIFY_US=PASS`.
