# PE-BTL6 — func_8006CDA4 +0xF0 state machine

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 6CDA4 / 6E6D4 / 6914C are not stubbed
to succeed.

## Window

`0x8006CDA4..0x8006D078` exclusive, 181 words, SHA-256
`c94c08adeed015909c745201d6c1f142f9966204b52470fc0fc0778fce3c8f79`.
`$s4=a0`, `$s2=D_800B0CD8`, `sll a1,1`, table `D_8009317C`.
Dispatch `lbu +0xF0; sltiu 11`, JT `0x80011428`. Zero stores of
overlay `+0xE`. Boot `6A674` zeros `+0xF0`.

## +0xF0 cases

| F0 | VA | Effect |
|---|---|---|
| 0 | `0x8006CE3C` | table → gp+0x400/404/408; a0==0 jal 87198; a0==3 jal 87414; a0==1 a2 stays -1, sb 7, return 1 |
| 7 | `0x8006CEC4` | `jal 6E6D4` CD issue |
| 8 | `0x8006CF28` | `jal 6E7E8` poll |
| 9 | `0x8006CF54` | stream decode by a0 |
| 0xA | `0x8006CFE8` | `jal 870E0` |
| 1-6 | `0x8006D03C` | unused → epilogue |

Live 6D078 0x28 is `6CDA4(1,1,0,+0x194,0x21,0)`. Table word
`0x8B0`, a1=1 halves `0x16`/`0x25` → gp+0x404/408 = `0x0F`,
gp+0x400 = `D_800B0DD8+0x8C6`. Then sb 7, return 1. State 7
is not entered on that tick.

Named cut implements that a0=1 state 0 and parks at F0=7.
It does not invent 6E6D4 success.

## Next

`0x8006E6D4` — 6CDA4 state 7 jal (already translated as the
host CD issue). Do not stub a complete read, jump to mode 7,
or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6cda4_oracle.py
python3 pc_port/tools/pe_btl6_6d078_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
