# func_80018C88 — two-reader call wrapper

## Verification

MATCHING_C — era -O2 -G0, 12/12 words
vram        0x80018C88..0x80018CB8 exclusive
file        0x9488 size 0x30
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018C88` dereferences each of its two reader pointers, passes the
resulting values to `func_800659F8`, and returns 1. The verified era compiler
output has the retail 12-word stack, loads, call, and return sequence.
