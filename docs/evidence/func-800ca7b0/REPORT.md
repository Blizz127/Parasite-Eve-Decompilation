# func_800CA7B0 — return-zero stub twin

## Verification

MATCHING_C — era -O2 -G0, 2/2 words
vram        0x800CA7B0..0x800CA7B8 exclusive
file        0xBAFB0 size 0x8
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_800CA7B0` is the adjacent two-word `return 0` stub, reproduced exactly
by era `-O2 -G0`.
