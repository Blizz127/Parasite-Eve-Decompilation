# func_80062A20 — refreshed volume attempt 2

Outcome: MATCHED@era -O2 -G0, 5/5 words. Integrated as leaf 288.

## C1 / pool row

0x53220 | func_80062A20 | 5 words | jr-ra | 39 direct callers | 0 jal in body | no gp | no symbolic global | no loop | real/real boundaries | TIER 1

## C2 / function hood

- Span: file [0x53220,0x53234), VRAM [0x80062A20,0x80062A34), five words.
- Body ends in jr ra with delay-slot nop.
- Previous boundary at 0x80062A1C is the real addu v0,a3,zero delay-slot instruction ending func_800629BC.
- Following boundary at 0x80062A34 is the real first instruction of func_80062A34 (lw v1,0x3E4(gp)).
- Exact-start direct jal references, collected from the retail disassembly, are:

- file 0x345C4 / VA 80043DC4
- file 0x34C68 / VA 80044468
- file 0x35348 / VA 80044B48
- file 0x356B4 / VA 80044EB4
- file 0x36528 / VA 80045D28
- file 0x36684 / VA 80045E84
- file 0x36A34 / VA 80046234
- file 0x36EE8 / VA 800466E8
- file 0x37378 / VA 80046B78
- file 0x379FC / VA 800471FC
- file 0x37B68 / VA 80047368
- file 0x37CE4 / VA 800474E4
- file 0x37E04 / VA 80047604
- file 0x37FB0 / VA 800477B0
- file 0x38254 / VA 80047A54
- file 0x38584 / VA 80047D84
- file 0x3875C / VA 80047F5C
- file 0x39050 / VA 80048850
- file 0x39754 / VA 80048F54
- file 0x399DC / VA 800491DC
- file 0x39A40 / VA 80049240
- file 0x39B68 / VA 80049368
- file 0x39CC8 / VA 800494C8
- file 0x3B638 / VA 8004AE38
- file 0x3B7C0 / VA 8004AFC0
- file 0x3B8C0 / VA 8004B0C0
- file 0x3BA24 / VA 8004B224
- file 0x3BBB0 / VA 8004B3B0
- file 0x3BC68 / VA 8004B468
- file 0x3BCCC / VA 8004B4CC
- file 0x3DAF0 / VA 8004D2F0
- file 0x3DEF4 / VA 8004D6F4
- file 0x3E888 / VA 8004E088
- file 0x3F4C8 / VA 8004ECC8
- file 0x3F780 / VA 8004EF80
- file 0x3F7D0 / VA 8004EFD0
- file 0x3F7FC / VA 8004EFFC
- file 0x3FBAC / VA 8004F3AC
- file 0x4026C / VA 8004FA6C

FUNCTION_HOOD=PROVEN. The 39 unique callsites establish callable function-hood; both boundary words are real instructions.

## C3 / screens and frame

| Screen | Result |
|---|---|
| Frame decomposition | args 0 + locals 0 + saves 0 = frame 0 |
| Callee buckets | none; no jal in the five-word body |
| Stage-0 written globals | none; the function only reads through its arguments |
| Coloring pressure | index/address live in $a1; return value in $v0 |
| $v0 liveness | load result is returned directly in the jr delay sequence |
| Address retention | $a1 is the scaled-index/address temporary; this is the address-retention pattern visible in retail |
| -O signal | none; no loop or repeated constant; -O2 is the default era shape for this straight-line getter |
| Indexed symbolic gate | none; base is an argument, not a symbolic global |
| Loop/back-edge owner | none |

Flags: era -O2 -G0. G0 is justified because the function has no gp-relative global access; no 3W, division, dispatch-fold, sched2, or pin/asm mechanism applies.

## C4 / minimal C

```c
unsigned int func_80062A20(unsigned int *a0, unsigned int a1) {
    return ((unsigned int *)((a1 << 2) + (unsigned int)a0))[2];
}
```

The pointer-plus-scaled-index phrasing preserves the retail address temporary in $a1. A first equivalent pointer-local phrasing produced addu a0,a0,a1; the alternate expression above produced the retail addu a1,a1,a0. This was the second and final phrasing iteration; no pins or inline assembly were used.

## C5 / single-leaf object comparison

```text
00000000 <func_80062A20>:
   0: 00052880  sll  a1,a1,0x2
   4: 00a42821  addu a1,a1,a0
   8: 8ca20008  lw   v0,8(a1)
   c: 03e00008  jr   ra
  10: 00000000  nop

ROM words:
  00052880  00a42821  8ca20008  03e00008  00000000
C words:
  00052880  00a42821  8ca20008  03e00008  00000000
RELOCS_NORMALIZED=none; BYTE_EXACT=5/5
```

## C6 / carve geometry

Prior asm span: [0x531BC,0x53234) = 0x78.

```text
asm prefix:  0x53220 - 0x531BC = 0x64
C leaf:      0x53234 - 0x53220 = 0x14
closure:     0x64 + 0x14 = 0x78
```

The generated split assembly was regenerated after the YAML carve; the trim guard then accepted the 0x64 prefix with no nonzero bytes beyond the boundary. The following C leaf func_80062A34 remains at 0x53234.

## C7 / full gates

```text
build_us.sh tail:
RESULT: EXACT MATCH
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b

scripts/verify_us.sh:
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 288 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
288
```

## C8 / packed boundary span

Candidate and retail matched byte-for-byte across file [0x5321C,0x53238):

```text
05321C: 00E01021 = 00E01021  preceding boundary (addu v0,a3,zero)
053220: 00052880 = 00052880  leaf word 1 (sll a1,a1,2)
053224: 00A42821 = 00A42821  leaf word 2 (addu a1,a1,a0)
053228: 8CA20008 = 8CA20008  leaf word 3 (lw v0,8(a1))
05322C: 03E00008 = 03E00008  leaf word 4 (jr ra)
053230: 00000000 = 00000000  leaf word 5 (nop)
053234: 8F8303E4 = 8F8303E4  following boundary (lw v1,0x3E4(gp))
PACKED_SPAN=EXACT
```

No pins, inline assembly, file-scope assembly, padding nops, or mismatch-hiding macros were used.
