# PE-BTL76 — 67A78 / 67294 and 68CE0 flag outs

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`67A78` — 50 words `0x80067A78..0x80067B40`,
SHA-256 `cc38093d…2759`. `68CE0` @ `68D00`.
Writes `container+0x38 = (+0x2C) - (BCF8C-160)` and
`+0x3A = (+0x2E) - (BCF8E-112)`. Then walks
`B1624+0x14` stride 56. `jal 67294` when
`rec[0]&2` and `+0x24==BCFFD`.

Live m0005i chunk2+`0x25CB0`: count 10, every
record has flags `0x02`. Rec0 `+0x24=0`. First
`68CE0` with `BCFFD=0` therefore jals `67294`.

`67294` — 249 words `0x80067294..0x80067678`,
SHA-256 `bb61228a…bdc6`. Zero jal. Writes
`+0x18/+0x1A`. OT splice uses `+0x30/+0x34`.
Those lists are published by
`3F074→68B94→677FC→66F60` (`66F60` 154w, sole
jal from `67940`). This 3F3C4 cut does not jal
`3F074`/`68B94`. File image `+0x30/+0x34` are
zero. Unpublished `+0x30==0` skips the OT walk
(host).

`67B74` early-out: `BCF88&0x400==0`.
`67D18` early-out: `BCF88&0x1000==0`.
Live both clear. Bodies not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl76_67a78_oracle.py
PE_TEST_FILTER=BTL76 ./pc_port/build/pe-native-tests
```
