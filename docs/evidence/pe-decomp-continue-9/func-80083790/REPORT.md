# `func_80083790` — signed-shift span packer

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, no maspsx
behavior gate. Integrated as matching-C leaf **763** (762 -> 763).

## Function hood

- File `[0x73F90,0x73FC8)` = `0x38` (14 words), VA
  `[0x80083790,0x800837C8)`.
- Carved out of the `[0x73DC0, asm]` run: prefix `0x1D0`, C `0x38`, then a new
  `[0x73FC8, asm]` segment resuming to `0x74420`.

## Retail body

```text
73F90 80083790 908200E3 lbu   v0,0xE3(a0)
73F94 80083794 908500E9 lbu   a1,0xE9(a0)
73F98 80083798 8C8400EC lw    a0,0xEC(a0)
73F9C 8008379C 24420001 addiu v0,v0,1
73FA0 800837A0 00021043 sra   v0,v0,1
73FA4 800837A4 00021080 sll   v0,v0,2
73FA8 800837A8 00051880 sll   v1,a1,2
73FAC 800837AC 00651821 addu  v1,v1,a1
73FB0 800837B0 24630003 addiu v1,v1,3
73FB4 800837B4 30630FFC andi  v1,v1,0xFFC
73FB8 800837B8 24630004 addiu v1,v1,4
73FBC 800837BC 00431021 addu  v0,v0,v1
73FC0 800837C0 03E00008 jr    ra
73FC4 800837C4 00441021 addu  v0,v0,a0
```

Semantics: `((obj[0xE3]+1) >> 1) << 2` plus `((5*obj[0xE9]+3) & 0xFFC) + 4`
plus the 32-bit little-endian value at `obj+0xEC`.

## Match note

The shifted expression must remain **signed**. Declaring the accumulator as
`unsigned int` lets era cc1 prove the loaded byte is in `[0,255]` and emit
`srl` (retail has `sra`, `0x00021043`). Keeping the accumulator `int` produces
the retail `sra`.

## Verification

```text
try_leaf scratch/func_80083790.c 0x73F90 0x38   -> WORDS MATCH
scripts/build_us.sh (distrobox pe-mipsel)       -> EXACT SHA-1
    cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
    Matching claim: YES (764 registered C leaves)
```
