# func_80077B98 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x68398, asm]` and does not count this leaf.

Span: `0x68398..0x683A3`, VA `0x80077B98..0x80077BA3`; no calls, globals, or
relocations. Retail and built body words are `00000000 00000000 00000000`.
See [objdump.txt](objdump.txt).

```text
before  0x68394  a0820007  sb v0,7(a0)
body    0x68398  00000000  nop
body    0x6839c  00000000  nop
body    0x683a0  00000000  nop
after   0x683a4  24020009  addiu v0,zero,9
```

The retained `.c` source is only a diagnostic file-scope-assembler
representation of the bytes, not a C decompilation or matching-C leaf.
