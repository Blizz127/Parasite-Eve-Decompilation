# func_80018B00 — two-reader call wrapper

```text
MATCHING_C — era -O2 -G0, 12/12 words
vram        0x80018B00..0x80018B30 exclusive
file        0x9300 size 0x30
sha1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The natural two-nested-reader call reproduces retail's frame, argument loads,
`jal`, and teardown-in-return-delay-slot schedule. The asm remainder begins at
`9330.s`.
