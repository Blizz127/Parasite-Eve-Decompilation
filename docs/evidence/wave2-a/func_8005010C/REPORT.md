# func_8005010C

- **VRAM**: 0x8005010C
- **File offset**: 0x4090C (size 0x6C)
- **Profile**: -O2 -G8
- **Unit**: 408A8
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `auto_leaf.py`: no zero-edit hit (`saved_reg_gp` draft, `-G0` addressing).
2. Hand-normalised; `try_leaf.py src/func_8005010C.c 0x4090C 0x6C --flags "-O2 -G8"` -> `WORDS MATCH`.
3. YAML carve + `era_o2_g8` assignment.

## Notes / divergences

gp+0x184 = D_8009CEF4. Latches `node`, registers `func_80050F64`, calls
`func_80062CC4()`, then the `func_8005EB58(1)` / `func_8005EB64(0x68)` /
`func_8005E8A4(0, 0x10)` pulse repeated three times. Here `func_8005E8A4` is a
real `jal` (not inlined), unlike the 50074 wrappers.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (825 registered C leaves)`
- `VERIFY_US=PASS`
