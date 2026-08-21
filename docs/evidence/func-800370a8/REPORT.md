# func_800370A8 — fixed-point quotient helper

```text
MATCHING_C — era -O2 -G0, 5/5 words
vram        0x800370A8..0x800370BC exclusive
file        0x278A8 size 0x14
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

`(value / (divisor >> 8)) << 8` emits the retail `sra; div; mflo; jr; sll`
sequence. The former `26C48.s` segment is split at `0x278A8` and resumes at
`278BC.s`. Full Docker-backed rebuild produced the target SHA-1.
