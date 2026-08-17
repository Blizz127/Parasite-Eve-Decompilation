# PE-BTL6 — func_8003B97C lighting at 0x8003BA24

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and PE.IMG `[428,434)`
SHA-256 `56b2db6d…7674e0`. No matching `src/` C. No host float.
No NCLIP. `andi 0xFC` was not fired.

## COP2 (psx-spx)

| Word | Role |
|---|---|
| `0x049E012` | MVMVA sf=1 mx=RT v=IR cv=None lm=0 — three columns |
| `0x118043F` | NCCT sf=1 lm=1 — `LLM*V`, `BK<<12+LCM*IR`, `RGB*IR<<4>>12` |

There is no opcode-6 NCLIP in `0x8003B97C..0x8003BCE0`.

Scratchpad `0x1F800004` is not `PE_RAM`; the RTIR 3×3 is kept in a
local MATRIX-shaped buffer then loaded as LLM. `RBK/GBK/BBK` =
`dest+0x88<<4`. RGBC comes from `D_8009CDA0` (`gp+0x30`). LCM is
the leftover 6698C `ctc2` of `BEA40+0x20` (live `D_800BD025=0` and
`dest+0x88=0` → LCM zero).

## Live 2-bone object

`obj+2=2`. Rec0 byte4=0 skips lighting. Rec1 byte4=1, `lhu0=1`,
`lhu2=38`. dest+0xC `+3/+7/+B` flags are 0, so the second NCCT
keeps `D_8009CDA0`. With live LCM/BK zero, FIFO words are 0.

## Verify

```text
python3 pc_port/tools/pe_btl6_3b97c_lighting_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```

STOP: `0x8006CC2C` — `6C5BC` `andi 0xFC` after jal 3D834.
3BCE0 live counts are already the full leaf. Do not complete `0x55`.
