# func_80018EB4 — unsigned-halfword reader wrapper

## Verification

MATCHING_C — era -O2 -G0, 11/11 words
vram        0x80018EB4..0x80018EE0 exclusive
file        0x96B4 size 0x2C
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018EB4` reads an unsigned halfword through its reader pointer, passes
it to `func_80066B60`, and returns 1. The verified era output exactly matches
the retail stack, load, call, and return sequence.
