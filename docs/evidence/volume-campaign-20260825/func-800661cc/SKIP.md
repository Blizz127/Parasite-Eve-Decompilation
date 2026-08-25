# `func_800661CC` — screened handwritten COP2 helper

No C leaf is claimed. Matching-C count remains 302.

## Function hood

Retail span `[0x569CC,0x569EC)`, VRAM `0x800661CC`, eight words. It ends in
`jr ra` with `move v0,zero` in the delay slot. The preceding word at
`0x569C8` is the real return delay slot of `func_800661A4`; the following word
at `0x569EC` is the real `lui t1,%hi(D_800BCF88)` start of
`func_800661EC`.

Seven exact direct callers occur at `0x8003492C`, `0x80035B34`,
`0x8003F580`, `0x800695D4`, `0x8006969C`, `0x8006CD80`, and
`0x8006E068`. `FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail semantics

The split marks the function handwritten. Its complete body is:

```text
addiu v1,zero,0xA0
addiu a0,zero,0x70
sll   t4,v1,16
sll   t5,a0,16
ctc2  t4,$24
ctc2  t5,$25
jr    ra
move  v0,zero
```

GTE control registers 24 and 25 are OFX and OFY. Existing projection evidence
in `docs/evidence/pe-vis1b-retail-projection/PROJECTION_MATH.md` proves this
helper restores the screen origin to `(160,112)` in 16.16 form after the
actor-origin projection path.

## Screen and disposition

This is a real PE1 projection helper, not padding. It is also not assigned to
the contiguous libGTE SDK family at `func_80078E04..func_80079024`: the
instruction-level role is proven, but SDK provenance for this game-specific
reset wrapper is not.

Its semantic body requires two `ctc2` instructions. The repository-wide COP2
screen already established that the sanctioned ordinary-C subset has no
intrinsic for this side effect; pins and inline assembly are forbidden. An
ordinary-C attempt would only rediscover the known expressibility blocker, so
no campaign attempt was spent.

`POOL_DISPOSITION=SKIP-HANDWRITTEN-COP2`

`INTEGRATION=NONE`

`MATCHING_C_COUNT=302`
