# func_800451D0 — wave2-b

- Leaf: `func_800451D0` (VRAM 0x800451D0)
- File: 0x359D0, size 0xF0 (60 words), unit 35698
- Source: `src/func_800451D0.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: `[0x35910, c, func_80045110]`, `[0x359D0, c, func_800451D0]`,
  `[0x35AC0, asm]`

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 810 → 815
with the sibling leaves landed in the same wave. `scripts/verify_us.sh`
= PASS at 815.

## Divergences / levers

1. **Xor swap phrasing.** The block at 0x7C–0xB8 swaps
   `(0x218,0x220)$gp` and `(0x21C,0x224)$gp`. It must be written as the
   two literal sequences `a ^= b; b ^= a; a ^= b;`. The four-temporary
   expansion m2c prints (`t0 = a^b; t1 = b^t0; ...`) lets cc1 interleave
   the two chains and shifts three words (`0x90,0x94,0x9C`).
2. **gp slots.** The four handles live in `D_8009CE54 + 0x134/+0x138`
   (`0x218/0x21C($gp)`) and in the scalar data symbols
   `D_8009CF90` (`0x220($gp)`) / `D_8009CF94` (`0x224($gp)`). All three
   containing symbols are declared as scalars for gp-relative loads;
   `0x23C($gp)` is `D_8009CFA0 + 0x0C`.
3. Callback install uses the project idiom
   `*(void **)(p + 0x2C) = func_800452C0;`.

## Sibling left parked

`func_8004542C` (0x35C2C, same run) is parked; see
`docs/ai_context/parked_blockers.json`.
