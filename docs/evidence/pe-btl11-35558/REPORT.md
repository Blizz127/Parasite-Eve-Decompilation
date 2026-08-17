# PE-BTL11 — func_80035558 D20C walk; type!=0 vtable 35E04

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## Recurring entry

`func_8003F3C4` is the field tick. Sole TEXT caller
`0x800123D8`. After `65400` it jals `35558` at `0x8003F4F0`.

`func_80035558` is 459 words `0x80035558..0x80035C84`,
SHA-256 `7352fc04…ba83`. Sole caller that site. Prologue:

```text
if (D_8009D1A0 & 4): skip walk
s1 = D_8009D20C
while s1:
    jalr *(s1+0x190) (a0=s1)
    s1 = *(s1+4)
```

The remaining 29 jals (including `6C5BC` at `35B24`) are not
this cut. This is the field-tick actor walk, not a dedicated
battle overlay. `M2_battle_overlay_entry` stays NO.

## Type!=0 vtable

EXE rodata `D_800915DC`: type 0 → `0x80035C84`; types 1–9 →
`0x80035E04`. `35038` stores that pair at actor `+0x190/+0x194`.

`func_80035E04` is 83 words, SHA-256 `84807116…5c6d`.
`D1A0` bit `0x100` skips the pose snapshot. Else copy
`+0x28/+0x38` → `+0x40/+0x50`, jal `361F4`, then if `+0x98`
bit 1 integrate motion. Live empty-`+0x1AC` ctor leaves that
bit clear.

`func_800361F4` is 24 words, SHA-256 `1ebed0df…cbe5`.
`sw actor → gp+0x580` (`D_8009D2F0`), then three words at
`+0xA0` into `D_8009D300`. Nonempty slots jal `17018`
(159 words, 0 jals, sole caller; task VM; not this cut).
Live `35038` zeros `+0xA0/+0xA4` and stores the `12700` task
at `+0xA8`.

Type0 `35C84` is not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl11_35558_oracle.py
PE_TEST_FILTER=BTL11 ./pc_port/build/pe-native-tests
```
