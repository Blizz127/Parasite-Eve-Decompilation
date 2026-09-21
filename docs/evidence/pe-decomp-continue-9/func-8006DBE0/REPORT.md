# `func_8006DBE0` — two-entry signed-byte id search

Outcome: **MATCHED** on natural C phrasing 2 under era `-O2 -G0`, no maspsx
behavior gate. Integrated as matching-C leaf **767** (766 -> 767).

## Function hood

- File `[0x5E3E0,0x5E418)` = `0x38` (14 words), VA `[0x8006DBE0,0x8006DC18)`.
- Carved from the `[0x5E39C, asm]` run: prefix `0x44`, C `0x38`, then a new
  `[0x5E418, asm]` resuming to `0x5EC54`.

## Retail body

```text
5E3E0 8006DBE0 00001821 addu  v1,zero,zero
5E3E4 8006DBE4 0B80053C lui   a1,0x800B
5E3E8 8006DBE8 D80CA524 addiu a1,a1,0xCD8
5E3EC 8006DBEC DC00A280 lb    v0,0xDC(a1)
5E3F0 8006DBF0 00000000 nop
5E3F4 8006DBF4 06004410 beq   v0,a0,0x8006DC10
5E3F8 8006DBF8 21106000 addu  v0,v1,zero
5E3FC 8006DBFC 01006324 addiu v1,v1,0x1
5E400 8006DC00 02006228 slti  v0,v1,0x2
5E404 8006DC04 F9FF4014 bnez  v0,0x8006DBEC
5E408 8006DC08 0200A524 addiu a1,a1,0x2
5E40C 8006DC0C FFFF0224 addiu v0,zero,-0x1
5E410 8006DC10 0800E003 jr    ra
5E414 8006DC14 00000000 nop
```

Semantics: linear search of the two signed bytes at `D_800B0CD8+0xDC` (stride
2); returns the matching index, else `-1`.

## Match note

The address must be formed through the extern symbol (`p = &D_800B0CD8;`). A
literal `(unsigned char *)0x800B0CD8` makes cc1 emit `ori a1,a1,0xCD8`
(`0x34A50CD8`), while the symbol form emits the retail `addiu a1,a1,0xCD8`
(`0x24A50CD8`).

## Verification

```text
try_leaf scratch3/b_8006DBE0.c 0x5E3E0 0x38 -> WORDS MATCH
scripts/build_us.sh (distrobox pe-mipsel)   -> EXACT SHA-1
    cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
    Matching claim: YES (768 registered C leaves)
```
