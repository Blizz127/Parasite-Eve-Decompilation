# func_80018F54 — D_800BCFEE bit-0x40 clearer

## Verification

MATCHING_C — era -O2 -G0, 8/8 words
vram        0x80018F54..0x80018F74 exclusive
file        0x9754 size 0x20
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b

An explicit local pointer to `D_800BCFEE` preserves retail's `$v1`
`lui/addiu` address materialization. The C leaf clears bit `0x40` and returns
1, matching the retail 8-word sequence exactly.
