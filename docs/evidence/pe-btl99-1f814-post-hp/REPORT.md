# PE-BTL99 — 1F814 after 1F704 (audit)

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
BTL83 capture: first HP store `1F704` `ra=1F5F0` 40→39.
No matching `src/` C. No UE5 tree writes.

## Boundary

`func_8001F4D4` does **not** return at the `1F704` `sh`.
Words `1F708..1F810` are the same function.

```
1F704  sh     HP = HP - s0          # BTL98 cut ended here
1F708  lw     D278
1F714  lh     D278+0x0C
1F71C  beq    HP==0 → 1F7D8         # skip 1F814
1F724  jal    1F814(a0=s2 actor)    # HP != 0
1F72C  lw     D254
1F738  lw     D254+0x98
1F740  andi   0x100
1F744  bne    set → 1F7D8
1F750  sw     s2 → D_8009D1D0
1F754  j      1F7D8
1F7D8  lbu    body+0x18 → weap+1
1F7E8  beq    0 → epilogue
1F7F0  jal    20288(a0=body)        # deferred
1F7F8  epilogue / jr
```

First retail delta is 40→39, so HP!=0 and `1F814` is live.
Do not poke HP to 0 to skip it. `20288` is not this cut.

`func_8001F814` is `0x8001F814..0x8001F9C4` exclusive
(0x1B0 bytes / 108 words). Next function at `1F9C4`.

## 1F814 CFG

```
s0 = 0
kind = lbu(D254+0x0E)
idx  = kind - 6
if idx >= 10u: goto common            # Aya+0x0E not in 6..15
jr  jtbl[idx] at 0x800106E4
```

Jump table (Aya+0x0E → target):

| +0x0E | Target | Arm |
|---|---|---|
| 6,8,10,12,13,14,15 | `1F860` | copy +0x0E/+0x0F to D29A/D29B; `sh 1, gp+0x528`; D29C = `lw D254+0x14` |
| 7,9,11 | `1F898` | same D29A/D29B / gp+0x528; D29C = `(lbu D254+0x0F) << 16` |

Both arms join at `1F8D0` then `1F8D8`.

Common tail:

```
if (D278+0x4C) & 0x00012000: return (s0 as i16)
jal 305C8(a0=target_actor, a1=D254) → s0
class = f(s0 as i16):
  <512 → 0
  <1536 → 2
  <2560 → 1
  <3584 → 0 or 3 via 0/-1 & 3
jal 1A680(D254, class)
jal 6DE80(0x46A, 0, D254+0x2A, D254+0x2E, stack D254+0x32)
if D254+0x98 & 0x100:
  clear that bit
  sh 2, gp+0x528
return (s0 as i16)
```

## Callees

| VA | Status on this host |
|---|---|
| `305C8` | NONMATCHING_C; called from `1F814` on PE-BTL99 |
| `1A680` | native `func_8001A680_command_cut`; live from `1F814` |
| `6DE80` | NONMATCHING_C wrapper; `6DFA8`/`6DF50` parked |
| `20288` | deferred (1F4D4 epilogue, not 1F814) |

## Classification

`func_8001F814`: NONMATCHING_C (native cut). Not MATCHED.
Jump-table stores plus `305C8`/`1A680`/`6DE80` are live.
Death is not at `1F4D4`. See `docs/evidence/pe-btl99-6de80/`.
Do not poke HP.
