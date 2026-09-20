# func_8004F798

- **VRAM**: 0x8004F798
- **File offset**: 0x3FF98 (size 0x40)
- **Profile**: -O2 -G8
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_8004F798` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_8004F798`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_8004F798.c 0x3FF98 0x40 --flags "-O2 -G8"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

gp base 0x8009CD70; gp+0x1E8 = D_8009CF58. Nested calls: lbu 4(D_8009CF58) minus 1 feeds func_8005DCEC, result feeds func_8005F27C. -G0 leaves the global absolute (lui/lw) instead of the retail single gp-relative lw, so the leaf is registered era_o2_g8.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
