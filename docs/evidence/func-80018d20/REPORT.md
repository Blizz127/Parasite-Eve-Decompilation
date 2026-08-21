# func_80018D20 — complemented two-reader call wrapper

## Verification

MATCHING_C — era -O2 -G0, 12/12 words
vram        0x80018D20..0x80018D50 exclusive
file        0x9520 size 0x30
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018D20` dereferences two reader pointers, complements the second
value in the call delay slot, passes both to `func_80065A9C`, and returns 1.
