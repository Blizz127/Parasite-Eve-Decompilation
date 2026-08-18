# Literal reconstruction — B04C wait/reissue to B060

Executable SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
File offset `pc - 0x80010000 + 0x800`. Exclusive window
`0x8006B04C..0x8006B060` is **5 words** / `0x14`.

```text
0x8006B040  24100001  addiu $s0, $zero, 1        # already in B54F prefix
0x8006B044  1242FFC9  beq   $s2, $v0, 0x8006AF6C # reissue D_800930EE
0x8006B048   nop
0x8006B04C  0C01B9FA  jal   func_8006E7E8        # LIVE POLL — this rung
0x8006B050   nop
0x8006B054  00409021  addu  $s2, $v0, $zero
0x8006B058  1640FFD0  bne   $s2, $zero, 0x8006AF9C
0x8006B05C   nop
0x8006B060  00008021  addu  $s0, $zero, $zero    # EXCLUSIVE END
```

Canonical locals at first `B04C` visit: `s0 = 1` (`B040`), `s2 = 1`
(`AF98` after the already-issued `D_800930EE` read). `B044` is
therefore not taken on entry.

`AF9C` with `s0 != 0` is `bne → B044`, the wait head, not the
`718D0` walk. Production therefore does not re-run `func_800718D0`.

```text
s2 = func_8006E7E8()
  0   -> B060 cut
  !=0 -> AF9C -> B044
         s2 == -1 -> AF6C reissue (existing func_8006E6A8 of D_800930EE)
         then poll again
```

`s2` and the poll result are the live `func_8006E7E8` return. They
are not stored as constants.

## Not taken

```text
0x8006B060  s0 = 0
0x8006B080  jal func_8006E6A8     # D_800930F0 / dest+0x14C
0x8006B0A4  jal func_800718D0
0x8006B0AC  jal func_80030894
```
