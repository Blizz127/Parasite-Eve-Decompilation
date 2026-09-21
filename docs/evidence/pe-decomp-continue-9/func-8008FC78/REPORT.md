# `func_8008FC78` — cursor post-increment byte into state +0x82

Outcome: **MATCHED** on natural C phrasing 1 under era `-O1 -G0` (the new
`era_o1_g0` profile assignment). No maspsx behavior gate. Integrated as
matching-C leaf **764** (763 -> 764).

## Function hood

- File `[0x80478,0x804B4)` = `0x3C` (15 words), VA
  `[0x8008FC78,0x8008FCB4)`.
- Carved out of the `[0x80428, asm]` run: prefix `0x50`, C `0x3C`, and the
  existing `[0x804B4, c, func_8008FCB4]` span follows directly.

## Retail body

```text
80478 8008FC78 8C820000 lw    v0,0(a0)
8047C 8008FC7C 00000000 nop
80480 8008FC80 24430001 addiu v1,v0,1
80484 8008FC84 AC830000 sw    v1,0(a0)
80488 8008FC88 90420000 lbu   v0,0(v0)
8048C 8008FC8C 00000000 nop
80490 8008FC90 14400003 bne   v0,zero,0x8008FCA0
80494 8008FC94 A4820082 sh    v0,0x82(a0)
80498 8008FC98 24020100 li    v0,0x100
8049C 8008FC9C A4820082 sh    v0,0x82(a0)
804A0 8008FCA0 24020001 li    v0,1
804A4 8008FCA4 A48000E6 sh    zero,0xE6(a0)
804A8 8008FCA8 A4800080 sh    zero,0x80(a0)
804AC 8008FCAC 03E00008 jr    ra
804B0 8008FCB0 A4820084 sh    v0,0x84(a0)
```

Semantics: reads the byte at the cached cursor into `+0x82`, replacing `0`
with `0x100`; clears `+0xE6` and `+0x80`; writes `1` to `+0x84` and returns
that same value.

## Match note

The shared `int r` return value is what keeps the constant materialised once:
retail has a single `li v0,1` used both for the `+0x84` store and the return,
with that store filling the `jr` delay slot. This shape only survives at
`-O1 -G0`; at `-O2 -G0` the scheduler hoists the `+0x84` store ahead of the
two zero stores and the `+0x80` store takes the delay slot instead. The leaf
is therefore assigned to `era_o1_g0` in
`configs/USA/disc1_build_profiles.json`.

## Verification

```text
try_leaf scratch/e_8008FC78.c 0x80478 0x3C --flags "-O1 -G0" -> WORDS MATCH
scripts/build_us.sh (distrobox pe-mipsel)                    -> EXACT SHA-1
    cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
    Matching claim: YES (764 registered C leaves)
```
