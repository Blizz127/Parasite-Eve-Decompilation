# func_800182E0 — D_8009D2F0 offset-0x20 reader commit

```text
MATCHING_C — era -O2 -G0, 8/8 words
vram        0x800182E0..0x80018300 exclusive
file        0x8AE0 size 0x20
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The direct nested-pointer store emits retail's `lw; lui; lw; lw; nop; sw; jr;
li` sequence exactly. The asm remainder begins at `8B00.s`.
