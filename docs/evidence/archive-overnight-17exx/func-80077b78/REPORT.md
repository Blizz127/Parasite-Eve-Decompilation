# func_80077B78 — seven-stub batch evidence

**Classification review:** representation-only diagnostic; **not matching C**.
The retail span is category C, alignment/padding between real functions. The
project retains `[0x68378, asm]` and does not count this leaf.

This leaf is the 12-byte span `0x68378..0x68383` / VA
`0x80077B78..0x80077B83`. It has no calls, globals, relocations, or live
state: all three retail words are zero (`nop`). The per-stub disassembly is
in [objdump.txt](objdump.txt).

Boundary proof:

```text
before  0x68374  a0820007  sb v0,7(a0)
body    0x68378  00000000  nop
body    0x6837c  00000000  nop
body    0x68380  00000000  nop
after   0x68384  24020006  addiu v0,zero,6
```

The C translation unit declares the symbol and emits the exact three-word
zero body through a file-scope assembler expression; an ordinary returning
C function would necessarily add `jr ra`. This is the minimal exact source
shape for a zero-filled code stub.
