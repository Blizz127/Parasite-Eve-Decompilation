# `func_80087864` — volatile-store scheduling family screen

`PARKED-VOLATILE-STORE-SCHEDULING-FAMILY`. No C attempt was spent because
the complete body reproduces the just-bounded `func_8008783C` mechanism.

## Function hood

Retail span `[0x78064,0x7808C)`, VRAM `0x80087864`, ten words. It ends in
the canonical `jr ra` with `sh v0,0(a0)` in the delay slot. The exact direct
caller is at `0x80087A80`. The preceding `func_8008783C` ends at
`0x7805C/0x78060`, and the next real function begins at `0x7808C`; there is
no boundary padding or ownership ambiguity.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER`.

## Retail body and family proof

```text
3C021F80  lui   v0,0x1F80
34421C08  ori   v0,v0,0x1C08
00042100  sll   a0,a0,4
00822021  addu  a0,a0,v0
94820000  lhu   v0,0(a0)
00000000  nop
3042FFF0  andi  v0,v0,0xFFF0
00451025  or    v0,v0,a1
03E00008  jr    ra
A4820000  sh    v0,0(a0)
```

This updates the same per-voice SPU register at
`0x1F801C08 + voice*0x10`, replacing bits 0..3 rather than bits 4..7. It has
the same retained `$a0` MMIO pointer and the same direct volatile halfword
store in the return slot as `func_8008783C`.

The two bounded `8783C` phrasings—void volatile RMW and explicit returned
value—both emitted `sh; jr; nop` after otherwise exact address/data flow.
The extra `a1 << 4` in `8783C` is before the mask/merge and does not own the
return-slot decision. Removing it here cannot supply a different delay-slot
lever. The existing maspsx store-fill rule is limited to absolute symbolic
`sw` macros and does not rewrite direct `sh 0(a0)`.

No source, YAML, build, or verifier change was made. Matching-C remains 311,
and this no-attempt family screen does not increment the consecutive-park
counter.
