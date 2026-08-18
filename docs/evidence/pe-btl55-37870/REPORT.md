# PE-BTL55 — func_80037870 message updater named cut

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
37870       1064w 0x80037870..0x80038910  sha 2b0eeaf2…9d4d
jal         3F3C4 @ 0x8003F568
```

Matching `src/` C was not added. Glyph SPRT emission is not this
cut.

## Gate

`B0CD8&0x100` skips to `3F5F4`. `B0CD8&0x200` skips 37870 and
68E24. `0xAA` sets `0x2000`, not `0x200`. Live path reaches
37870.

## State machine (TXT0)

```text
state 1: CE90 → +0x04; scan (FF|F9) FE <id>; cursor = marker+3
parse until yield
F9 → state 0
FF + state!=2 → stop; 388AC promotes 1→2
FF + state==2 + (D1F4&0x100) + !(flags&0x02000000) → 0
```

Live type-0 `0x0D` id 7 then `0x22`. Battle chunk2 has
`FF FE 07` at +171947, terminator `FF` (not F9). Close needs
the newly-pressed `D1F4&0x100` edge from `3EB04`. Do not invent
that edge. Do not invent type-5 pad-hit `D1F4`.

Host scan refuses non-KSEG `CE90` (ROM would walk address 0).
Scan cap `0x10000` is an APPROXIMATION.
