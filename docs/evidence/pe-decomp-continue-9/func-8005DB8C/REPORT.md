# `func_8005DB8C` — global word plus 512-strided record base

Outcome: **MATCHED** on natural C phrasing 2 under era `-O2 -G0`, no maspsx
behavior gate. Integrated as matching-C leaf **766** (765 -> 766).

## Function hood

- File `[0x4E38C,0x4E3AC)` = `0x20` (8 words), VA `[0x8005DB8C,0x8005DBAC)`.
- Carved from the `[0x4E2FC, asm]` run: prefix `0x90`, C `0x20`, then a new
  `[0x4E3AC, asm]` resuming to `0x4E3F8`.

## Retail body

```text
4E38C 8005DB8C 3C02800B lui   v0,0x800B
4E390 8005DB90 38804224 addiu v0,v0,-0x7FC8   ; &D_800A8038
4E394 8005DB94 00042240 sll   a0,a0,9
4E398 8005DB98 F0FF4324 addiu v1,v0,-0x10
4E39C 8005DB9C 0000428C lw    v0,0x0(v0)       ; D_800A8038
4E3A0 8005DBA0 21208300 addu  a0,a0,v1
4E3A4 8005DBA4 0800E003 jr    ra
4E3A8 8005DBA8 21104400 addu  v0,v0,a0
```

Semantics: `D_800A8038 + (a0 << 9) + (&D_800A8038 - 0x10)`.

## Match note

The `int x = a0 << 9;` temporary is load-bearing: it forces the `sll` ahead of
`addiu v1,v0,-0x10` in the emitted schedule. Folding the shift into the final
expression lets the scheduler swap the two (`addiu` then `sll`), which no
longer matches retail words `0x00042240` / `0x2443FFF0`.

## Verification

```text
try_leaf scratch2/c_8005DB8C.c 0x4E38C 0x20 -> WORDS MATCH
scripts/build_us.sh (distrobox pe-mipsel)   -> EXACT SHA-1
    cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
    Matching claim: YES (766 registered C leaves)
```
