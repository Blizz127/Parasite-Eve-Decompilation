# func_80077BD8 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x683D8, asm]` and does not count this leaf.

Span: `0x683D8..0x683E3`, VA `0x80077BD8..0x80077BE3`; no calls, globals, or
relocations. Retail and built body words are `00000000 00000000 00000000`.
See [objdump.txt](objdump.txt).

```text
before  0x683d4  a0820007  sb v0,7(a0)
body    0x683d8  00000000  nop
body    0x683dc  00000000  nop
body    0x683e0  00000000  nop
after   0x683e4  2402000c  addiu v0,zero,12
```

The retained `.c` source is only a diagnostic file-scope-assembler
representation of the bytes, not a C decompilation or matching-C leaf.
