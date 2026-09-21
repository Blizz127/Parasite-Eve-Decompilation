# func_800500A8

- **VRAM**: 0x800500A8
- **File offset**: 0x408A8 (size 0x64)
- **Profile**: -O2 -G8
- **Unit**: 408A8
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `auto_leaf.py` swept all 12 era profiles (no zero-edit hit): the m2c draft used the
   `saved_reg_gp` pseudo-struct.
2. Hand-normalised to the real gp-relative global; `try_leaf.py src/func_800500A8.c 0x408A8 0x64 --flags "-O2 -G8"` -> `WORDS MATCH`.
3. YAML carve (the whole former 0x408A8 asm span is now three C leaves) + profile assignment.

## Notes / divergences

gp base 0x8009CD70; gp+0x1E4 = D_8009CF54. `func_80062A34(2, 0x17)` result is
null-checked; non-null reads `+0x48` and adds 0x73, else 0x75; result feeds
`func_8005DC4C` and is stored to the gp word. Then `func_800638D8(node, func_80050F10)`.
The `int text` local is the single reused `$a0` value. Label name `L800500D8`.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (825 registered C leaves)`
- `VERIFY_US=PASS`
