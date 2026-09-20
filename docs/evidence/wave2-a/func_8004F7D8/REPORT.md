# func_8004F7D8

- **VRAM**: 0x8004F7D8
- **File offset**: 0x3FFD8 (size 0x30)
- **Profile**: -O2 -G8
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_8004F7D8` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_8004F7D8`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_8004F7D8.c 0x3FFD8 0x30 --flags "-O2 -G8"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

gp+0x1E8 = D_8009CF58. func_8005E8A4(6,6) then func_80053648(D_8009CF58). Requires -G8 for the gp-relative load.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
