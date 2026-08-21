# func_80018CF0 — two-reader call wrapper

## Verification

MATCHING_C — era -O2 -G0, 12/12 words
vram        0x80018CF0..0x80018D20 exclusive
file        0x94F0 size 0x30
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018CF0` dereferences each of its two reader pointers, passes the
resulting values to `func_80065A9C`, and returns 1. The verified era compiler
output has the retail 12-word stack, loads, call, and return sequence.
