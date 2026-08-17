# PE-BTL6 — 144FC state 0x38 jal 6D60C(1)

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 6D60C is not stubbed to return 0.

## Window

`0x8006D60C..0x8006DC18` exclusive, 387 words, SHA-256
`14e5794d4515e763d20ee26c939500f8764e747e83eddd7ad64b4642d905c3a2`.
`$s2=a0`, `$s1=D_800B0CD8`, dispatch `lbu +0xF2; sltiu 0x41`,
JT `0x80011508`. Default unused states at `0x8006DB28` return 0.
Zero stores of overlay `+0xE`.

144FC `0x38` @ `0x8001459C` is `jal 6D60C(1)`. If v0==1 the opcode
rewinds; else `jal 42F20` and `sb 0x39`. Returning 0 from a stub
would skip to `6914C` and is forbidden.

## Live +0xF2=0 (boot `6A674` zeros it)

State 0 @ `0x8006D658`: `sw 0` gp+0x418, `jal 87024` (stream 0xF1,
already translated), then if a0!=0 and overlay word bit `0x400000`
clear and `D_800B0E08` nonzero, two `jal 6DF50`. Always
`sb 0x2C → +0xF2` and re-dispatch.

State 0x2C @ `0x8006D728`: `lb +0xE0` vs `lb +0xDC`. Boot leaves
`+0xDC=0xFF` and `+0xE0=0x27` (not equal). Unequal path may
`jal 86C5C` if overlay word bit `0x40` is set, then `ori 4` on
overlay word 0 and `sb 0x2E`.

State 0x2E @ `0x8006D788`: `jal 0x8006D078`. If that returns 1,
6D60C returns 1 (0x38 parks). 6D078 is **117 words**
(`0x8006D078..0x8006D24C`). See `pe-btl6-6d078`. Do not stub it.

## Next

`0x80087090` — 6CDA4 state 9. Do not stub stream-open,
6914C, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6d60c_oracle.py
python3 pc_port/tools/pe_btl6_14544_oracle.py
```
