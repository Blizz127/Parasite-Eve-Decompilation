# PE-BTL6 — func_8003A088 GTE walk at 0x8003A3B4

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and PE.IMG `[428,434)`
SHA-256 `56b2db6d…7674e0`. No matching `src/` C. No host float.
`andi 0xFC` was not fired.

## COP2 (psx-spx bitfields)

| Word | sf | mx | v | cv | lm | Role |
|---|---|---|---|---|---|---|
| `0x049E012` | 1 | RT | IR | None | 0 | column RTIR → dest+0x84 rows |
| `0x0480012` | 1 | RT | V0 | TR | 0 | MAC → dest+0x84+20; optional dest+0x80 |

```text
MAC = (Tx<<12 + RT*V) >> 12
IR  = sat16(MAC)          # lm=0: -0x8000..0x7FFF
```

`PE_GTE_MVMVA` in `pc_port/platform/pe_gte.c`. mx≠0 / v∉{0,3} / cv∉{0,3}
are refused (not invented).

## Live 2-bone fixture

CE2=11 + CE4=1 object SHA-256 `fcf33e91…fba4`: `obj+2=2`,
`obj+0x18=2`, parents `[0,1]`. Rec0 byte4=0 (no dest+0x18).
Rec1 byte4=1, slot1 `(VX,VY,VZ)=(0,20,-7)`, `+0xE=1`.
Identity RT → dest+0x80 stores `0, 20, -7`.

Parent ±1/±2 scratchpad arms are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl6_3a088_walk_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```

STOP: 3B97C lighting is in `pe-btl6-3b97c-lighting`. Do not `andi 0xFC`.
