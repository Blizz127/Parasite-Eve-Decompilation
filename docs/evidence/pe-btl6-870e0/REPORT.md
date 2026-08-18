# PE-BTL6 — func_800870E0 + 6CDA4 state 0xA

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. DMA-complete / 851A8 / 6914C are not
stubbed to succeed.

## Window

`0x800870E0..0x800870F0` exclusive, 4 words, SHA-256
`bc8b76454f526e929e58c6350e12408fbb4e627594ef3377e91234dd862a6b66`.

    lui $v0, 0x800a
    lw  $v0, D_8009D24C
    jr  $ra
    nop

No store in this leaf.

## Who writes D_8009D24C

| Site | Store | When |
|---|---|---|
| `0x800850C0` | 1 | 851A8 issue / `func_800850F4` arm |
| `0x80085098` | 0 | DMA callback after transfer |
| `0x8008526C` | -1 | 851A8 magic-check fail |

State 0xA only reads. Completing DMA by writing 0 from this
path would be a stub; the named cut does not do that.

## State 0xA (`0x8006CFE8`)

| 870E0 | Effect |
|---|---|
| -1 | sb F0=0, return 1 |
| != 0 && != -1 | stay 0xA, return 1 |
| 0 | remain `gp+0x408` -= chunk `gp+0x40C`, sb F0=7, return 1 |

Live flag=0 does not re-dispatch. After a real 87090!=-1 the
issue path leaves D24C=1, so the next tick parks at 0xA until
`85098` runs.

## Next

`0x80087198` — 6CDA4 a0=0 from F2=0x2F. See `pe-btl6-6d79c`.
Do not stub `6914C`, jump to mode 7, or complete `0x55`.

## Verify

```text
python3 pc_port/tools/pe_btl6_870e0_oracle.py
python3 pc_port/tools/pe_btl6_87090_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
