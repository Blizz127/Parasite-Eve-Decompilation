# PE-BTL26 — type-3 0x77 region test

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80014DA0` — 36 words `0x80014DA0..0x80014E30`,
SHA-256 `e66738b1…8201`. `D_800910A0[0x77]`. Always v0=1.
Copies four (x,y) words to ROM `sp+16`, then
`1CAB0(*arg8, *arg9, sp+16, 4)` and `*arg10 = v0`.

`func_8001CAB0` — 60 words `0x8001CAB0..0x8001CBA0`,
SHA-256 `e011a7b8…9ebc`. Zero jal. `sra` a0/a1 by 16,
then the 1C614-family edge-crossing toggle against
8-byte vertices (`lh +2` / `lh +6`).

Live type-3 after `0x5E` (D254 `+0x28/+0x30` → local[0]/[2]):

```text
rectangle (0x080A,0x04CD)-(0x0994,0x063D)
point = (local[0]>>16, local[2]>>16)
store local[4]
0x09 subop 0x0B: cond[0] = (local[4] == 1)
0x05 rel 0x7C: if cond[0]==0 goto base+0xF8
```

Type-0 spawn zeros `+0x28/+0x30`, so the first test
misses and skips to `+0xF8` (another `0x5E`/`0x77` pair).
Do not invent a hit. Do not name type 3.

## Verify

```text
python3 pc_port/tools/pe_btl26_14da0_oracle.py
PE_TEST_FILTER=BTL26 ./pc_port/build/pe-native-tests
```
