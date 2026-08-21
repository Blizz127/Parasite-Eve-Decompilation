# func_800CD970 — return-zero stub

## Verification

MATCHING_C — era -O2 -G0, 2/2 words
vram        0x800CD970..0x800CD978 exclusive
file        0xBE170 size 0x8
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

`func_800CD970` is the two-word `return 0` stub (`jr $ra; move $v0,$zero`).
Era `-O2 -G0` reproduces it exactly.
