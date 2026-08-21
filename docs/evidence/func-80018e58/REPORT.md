# func_80018E58 — one-reader call wrapper

## Verification

MATCHING_C — era -O2 -G0, 11/11 words
vram        0x80018E58..0x80018E84 exclusive
file        0x9658 size 0x2C
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018E58` dereferences its reader pointer, passes the value to
`func_80066800`, and returns 1. The verified era compiler output has the
retail 11-word stack, load, call, and return sequence.
