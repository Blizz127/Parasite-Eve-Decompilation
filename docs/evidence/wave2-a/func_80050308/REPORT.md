# func_80050308

- **VRAM**: 0x80050308
- **File offset**: 0x40B08 (size 0x34)
- **Profile**: -O2 -G8
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_80050308` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_80050308`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_80050308.c 0x40B08 0x34 --flags "-O2 -G8"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

gp+0x1A8 = D_8009CF18. func_8005EB64(index + (D_8009CF18 ? 0x7C : 0x7F)); inverted-branch ternary with a jump, exactly as the port spec. -G8 for the gp load.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
