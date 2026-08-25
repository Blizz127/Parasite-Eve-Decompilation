# func_80077BB8 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x683B8, asm]` and does not count this leaf.

Span: `0x683B8..0x683C3`, VA `0x80077BB8..0x80077BC3`; no calls, globals, or
relocations. Retail and built body words are `00000000 00000000 00000000`.
See [objdump.txt](objdump.txt).

```text
before  0x683b4  a0820007  sb v0,7(a0)
body    0x683b8  00000000  nop
body    0x683bc  00000000  nop
body    0x683c0  00000000  nop
after   0x683c4  24020008  addiu v0,zero,8
```

The retained `.c` source is only a diagnostic file-scope-assembler
representation of the bytes, not a C decompilation or matching-C leaf.
