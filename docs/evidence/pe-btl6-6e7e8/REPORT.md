# PE-BTL6 — func_8006E7E8 + 6CDA4 state-8 poll

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 6E7E8 / 87090 / 6914C are not stubbed
to succeed.

## Window

`0x8006E7E8..0x8006E834` exclusive, 19 words, SHA-256
`968f5fb0f55b63c6b38d3a2cc155e06bd89095b6e621c665590d280501e54c89`.
`jal 811E4(sp+0x10)`; if `st+1 < 2` then
`D_800B0CD8 &= 0xFEFFBFFF`; return st. Host I/O is
synchronous: after a real 6E6D4, `D_8009B6B4` is 0, so the
next poll returns 0. No sleep is added.

## 6CDA4 state 8

| poll | Effect |
|---|---|
| -1 | sb F0=7, return 1 |
| !=0 | F0 stays 8, return 1 |
| 0 | sb F0=9, return 1 (state 9 not entered) |

Live a0=1 state 9 jals `0x80087090` (20 words, jal 851A8
loop). a0==0 path `sll` gp+0x40C by 11, so the 6E6D4 size
is sectors. Host `func_8006E6D4` takes bytes: state 7
converts `chunk<<11` at that boundary (same as 6E6A8).

## PE.IMG payload

PE.IMG LBA 1013 + `0x8C6`, 0x0F Form1 sectors (30720 bytes),
magic `AKAO`, SHA-256
`cf9670eb8c7b0b75223aac794e66d1c007757a40cf801317611a445af4f1c28b`.

## Next

`0x80087090` — 6CDA4 state 9 live a0=1. Do not stub
stream-open, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6e7e8_oracle.py
python3 pc_port/tools/pe_btl6_6cda4_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
