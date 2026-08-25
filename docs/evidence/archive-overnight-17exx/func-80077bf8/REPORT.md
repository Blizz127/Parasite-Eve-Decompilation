# func_80077BF8 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x683F8, asm]` and does not count this leaf.

Span: `0x683F8..0x68403`, VA `0x80077BF8..0x80077C03`; no calls, globals, or
relocations. Retail and built body words are `00000000 00000000 00000000`.
See [objdump.txt](objdump.txt).

```text
before  0x683f4  a0820007  sb v0,7(a0)
body    0x683f8  00000000  nop
body    0x683fc  00000000  nop
body    0x68400  00000000  nop
after   0x68404  24020004  addiu v0,zero,4
```

The retained `.c` source is only a diagnostic file-scope-assembler
representation of the bytes, not a C decompilation or matching-C leaf.
