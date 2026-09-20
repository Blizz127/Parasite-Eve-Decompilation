# func_80050178

- **VRAM**: 0x80050178
- **File offset**: 0x40978 (size 0x50)
- **Profile**: -O2 -G8
- **Unit**: 408A8
- **Result**: byte-exact in a fresh `scripts/build_us.sh` + `scripts/verify_us.sh`

## Method

1. `auto_leaf.py`: no zero-edit hit.
2. Hand-normalised; `try_leaf.py src/func_80050178.c 0x40978 0x50 --flags "-O2 -G8"` -> `WORDS MATCH`.
3. YAML carve + `era_o2_g8` assignment.

## Notes / divergences

gp+0x184 = D_8009CEF4. Same shape as func_8005010C minus `func_80062CC4()` and
the final pulse: latch `node`, register `func_80050FB8`, `func_8005EB58(1)`,
`func_8005EB64(0x68)`, `func_8005E8A4(0, 0x10)`, `func_8005EB64(0x68)`.

## Evidence

- `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
- `Matching claim: YES (825 registered C leaves)`
- `VERIFY_US=PASS`
