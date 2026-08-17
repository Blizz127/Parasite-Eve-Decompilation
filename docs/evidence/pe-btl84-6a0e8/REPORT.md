# PE-BTL84 — 6A0E8 D1A0&0x10 early-out

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`6A0E8` — 92 words `0x8006A0E8..0x8006A258`,
SHA-256 `aa84251c…bb86`. Sole TEXT jal
`3F3C4@3F640`.

`B0CD8&0x200` returns. Else `D1A0&0x10==0`
skips PutDrawEnv `75424` and the two
`75358` SetDrawArea jals. Live `3E974`
leaves `D1A0|=0x4000`, so the body is out.

`3F5EC` VSync(2) is live (66C7C skipped:
white RGB plus `CFEE&0x40` is not the
`0x86` state). `6A25C` needs
`D26C&0x0F000006`. Do not invent pad.

## Verify

```text
python3 pc_port/tools/pe_btl84_6a0e8_oracle.py
PE_TEST_FILTER=BTL84 ./pc_port/build/pe-native-tests
```
