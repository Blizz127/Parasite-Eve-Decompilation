# PE-BTL59 — opcode 0x2A and 3F3C4 pad/CE90 host cut

## 0x2A / `0x80017A50`

10 words `0x80017A50..0x80017A78`, SHA-256
`2f58ffe89e4130506d67a6784ea268503a8d0a6dec26bbe01506e2cec926db68`.
`D_800910A0[0x2A]`. `*arg0 |= (1 << *arg1)`; delay-slot `v0=1`.

Live type-0 `+0x1150` kinds `[4,0]` imms `[0,4]` writes `scratch[0] |= 0x10`
(bit 4). Type-6 waits on `scratch[0] & 4` (bit 2). **0x2A is not that
producer.**

## 3F3C4 first jal

`0x8003F3D4` is `jal 0x8003F074`. Full 3F074 also jals `34FC4` (rebuilds
the actor freelist and zeros `D20C`) and `125E0` every call. This cut
does **not** jal those. It runs the `3F244` `371B0` site only while
`CE90==0` and `B162C` is KSEG, after an optional `6B4F8_12574_publish_cut`
when `overlay+0x18C` is already a KSEG chunk2 dest.

## Host pad

Retail idle raw is `0xFFFF`. Host RAM zero after reset is uninitialized.
`3F3C4` writes `0xFFFF` when `BE9A2==0` and no window is open. A
state-2 message writes `0xBFFF` (raw Cross) for one tick so `0x22` can
complete. That is deterministic confirm of cursor 0 (Watch), not a
planted type-5 `D1F4`.
