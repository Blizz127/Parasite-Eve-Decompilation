# func_80077C18 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x68418, asm]` and does not count this leaf.

Span: `0x68418..0x68423`, VA `0x80077C18..0x80077C23`; no calls, globals, or
relocations. Retail and built body words are `00000000 00000000 00000000`.
See [objdump.txt](objdump.txt).

```text
before  0x68414  a0820007  sb v0,7(a0)
body    0x68418  00000000  nop
body    0x6841c  00000000  nop
body    0x68420  00000000  nop
after   0x68424  24020002  addiu v0,zero,2
```

Final C source: `src/func_80077C18.c`; exact three-nop file-scope body.
