# PE-BTL6 — func_80087090 + 6CDA4 state-9 a0=1

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 851A8 / 870E0 / 6914C are not stubbed
to succeed.

## Window

`0x80087090..0x800870E0` exclusive, 20 words, SHA-256
`3cb0ae9829fa97505104d41f8517ef9a96a8339bc7ee76c7ea5d63901b522e3f`.
Already translated (B45): retry `func_800851A8(buffer, count)`
while v0==1. Live 6CDA4 a0=1 is `87090(dest, 0)` at
`0x8006CF98`. Dest is `$s5` = lw overlay+0x194 (the 6E6D4
AKAO buffer).

## State 9

| 87090 | Effect |
|---|---|
| -1 | sb F0=0, return 1 |
| != -1 | sb F0=0xA, return 1 (state 0xA not entered) |

Empty dest fails the 851A8 magic check and stores
`D_8009D24C = -1`. 851A8 success is not invented.

## AKAO consumption

PE.IMG LBA 1013 + `0x8C6`, 0x0F sectors, SHA-256
`cf9670eb…c28b`. Word0 `AKAO` (`0x4F414B41`) +
`0xB0BEB4BF` == 0, so this buffer is the 851A8 upload
format. Params at +0x10: SPU `0x48000`, size `0x6CD0`,
off `0x80`, end `0x90`. Copy is
`(0x90-0x80)<<4` = 256 words. Not issued from the named
cut without a planted dest.

## Next

`0x800870E0` — 6CDA4 state 0xA, 4 words,
`return D_8009D24C`. Do not stub DMA-complete, jump to
mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_87090_oracle.py
python3 pc_port/tools/pe_btl6_6e7e8_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
