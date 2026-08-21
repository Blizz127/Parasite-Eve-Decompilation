# func_80018CB8 — three-reader call wrapper

## Verification

MATCHING_C — era -O2 -G0, 14/14 words
vram        0x80018CB8..0x80018CF0 exclusive
file        0x94B8 size 0x38
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018CB8` dereferences three reader pointers, passes the values to
`func_80065A60`, and returns 1. The verified era compiler output has the
retail 14-word stack, loads, call, and return sequence.
