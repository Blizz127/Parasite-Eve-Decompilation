# func_800501C8

- **VRAM**: 0x800501C8
- **File offset**: 0x409C8 (size 0x3C)
- **Profile**: -O2 -G8
- **Unit**: see configs/USA/disc1.yaml carve
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `tools/analysis/auto_leaf.py func_800501C8` swept all era/modern profiles (no zero-edit hit).
2. m2c draft (`tools/analysis/m2c_leaf.py --print func_800501C8`) used as the starting shape; the
   `saved_reg_gp` pseudo-struct was rewritten to the real gp-relative globals listed below.
3. `tools/analysis/try_leaf.py src/func_800501C8.c 0x409C8 0x3C --flags "-O2 -G8"` -> `WORDS MATCH`.
4. YAML carve + profile assignment, then authoritative fresh build.

## Notes / divergences

gp+0x184 = D_8009CEF4 (node latch). func_800638D8(node, func_8005100C), func_8005EB58(1), func_8005EB64(0x68). The pure wrapper spec in pc_port collapses this; retail holds the explicit latches and calls. -G8 required for the gp store.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (822 registered C leaves)`
- `VERIFY_US=PASS`
