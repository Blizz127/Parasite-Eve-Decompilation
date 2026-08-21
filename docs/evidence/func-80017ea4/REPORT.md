# func_80017EA4 — reader result commit

```text
MATCHING_C — era -O2 -G0, 8/8 words
vram        0x80017EA4..0x80017EC4 exclusive
file        0x86A4 size 0x20
```

The direct nested-pointer store emits retail's `lw/lui/lw/lw/nop/sw/jr/li`
sequence exactly. The former asm segment resumes at `86C4.s`.

The full Docker rebuild produces target SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
