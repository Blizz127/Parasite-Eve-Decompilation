# func_80017F20 — D_8009D2F0 flag clearer

```text
MATCHING_C — era -O2 -G0, 9/9 words
vram        0x80017F20..0x80017F44 exclusive
file        0x8744 size 0x24
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The direct `&= ~0x100` update emits retail's global field clear sequence.
