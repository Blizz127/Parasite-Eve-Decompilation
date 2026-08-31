# `func_80036E34` — exact three-state initializer

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 377 and Tier-2 continuation rung 36.

## Function hood and screens

- File `[0x27634,0x27658)`, VA `[0x80036E34,0x80036E58)`: nine words,
  ending in canonical `jr ra; nop`.
- Exact direct caller `0x80036DD8` targets the start.
- Preceding real `func_80036DF8` ends at `0x2762C/0x27630`; following real
  twin `func_80036E58` starts immediately at `0x27658`.
- No callees, loop, address retention, or `$v0` result liveness. Coloring
  pressure is only the constant-one temporary used by the final store.
- Stage-0 finds `D_800A76BC` read at `0x8005C2B8` and written at
  `0x8005C3FC` and `0x8005D8E4`; `D_800A76B8` and `D_800A76C0` have no
  other exact-symbol executable references. This leaf is a three-word state
  initializer, not padding.
- Retail's three absolute `lui $at` store pairs prove `-G0`; there is no
  constant-materialization ambiguity and no maspsx gate is needed.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`.

## C and object

```c
extern int D_800A76B8;
extern int D_800A76BC;
extern int D_800A76C0;

void func_80036E34(void) {
    D_800A76BC = 0;
    D_800A76C0 = 0;
    D_800A76B8 = 1;
}
```

Flags: era `-O2 -G0`; no maspsx gate.

```text
27634: 24020001  addiu v0,zero,1
27638: 3c01800a  lui   at,%hi(D_800A76BC)
2763C: ac2076bc  sw    zero,%lo(D_800A76BC)(at)
27640: 3c01800a  lui   at,%hi(D_800A76C0)
27644: ac2076c0  sw    zero,%lo(D_800A76C0)(at)
27648: 3c01800a  lui   at,%hi(D_800A76B8)
2764C: ac2276b8  sw    v0,%lo(D_800A76B8)(at)
27650: 03e00008  jr    ra
27654: 00000000  nop

object: 24020001 3c010000 ac200000 3c010000 ac200000 3c010000 ac220000 03e00008 00000000
ROM/C:  24020001 3c01800a ac2076bc 3c01800a ac2076c0 3c01800a ac2276b8 03e00008 00000000
RELOCS_NORMALIZED=three R_MIPS_HI16/R_MIPS_LO16 symbol pairs
BYTE_EXACT=9/9
```

## Carve and packed gate

Former asm `[0x26C48,0x278A8)` = `0x0C60`:

```text
prefix: 0x27634 - 0x26C48 = 0x09EC
leaf:   0x27658 - 0x27634 = 0x0024
resume: 0x278A8 - 0x27658 = 0x0250
close:  0x09EC + 0x0024 + 0x0250 = 0x0C60
```

```text
27624: 3c01800a = 3c01800a  preceding real body
27628: ac2276a0 = ac2276a0
2762C: 03e00008 = 03e00008  preceding return
27630: 00000000 = 00000000
27634: 24020001 = 24020001  leaf 1
27638: 3c01800a = 3c01800a  leaf 2
2763C: ac2076bc = ac2076bc  leaf 3
27640: 3c01800a = 3c01800a  leaf 4
27644: ac2076c0 = ac2076c0  leaf 5
27648: 3c01800a = 3c01800a  leaf 6
2764C: ac2276b8 = ac2276b8  leaf 7
27650: 03e00008 = 03e00008  leaf 8
27654: 00000000 = 00000000  leaf 9
27658: 24020001 = 24020001  following real twin
2765C: 3c01800a = 3c01800a
27660: ac2076b0 = ac2076b0
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/build_us.sh: exit 0; Compare: EXACT SHA-1 MATCH
matching-C count: 377
```
