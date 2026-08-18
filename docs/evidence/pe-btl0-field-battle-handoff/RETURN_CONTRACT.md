# RETURN_CONTRACT

## Scene

No `0x31` on the first `0x89` path. Resume scene is **m0005i**.

## Position

`0x89` does not write `actor+0x28/+0x2C/+0x30`. The field actor is
the same `D_8009D254` object the battle tick reloads. Position is
**preserved** unless a later field opcode (`0x0B`) overwrites it.
First request is not followed by an immediate snap.

## HP / stats

Interface only: `0x5A` / actor body. Formula **not** decoded.
Do not invent current-HP addresses.

## EXP / BP / rewards

After the mode==7 poll, m0005i module 6 performs ALU on
`persist[0x0A]`, `persist[0x12]`, and other slots, plus `0xB2`.
That is the **reward/persist interface**. Arithmetic is not the
battle-engine XP table; it is field script. BTL1 must not invent
an EXP formula.

## persist

Battle tick does not memcpy persist. Field script may write persist
before/after the fight. Room change still does not clear the bank
(PST0).

## Script continuation

```text
+0x350C  0x89
+0x3514  wait D_8009D28C == 7
+0x3560  0x40 inhibit
+0x3568  0xAA load bit
+0x3570  0x1C mailbox (2,0,125)
+0x3584  0x20 park
```

Control is **re-inhibited** after return, not restored. A later
`0x3F` on other arms restores pad. First-fight return is not a
free-walk handoff.

## Control restore

Not part of the `0x89` return. Look for a later `0x3F` on the same
map (exists on other m0005i arms; not on this first park).
