# `func_80036E58` — exact adjacent three-state initializer

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 378 and Tier-2 continuation rung 37.

## Function hood and screens

- File `[0x27658,0x2767C)`, VA `[0x80036E58,0x80036E7C)`: nine words,
  ending in canonical `jr ra; nop`.
- Exact direct caller `0x80036DE0` targets this start independently.
- Preceding accepted C `func_80036E34` ends immediately at `0x27654`;
  following real `func_80036E7C` starts immediately at `0x2767C`.
- No callees, loop, address retention, or result liveness. Coloring pressure
  is only the constant-one `$v0` used by the final store.
- Stage-0 finds another `D_800A76B0` writer at `0x8005D8DC`;
  `D_800A76AC` and `D_800A76B4` have no other exact-symbol executable
  references. The three absolute store pairs prove `-G0`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`.

## C and object

```c
extern int D_800A76AC;
extern int D_800A76B0;
extern int D_800A76B4;

void func_80036E58(void) {
    D_800A76B0 = 0;
    D_800A76B4 = 0;
    D_800A76AC = 1;
}
```

Flags: era `-O2 -G0`; no maspsx gate.

```text
27658: 24020001  addiu v0,zero,1
2765C: 3c01800a  lui   at,%hi(D_800A76B0)
27660: ac2076b0  sw    zero,%lo(D_800A76B0)(at)
27664: 3c01800a  lui   at,%hi(D_800A76B4)
27668: ac2076b4  sw    zero,%lo(D_800A76B4)(at)
2766C: 3c01800a  lui   at,%hi(D_800A76AC)
27670: ac2276ac  sw    v0,%lo(D_800A76AC)(at)
27674: 03e00008  jr    ra
27678: 00000000  nop

object: 24020001 3c010000 ac200000 3c010000 ac200000 3c010000 ac220000 03e00008 00000000
ROM/C:  24020001 3c01800a ac2076b0 3c01800a ac2076b4 3c01800a ac2276ac 03e00008 00000000
RELOCS_NORMALIZED=three R_MIPS_HI16/R_MIPS_LO16 symbol pairs
BYTE_EXACT=9/9
```

## Carve and packed gate

Former asm `[0x27658,0x278A8)` = `0x0250`:

```text
leaf:   0x2767C - 0x27658 = 0x0024
resume: 0x278A8 - 0x2767C = 0x022C
close:  0x0024 + 0x022C = 0x0250
```

```text
27648: 3c01800a = 3c01800a  preceding accepted leaf
2764C: ac2276b8 = ac2276b8
27650: 03e00008 = 03e00008
27654: 00000000 = 00000000
27658: 24020001 = 24020001  leaf 1
2765C: 3c01800a = 3c01800a  leaf 2
27660: ac2076b0 = ac2076b0  leaf 3
27664: 3c01800a = 3c01800a  leaf 4
27668: ac2076b4 = ac2076b4  leaf 5
2766C: 3c01800a = 3c01800a  leaf 6
27670: ac2276ac = ac2276ac  leaf 7
27674: 03e00008 = 03e00008  leaf 8
27678: 00000000 = 00000000  leaf 9
2767C: 8c870004 = 8c870004  following real body
27680: 3c088888 = 3c088888
27684: 35088889 = 35088889
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/build_us.sh: exit 0; Compare: EXACT SHA-1 MATCH
matching-C count: 378
```
