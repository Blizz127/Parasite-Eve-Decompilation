# func_80077C38 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x68438, asm]` and does not count this leaf.

Span: `0x68438..0x68443`, VA `0x80077C38..0x80077C43`; no calls, globals, or
relocations. Retail and built body words are `00000000 00000000 00000000`.
See [objdump.txt](objdump.txt).

```text
before  0x68434  a0820007  sb v0,7(a0)
body    0x68438  00000000  nop
body    0x6843c  00000000  nop
body    0x68440  00000000  nop
after   0x68444  24020003  addiu v0,zero,3
```

Final C source: `src/func_80077C38.c`; exact three-nop file-scope body.
