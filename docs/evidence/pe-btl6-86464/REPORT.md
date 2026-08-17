# PE-BTL6 — func_80086464 + func_80086C1C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. Stream-complete / 6914C / 6E6D4
success are not invented.

## 86464

`0x80086464..0x80086498` exclusive, 13 words, SHA-256
`aa81a73b…ca782`. `D_800BCD80=0x10`, `D_800BCD84=a0`,
`jal 8CBA8`. Live 6D60C F2=0x30 passes `lw overlay+0x124`.

## 86C1C

`0x80086C1C..0x80086C5C` exclusive, 16 words, SHA-256
`e1bd632d…d13d`. `D_800BCD80=0xC0`, `andi a1,0x7F`,
`D_800BCD84=a1`, `D_800BCD90=a0`, `jal 8CBA8`. Live
`86C1C(0, 0x7F)`. Cmd `0xC0` is the 8CBA8 default
ring-enqueue (already ported).

## 8CBA8 cmd 0x10

`beq cmd,0x10` → `0x8008CC68`: `a0=*D_800BCD84`,
`jal 85084`. `85084` is 5 words
`0x80085084..0x80085098`, SHA-256 `cec5712d…6cbfa`:
`return *buffer + 0xB0BEB4BF` (AKAO magic 0). `v0!=0`
→ `s1=-1`, epilogue `D_8009D268=0`, return -1.
`v0==0` continues to `8CB54`/`8CB08` ring fill — not
entered, not invented.

## 6D60C 0x30

`0x8006D898`: jal 86464, jal 86C1C(0,0x7F), sb F2=0,
return 0. 144FC 0x38 still returns 0. No `0x39` /
`6914C`.

## Next

`0x80042F20` — 144FC 0x38 after 6D60C returns 0.
Do not stub `6914C`, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_86464_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
