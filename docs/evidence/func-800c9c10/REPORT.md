# func_800C9C10 — return-zero stub

## Verification

MATCHING_C — era -O2 -G0, 2/2 words
vram        0x800C9C10..0x800C9C18 exclusive
file        0xBA410 size 0x8
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_800C9C10` is the two-word `return 0` stub, reproduced exactly by era
`-O2 -G0`.
