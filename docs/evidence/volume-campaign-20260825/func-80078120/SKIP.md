# func_80078120 — refreshed volume screen — SDK/GTE tail wrapper

Outcome: SKIP-SDK-LIBRARY-GTE-TAIL. No matching-C attempt or leaf claim; count remains 289.

## Function hood

- Span: file [0x68920,0x68934), VRAM [0x80078120,0x80078134), five words.
- Body loads words at offsets 0, 4, and 8 from a0 into t0, t1, and t2, then executes b .L80078170 with a nop delay slot.
- The branch target 0x80078170 is inside the following func_80078134 span, where the shared path saves ra, calls handwritten func_80078194 COP2 code, stores three halfwords, restores ra, and returns.
- Previous boundary at 0x8007811C is the real addiu v0,zero,-1 delay-slot instruction ending func_80078094.
- Following boundary at 0x80078134 is the real first instruction of func_80078134.
- Three unique direct jal callsites target the exact start:

- file 0xB6E8C / VA 800C668C
- file 0xB6E98 / VA 800C6698
- file 0xB6EA4 / VA 800C66A4

FUNCTION_HOOD=PROVEN_BY_TAIL_JUMP. This is callable tail-entry code, not padding.

## Retail control-flow screen

```text
80078120: 8C880000  lw t0,0(a0)
80078124: 8C890004  lw t1,4(a0)
80078128: 8C8A0008  lw t2,8(a0)
8007812C: 10000010  b 0x80078170
80078130: 00000000  nop
```

The target is not a normal function return; it intentionally enters the middle of the adjacent wrapper. The shared continuation performs the GTE/COP2 operation through handwritten func_80078194. This is an SDK/libGTE architectural wrapper, not an ordinary standalone C leaf under R7.

| Screen | Result |
|---|---|
| Frame decomposition | no frame in this tail entry |
| Callee buckets | no jal in the five-word span; branch target owns the shared call |
| Stage-0 written globals | none |
| Coloring pressure | t0/t1/t2 are live across the cross-function tail entry |
| $v0 liveness | no return value is produced before the shared continuation |
| Address retention | none |
| -O signal | none; the defining mechanism is cross-function tail entry and COP2 |
| Loop/back-edge owner | none |

## Disposition

This span is removed from matching-C scheduling as SKIP-SDK-LIBRARY-GTE-TAIL. Attempting ordinary C would require reproducing a branch into another function’s interior and the handwritten COP2 continuation; R7 forbids inline assembly and mismatch-hiding constructs. The adjacent 78134/78194 SDK family remains separately represented in the pool/evidence and is not relabeled by this single screen.

Evidence is static retail disassembly plus the three exact-start callers; no YAML/build/verifier integration or count change is made.
