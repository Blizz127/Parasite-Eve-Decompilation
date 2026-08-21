# func_80033A2C — byte-flag setter

```text
MATCHING_C — era -O2 -G0, 5/5 words
vram        0x80033A2C..0x80033A40 exclusive
file        0x2422C size 0x14
```

`D_8009D244 = 1` produces retail's `li; lui; sb; jr; nop` sequence. The
former `2422C.s` segment resumes at `24240.s`.
