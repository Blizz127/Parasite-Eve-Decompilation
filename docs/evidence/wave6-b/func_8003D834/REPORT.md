# func_8003D834

- **VRAM**: 0x8003D834
- **File offset**: 0x2E034 (size 0x118)
- **Build profile**: era_o2_g0 (default `-O2 -G0`)
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
Rect/actor init + teardown wrapper. When arg1 != 0 it stores arg1 at +0xB0,
runs the two `func_8003DFD8` copies around the +0x34 block and calls
`func_80039B74`; unconditionally it runs `func_8003A088`, `func_8003DFD8`,
`func_8003B97C`, toggles `D_8009CDDC` twice through `func_8003BCE0(arg0,1,…)`
(sign-extending the halfword each time) and returns the final toggled value.

## Method
m2c draft + the third `func_8003BCE0` parameter typed `short` so cc1 folds the
two `D_8009CDDC` reads to retail's `lh` (the toggled register value needs the
explicit `sll/sra` sign-extension pair). Matched on the first try_leaf pass.

## Evidence
try_leaf `WORDS MATCH`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (889
registered C leaves)`, `VERIFY_US=PASS`.
