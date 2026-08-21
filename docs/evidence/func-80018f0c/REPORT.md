# func_80018F0C — five-reader call wrapper

## Verification

MATCHING_C — era -O2 -G0, 18/18 words
vram        0x80018F0C..0x80018F54 exclusive
file        0x970C size 0x48
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_80018F0C` forwards five unsigned-halfword values through the standard
argument registers and stack slot to `func_80066BD8`, then returns 1. Era
`-O2 -G0` produces the retail 18-word sequence exactly.
