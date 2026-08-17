# PE-BTL45 — type-2 0xB7 / 18A48 and 0x70 / 1897C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018A48` — 21 words `0x80018A48..0x80018A9C`,
SHA-256 `9218163e…d862`. `D_800910A0[0xB7]`.
`jal 2FAA4(D2F0, lbu×4, lhu)`; v0=1.
Live `(3,0,6,7,cond[1])`.

`func_8001897C` — 51 words `0x8001897C..0x80018A48`,
SHA-256 `64cd42b0…0ff3`. `D_800910A0[0x70]`.
`jal 2FA10(D2F0, lbu×3, lbu, lhu, lb×4, lbu×2)`;
v0=1. Live `(0,0,8,9,cond[1],5,-1,-1,-1,3,15)`.

`2FAA4` / `2FA10` are already ported (PE-CH1).
Next unported on this arm is `0x59` (18004 →
2FE78 / 3010C, not yet ported).

## Verify

```text
python3 pc_port/tools/pe_btl45_18a48_1897c_oracle.py
PE_TEST_FILTER=BTL45 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
