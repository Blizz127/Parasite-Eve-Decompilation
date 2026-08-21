# func_8005288C — return-zero stub

## Verification

MATCHING_C — era -O2 -G0, 2/2 words
vram        0x8005288C..0x80052894 exclusive
file        0x4308C size 0x8
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_8005288C` is the two-word `return 0` stub. Era `-O2 -G0` preserves
retail's `addu $v0,$zero,$zero` delay-slot encoding.
