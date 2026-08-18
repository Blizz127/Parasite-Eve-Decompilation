# PE-BTL49 — type-2 fork 0x4B / 13C34 and 0x54 / 143B0

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80013C34` — 148 words `0x80013C34..0x80013E84`,
SHA-256 `0c508334…5de2`. `D_800910A0[0x4B]`.
`jal 79FB4`. v0=1.

Live fork `0x6B0`: `(0,0,0x400)` faces D254.
`desired = (0x1400 - ratan2(dZ,dX)) & 0xFFF`.
Steps `D2F0+0x3A` by at most `0x400`. Unfinished
turns rewind `CE00` by `0x14` and set delay 1.

`func_800143B0` — 83 words `0x800143B0..0x800144FC`,
SHA-256 `53cb1131…a629`. `D_800910A0[0x54]`.
Live fork `0x6FE`: `(0,0,local[0x11])` stores
`abs(dX)+abs(dY)+abs(dZ)` vs D254. Miss = -1.

Do not invent scratch / hit / pad.

## Verify

```text
python3 pc_port/tools/pe_btl49_13c34_143b0_oracle.py
PE_TEST_FILTER=BTL49 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
