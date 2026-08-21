# func_80017FB0 — D_8009D1A0 dynamic bit clearer

```text
MATCHING_C — era -O2 -G0, 11/11 words
vram        0x80017FB0..0x80017FDC exclusive
file        0x87B0 size 0x2C
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The nested mask load and direct `&= ~` global update reproduce retail's two
loads, load-delay nop, `nor`, `and`, and return-1 schedule exactly.
