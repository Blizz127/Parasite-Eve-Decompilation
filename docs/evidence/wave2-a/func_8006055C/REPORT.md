# func_8006055C

- **VRAM**: 0x8006055C
- **File offset**: 0x50D5C (size 0x34)
- **Profile**: -O2 -G8
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_8006055C` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_8006055C`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_8006055C.c 0x50D5C 0x34 --flags "-O2 -G8"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

gp+0x3B4 = D_8009D124, gp+0x3B8 = D_8009D128. func_800602D0(value, 5) then the inlined func_8005E8A4(5, 0) pair-bump. Divergence: a literal `D_8009D128 += 0` is folded away by cc1, but retail keeps the load/store. The local `int *p = &D_8009D128; ... *p = D_8009D128;` pointer idiom (same lever as src/func_80067B40.c) reproduces the redundant self-store exactly.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
