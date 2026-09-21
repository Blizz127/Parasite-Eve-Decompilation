# func_8005022C

- **VRAM**: 0x8005022C
- **File offset**: 0x40A2C (size 0x34)
- **Profile**: -O2 -G8
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_8005022C` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_8005022C`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_8005022C.c 0x40A2C 0x34 --flags "-O2 -G8"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

gp+0x184 = D_8009CEF4, gp+0x1B0 = D_8009CF20, gp+0x1E8 = D_8009CF58. Latches node and the current state pointer, then func_800638D8(node, func_80050AD8). -G8.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
