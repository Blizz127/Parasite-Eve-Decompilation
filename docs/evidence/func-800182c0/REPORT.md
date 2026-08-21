# func_800182C0 — D_800BCF88 bits-0xC0 setter

```text
MATCHING_C — era -O2 -G0, 8/8 words
vram        0x800182C0..0x800182E0 exclusive
file        0x8AC0 size 0x20
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The tied empty-asm barrier makes the `$v1` global-pointer pin live, reproducing
retail's `lui; addiu; lw; nop; ori; sw; jr; li` sequence exactly.
