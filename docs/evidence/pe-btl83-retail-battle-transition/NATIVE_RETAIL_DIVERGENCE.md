# Native vs retail — first implementable combat dispatch

No invented `4D4` / scratch bit 2 / mode 3 / mode 7 stores outside
the recovered ROM control flow.

## Capture firsts (completed BTL83)

Theater `M0064I` → field hops via `17BF8` → combat dest `0xA8001248`
(M0036I). Then:

1. `0x2A` / `0x55` (already ported)
2. `293F4(0)` HP copy
3. `2A7F8` mode==6 → jal `2BC90` (`s1=1`, `D2E8|=1`, `D1A0&=~4`)
4. Walk exhausts at `2CED8` into `2CEE0`
5. `6914C(0)==0` and `s1!=0` → `2CF24` stores mode 7
6. Opcode `0xCF` (`19D24`) jals `33A2C` → `sb 1, D244` (`4D4=1`)
7. `299CC` mode==0 && `4D4!=0` → jal `1D340` @ `2A4FC`
8. `1F704` subtracts HP 40→39, later 39→34

`0x800192BC` is `func_800192B8` (`sw $zero, D28C`), opcode `0x95`.
It is not a mode-7 store. Capture MODE_WRITES mislabeled that PC.

## Native miss that this rung recovers

`func_8006914C(0)` returned 1 for every EF==0 call, so `2CEE0`
could never take `2CF24`. ROM `0x800693B4` returns 0 when
`D1A0&2==0` (or when bit 1 is set and overlay bit 8 is clear).

`299CC` after-consume treated `mode!=0` as a silent return.
Retail `29A6C` goes to `2A7F8`, whose mode-6 arm jals `2BC90`.

Opcode `0xCF` was an unported table slot (advance / v0=0), so
`33A2C` never ran.

## Still not this cut

`21D4C` / `374E8` / actor walk `2CD40..2CEDC` / `6DE80` /
`20288` / HUD sb storm / teardown / field return. Type-6
`scratch[0]&4` wait on M0005I/M0367I is a different script
loop. Do not force the bit. `1F814` and the `1F080` death
stores are PE-BTL99.
