# PE-BTL44 — type-2 0x6F / 18954 and 0x5A / 18164

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018954` — 10 words `0x80018954..0x8001897C`,
SHA-256 `9dc14475…a5ff`. `D_800910A0[0x6F]`.
`jal func_8002F7D8(*D_8009D2F0)`; v0=1.

`func_80018164` — 26 words `0x80018164..0x800181CC`,
SHA-256 `b2d7e11c…9c8f`. `D_800910A0[0x5A]`.

```text
if (D2F0+0x0C == 0)
    2FF78(lbu *arg0, *arg1)      # Aya / D254
else
    30220(D2F0, lbu *arg0, *arg1) # slot tags
```

`2F7D8` / `2FF78` / `30220` are already ported
(PE-CH1). Live type-2 is type 2, so `0x5A` takes
30220. First tags are 40/41/42 then 44/45/60/61
and the 90–110 block. Next unported on this arm
is `0xB7`.

If `(actor+0x98 & 0x2000)==0`, `2F7D8` records
`1A680(actor, 2)`. Do not invent that resource.

## Verify

```text
python3 pc_port/tools/pe_btl44_18954_18164_oracle.py
PE_TEST_FILTER=BTL44 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
