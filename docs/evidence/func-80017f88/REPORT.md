# func_80017F88 — D_8009D1A0 dynamic bit setter

```text
MATCHING_C — era -O2 -G0, 10/10 words
vram        0x80017F88..0x80017FB0 exclusive
file        0x8788 size 0x28
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The nested mask load and direct `|=` global update reproduce retail's two
loads, load-delay nop, `or`, and return-1 schedule. The asm remainder resumes
at `87B0.s`.
