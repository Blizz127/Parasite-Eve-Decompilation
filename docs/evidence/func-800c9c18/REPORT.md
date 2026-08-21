# func_800C9C18 — return-zero stub twin

## Verification

MATCHING_C — era -O2 -G0, 2/2 words
vram        0x800C9C18..0x800C9C20 exclusive
file        0xBA418 size 0x8
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_800C9C18` is the adjacent two-word `return 0` stub, reproduced exactly
by era `-O2 -G0`.
