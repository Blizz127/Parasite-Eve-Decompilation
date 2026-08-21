# func_800182A0 — D_800BCF88 bits-0xC0 clearer

```text
MATCHING_C — era -O2 -G0, 8/8 words
vram        0x800182A0..0x800182C0 exclusive
file        0x8AA0 size 0x20
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The tied empty-asm barrier makes the `$v0` global-pointer pin live, yielding
retail's `lui; addiu; lw; addiu; and; sw; jr; li` sequence exactly.
