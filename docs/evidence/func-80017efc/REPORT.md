# func_80017EFC — D_8009D2F0 flag setter

```text
MATCHING_C — era -O2 -G0, 9/9 words
vram        0x80017EFC..0x80017F20 exclusive
file        0x86FC size 0x24
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The direct `|= 0x100` global field update emits retail's two loads, two nops,
`ori`, store, and return-1 slot exactly. The asm remainder begins at `8720.s`.
