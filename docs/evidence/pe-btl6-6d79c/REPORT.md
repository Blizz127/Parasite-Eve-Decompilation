# PE-BTL6 — 6D60C after 6D078 returns 0 (`0x8006D79C`)

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. 6D60C / 6914C / 86C5C are not stubbed
to succeed.

## D79C

After `jal 6D078` at `0x8006D788`, `beq v0,1` returns 1.
Fall-through at `0x8006D79C` reads overlay word 0:

| bits | Effect |
|---|---|
| `0x4` clear | sb F2=0x3F, re-dispatch |
| `0x4` set, `0x40` clear | sb F2=0x3F, re-dispatch |
| both set | timer `60-(D_8009CDA4-gp+0x41C)` into gp+0x420; maybe `86C5C`; sb 0x3F |

Live 0x2C `ori 4` and leaves bit `0x40` clear, so NYPD takes
the middle row.

## +0xF2 after that

| F2 | VA | Live effect |
|---|---|---|
| 0x3F | `0x8006D818` | bit4 set, bit40 clear → sb 0x2F |
| 0x2F | `0x8006D860` | `jal 6CDA4(0, lb +0xE1, 0, +0x194, 0x21, 0)` |

`6CDA4` a0==0 is `87198` (`D_8009D270=1`) after the table fill,
then sb F0=7. 0x2F parks on that tick (v0==1). Next 0x2F tick
is state 7 `6E6D4`; host -1 sb F0=0 and 0x2F sb F2=0x30.
144FC 0x38 stays parked. No `0x39` / `6914C`.

## D_800B0E64 archive

Only two TEXT stores: `6A8D4` `@0x8006A9D0` and twin
`0x8006EB10`. Both write `D_80011614 - 8`. No PE.IMG range
is the 0x2A table. Boot `6E834` reads size 0 into
`D_80011614`. NYPD first 0x2A is empty (count 0) → 6D078
returns 0 into D79C. No payload hash.

## Next

`0x80086464` — 6D60C F2=0x30 after 0x2F `6E6D4` -1. Do not
stub `86464`/`86C1C`/`6914C`, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_6d60c_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
