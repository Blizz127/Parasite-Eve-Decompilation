# func_80018EE0 — unsigned-halfword reader wrapper twin

## Verification

MATCHING_C — era -O2 -G0, 11/11 words
vram        0x80018EE0..0x80018F0C exclusive
file        0x96E0 size 0x2C
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018EE0` reads an unsigned halfword through its reader pointer, passes
it to `func_80066C7C`, and returns 1. The verified era output exactly matches
the retail stack, load, call, and return sequence.
