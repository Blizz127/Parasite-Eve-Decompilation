# Literal reconstruction — AF68..B04C

Executable SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Exclusive window `0x8006AF68..0x8006B04C` is **57 words** / `0xE4`.

```text
0x8006AF68  addu  s0, zero, zero
0x8006AF6C  s1 = &D_800930EE
0x8006AF74  s2 = -1
0x8006AF78  a1 = lw(s5+0x180)             # dest
            v0 = lhu(s1)                  # 197
            a2 = lhu(s1+2)                # 200
            a0 = s6 + v0                  # lba_base+197
0x8006AF88  jal   func_8006E6A8           # 3 sectors
             a2 = a2 - v0
0x8006AF90  beq   v0, s2, AF78            # retry on -1
0x8006AF98  s2 = 1
0x8006AF9C  bne   s0, zero, B044          # s0==0, take 718D0
0x8006AFA0   v0 = -1
0x8006AFA4  a0 = lw(s5+0x174)             # previous TIM, not +0x180
0x8006AFA8  jal   func_800718D0
0x8006AFB0  a1 = 0
0x8006AFB4  pack loop a1 = 0, 0x10; a1 < 0x20
0x8006AFF8  sh packed1, D_80091650(a1)    # record 0 tpage when a1=0
0x8006B02C  sh packed2, D_80091652(a1)    # record 0 CLUT when a1=0
0x8006B040  s0 = 1
0x8006B044  beq   s2, v0, AF6C            # s2==1, not taken
0x8006B04C  jal   func_8006E7E8           # EXCLUSIVE END — not consumed
```

`718D0` walks the TIM already completed by the AF54 wait
(`dest+0x174` = PE.IMG `[180,197)`). The new issue is
`D_800930EE..F0` = `[197,200)` into `dest+0x180`. It is in
flight and unpolled at this cut.
