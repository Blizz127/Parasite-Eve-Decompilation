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
| 0x2B | `0x8006D1DC` | `jal 6CDA4(3, lhu+4, lhu+6, +0x194, 0x21, 0)`; v0==1 → return 1 |
| other <44 | `0x8006D22C` | v0=0 |

0x2A: `s2 = base+lw(base+4)`; count = `lw(s2+0x24)>>16`;
records at `base+(word24&0x3FFFFF)`, 8 bytes. Count
`<<16` overlaps the 22-bit offset (count=1 → `+0x10000`). Skip unless
byte+3 bit `0x10` and `lhu+4>=2`. Empty/`D_800B0E64==0`
→ sb F3=0 return 0. 0x28 re-dispatches 0x2A on the same
tick. 0x2B a0=3 runs the table fill then `87414`
(`D_8009D270=2`, return 0) and sb F0=7.

## Next

`0x8006D79C` — 6D60C after 6D078 returns 0. Live 0x2A
needs the `D_800B0E64` archive. Do not stub `6914C`,
jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6d078_oracle.py
python3 pc_port/tools/pe_btl6_6d60c_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
