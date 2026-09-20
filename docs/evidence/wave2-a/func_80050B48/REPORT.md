# func_80050B48

- **VRAM**: 0x80050B48
- **File offset**: 0x41348 (size 0x4C)
- **Profile**: -O2 -G0
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_80050B48` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_80050B48`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_80050B48.c 0x41348 0x4C --flags "-O2 -G0"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

Zero-edit auto_leaf hit (build/auto_leaves/func_80050B48.c) with era_o2_g0. No gp access; guard is func_80054288(), body calls func_800556E8/index -> func_8005DC9C -> func_8005F27C with +0xEB.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
