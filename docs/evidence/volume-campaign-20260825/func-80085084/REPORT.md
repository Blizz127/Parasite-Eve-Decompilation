# func_80085084 — refreshed volume attempt 4

Outcome: MATCHED@era -O2 -G0, 5/5 words. Integrated as leaf 289.

## C1 / pool row

0x75884 | func_80085084 | 5 words | jr-ra | 6 direct callers | 0 jal in body | no gp | no indexed symbolic access | no loop | real/real boundaries | TIER 1

## C2 / function hood

- Span: file [0x75884,0x75898), VRAM [0x80085084,0x80085098), five words.
- Body ends in jr ra with addu v0,v0,v1 in the delay slot.
- Previous boundary at 0x80085080 is the real xori v0,v0,1 delay-slot instruction ending func_80084FE4.
- Following boundary at 0x80085098 is the real addiu sp,sp,-0x18 first instruction of func_80085098.
- Six unique direct jal callsites target the exact start:

- file 0x759CC / VA 800851CC
- file 0x76E2C / VA 8008662C
- file 0x779E4 / VA 800871E4
- file 0x77C68 / VA 80087468
- file 0x77E38 / VA 80087638
- file 0x7D478 / VA 8008CC78

FUNCTION_HOOD=PROVEN. This is callable code, not padding or a mislabeled span.

## C3 / screens and frame

| Screen | Result |
|---|---|
| Frame decomposition | args 0 + locals 0 + saves 0 = frame 0 |
| Callee buckets | none; no jal in the body |
| Stage-0 written globals | none; reads the caller-supplied word only |
| Coloring pressure | loaded value in v0; constant in v1; one live add at return |
| $v0 liveness | lw result remains in v0 through the return add |
| Address retention | none; a0 is used directly by lw |
| -O signal | one repeated 32-bit constant requires lui/ori; no loop or per-use materialization |
| Indexed symbolic gate | none |
| Loop/back-edge owner | none |

Flags: era -O2 -G0. G0 is justified by the absence of gp-relative accesses; no 3W, division, dispatch-fold, sched2, pin, or inline-asm mechanism applies.

## C4 / minimal C

```c
unsigned int func_80085084(const unsigned int *a0) {
    return *a0 + 0xB0BEB4BFu;
}
```

This first phrasing preserves the retail constant materialization order and the load/add return shape. No alternate phrasing, pins, or inline assembly was used.

## C5 / single-leaf object comparison

```text
00000000 <func_80085084>:
   0: 3c03b0be  lui v1,0xb0be
   4: 8c820000  lw v0,0(a0)
   8: 3463b4bf  ori v1,v1,0xb4bf
   c: 03e00008  jr ra
  10: 00431021  addu v0,v0,v1

ROM words:
  3C03B0BE 8C820000 3463B4BF 03E00008 00431021
C words:
  3C03B0BE 8C820000 3463B4BF 03E00008 00431021
RELOCS_NORMALIZED=none; BYTE_EXACT=5/5
```

## C6 / carve geometry

Prior asm span: [0x74FB0,0x75F28) = 0xF78.

```text
asm prefix:    0x75884 - 0x74FB0 = 0x8D4
C leaf:        0x75898 - 0x75884 = 0x14
asm resume:    0x75F28 - 0x75898 = 0x690
closure:       0x8D4 + 0x14 + 0x690 = 0xF78
```

The generated split created 75898.s as the trailing resume object; the trim guard accepted all three pieces.

## C7 / full gates

```text
build_us.sh:
RESULT: EXACT MATCH
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b

scripts/verify_us.sh:
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 289 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
289
```

## C8 / packed boundary span

Candidate and retail matched byte-for-byte across file [0x75880,0x7589C):

```text
075880: 38420001 = 38420001  preceding boundary (xori v0,v0,1)
075884: 3C03B0BE = 3C03B0BE  leaf word 1 (lui v1,0xB0BE)
075888: 8C820000 = 8C820000  leaf word 2 (lw v0,0(a0))
07588C: 3463B4BF = 3463B4BF  leaf word 3 (ori v1,0xB4BF)
075890: 03E00008 = 03E00008  leaf word 4 (jr ra)
075894: 00431021 = 00431021  leaf word 5 (addu v0,v0,v1)
075898: 27BDFFE8 = 27BDFFE8  following boundary (addiu sp,sp,-0x18)
PACKED_SPAN=EXACT
```

No pins, inline assembly, file-scope assembly, padding nops, or mismatch-hiding macros were used.
