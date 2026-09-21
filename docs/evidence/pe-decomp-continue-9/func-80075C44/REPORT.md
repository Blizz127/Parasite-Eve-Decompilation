# `func_80075C44` — GPU primitive packer twin (0xE6000002)

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, no maspsx
behavior gate. Integrated as matching-C leaf **765** (764 -> 765).

## Function hood

- File `[0x66444,0x6646C)` = `0x28` (10 words), VA `[0x80075C44,0x80075C6C)`.
- The `[0x66444, asm]` run had exactly this extent; it becomes the C span and
  the existing `[0x6646C, c, func_80075C6C]` twin follows directly.

## Retail body

```text
66444 80075C44 24020002 addiu v0,zero,0x2
66448 80075C48 A0820003 sb    v0,0x3(a0)
6644C 80075C4C 10A00002 beqz  a1,0x80075C58
66450 80075C50 3C03E600 lui   v1,0xE600
66454 80075C54 34630002 ori   v1,v1,0x2
66458 80075C58 0006102B sltu  v0,zero,a2
6645C 80075C5C 00431025 or    v0,v1,v0
66460 80075C60 AC820004 sw    v0,0x4(a0)
66464 80075C64 03E00008 jr    ra
66468 80075C68 AC800008 sw    zero,0x8(a0)
```

Semantics: `a0[3] = 2`; `v1 = (a1 != 0) ? 0xE6000002 : 0xE6000000`;
`*(u32*)(a0+4) = v1 | (a2 != 0)`; `*(u32*)(a0+8) = 0`.

## Verification

```text
try_leaf scratch2/func_80075C44.c 0x66444 0x28 -> WORDS MATCH
scripts/build_us.sh (distrobox pe-mipsel)       -> EXACT SHA-1
    cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
    Matching claim: YES (766 registered C leaves)
```
