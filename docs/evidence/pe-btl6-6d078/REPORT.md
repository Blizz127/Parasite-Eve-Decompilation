# PE-BTL6 — func_8006D078 +0xF3 state machine

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 6D078 / 6CDA4 / 6D60C / 6914C are not
stubbed to succeed.

## Window

`0x8006D078..0x8006D24C` exclusive, **117 words**, SHA-256
`7cf2bb6168b5ccf24db7639f024d093d1925dc90d63cba77ac861c4ffde5b1dd`.
The earlier 357-word span to `6D60C` included later leaves
`6D24C` and `6D2B8`. `$s0=D_800B0CD8`, prologue also loads
`D_800B0E64`. Dispatch `lbu +0xF3; sltiu 0x2C`, JT `0x80011458`.
Default unused states return 0. Zero stores of overlay `+0xE`.
Sole TEXT jal: 6D60C state 0x2E @ `0x8006D788`.

## +0xF3 cases

| F3 | VA | Effect |
|---|---|---|
| 0 | `0x8006D0E4` | `sw 0` gp+0x5C; `sb 0x28`; re-dispatch |
| 0x28 | `0x8006D0F4` | `jal 6CDA4(1,1,0,+0x194,0x21,0)`; v0==1 → return 1; else +0x10>=2 → sb 0x29 return 1; else sb 0x2A |
| 0x29 | `0x8006D140` | `jal 6CDA4(1,+0x10,0,+0x194,0x21,0)`; v0==1 → return 1; else sb 0x2A |
| 0x2A | `0x8006D178` | walk `D_800B0E64` records via gp+0x5C; sb 0x2B or sb 0 and return 0 |
| 0x2B | `0x8006D1DC` | `jal 6CDA4(3, …)`; v0==1 → return 1 |
| other <44 | `0x8006D22C` | v0=0 |

Named cut runs state 0 then jals 6CDA4 at 0x28. A 6E6D4
-1 with `+0x10<2` sb 0x2A and returns 1. The 0x2A walk is
not entered.

## Next

`0x800870E0` — 6CDA4 state 0xA. See `pe-btl6-87090`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6d078_oracle.py
python3 pc_port/tools/pe_btl6_6d60c_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
