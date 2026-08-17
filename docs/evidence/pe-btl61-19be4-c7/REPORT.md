# PE-BTL61 — type-6 scratch polarity and 0xC7 / 0x8B / 0x94 / 0xAD

## Scratch[0]&4 polarity (corrected)

Type-6 `+0x190`: `cond[0] = scratch[0] & 4`, `cond[1] = NOT cond[0]`,
`0x05` skip-if-false uses `base + (imm << 1)`. Imm `0xF4` is
`+0x1E8` (`0x12` fork). Live scratch is 0 after 34F10, so the
script **does not wait**. The wait loop is taken only while the
bit is **set**. Do not force the bit.

Type-6 `+0x1850` `0x2A` imms `[0,2]` is a later aligned setter of
bit 2, reached from `+0x1138`. It is not the first-visit producer.

## Handlers

| op | VA | words | SHA-256 |
|---|---|---|---|
| 0xC7 | `0x80019BE4` | 8 | `ad177402…043a` |
| 0xAD | `0x80019748` | 8 | `6776e7eb…4327` |
| 0x94 | `0x80019154` | 7 | `d126b80d…8c97` (CH1; now dispatched) |
| 0x8B | `0x80018080` | 57 | `a896f3b9…d1c7` |

0xC7: `lw $v1, 0x590($gp)` = D300; `lhu`/`ori 0x80`/`sh` at +8.
0xAD: `D_8009D2E8 &= ~4`. 0x94: `*arg0 = D28C`.
0x8B: `*arg0==0` → `2FE78(lbu *arg2)` and store `*arg3`.
Else walk `D20C` for `+0xC==*arg0` and `+0xD==*arg1` with
`+0x98&0x10` clear, then `3010C`. Exhausted list / bit-set-only
matches skip the dest store (`beqz a0, +0x1814C`).

## Live type-6 after bit-clear `0x12`

Main `+0x1F4`: `0x02` / `0xC7` / two `0x8B (2,0,0x2C,local)`.
Equal reads skip `0x5E` to `+0x2FC`. `persist[0xA]&1` then
selects `0x94`/`0x89` vs `+0x414`.
Fork `+0x630`: `0x94` vs 10 selects `0x40`/`0xAA`/`0xAD` vs
`+0x77C`.
