# func_80018B68 — two-reader call wrapper twin

```text
MATCHING_C — era -O2 -G0, 12/12 words
vram        0x80018B68..0x80018B98 exclusive
file        0x9368 size 0x30
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The natural two-nested-reader call matches the `18B00` wrapper frame and
teardown schedule while targeting `func_8006590C`. The asm remainder is
`9398.s`.
