# `func_80089218` — record-word rewrite sweep

Outcome: **MATCHED** on natural C phrasing 2 under era `-O2 -G0`, no maspsx
behavior gate. Integrated as matching-C leaf **768** (767 -> 768).

## Function hood

- File `[0x79A18,0x79A50)` = `0x38` (14 words), VA `[0x80089218,0x80089250)`.
- Carved from the `[0x780F0, asm]` run: prefix `0x1928`, C `0x38`, then a new
  `[0x79A50, asm]` resuming to `0x7A160`.

## Retail body

```text
79A18 80089218 21180000 addu  v1,zero,zero
79A1C 8008921C 18000624 addiu a2,zero,0x18
79A20 80089220 F0008424 addiu a0,a0,0xF0
79A24 80089224 0000828C lw    v0,0x0(a0)
79A28 80089228 00000000 nop
79A2C 8008922C 0200A214 bne   a1,v0,0x80089238
79A30 80089230 00000000 nop
79A34 80089234 000086AC sw    a2,0x0(a0)
79A38 80089238 01006324 addiu v1,v1,0x1
79A3C 8008923C 1800622C sltiu v0,v1,0x18
79A40 80089240 F8FF4014 bnez  v0,0x80089224
79A44 80089244 1C018424 addiu a0,a0,0x11C
79A48 80089248 0800E003 jr    ra
79A4C 8008924C 00000000 nop
```

Semantics: for `i` in `[0,0x18)`, rewrite the first word of the record at
`a0 + 0xF0 + i*0x11C` with `0x18` when it equals `a1`.

## Match note

The do/while over the offset form `a0 + 0xF0` is load-bearing: it emits
`i = 0` / `li a2,0x18` before the pointer bump, matching retail
`0x00001821 / 0x24060018 / 0x248400F0`. Pre-adjusting `a0` and using a `for`
loop moves the `addiu a0,a0,0xF0` first.

## Verification

```text
try_leaf scratch3/a_80089218.c 0x79A18 0x38 -> WORDS MATCH
scripts/build_us.sh (distrobox pe-mipsel)   -> EXACT SHA-1
    cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
    Matching claim: YES (768 registered C leaves)
```
