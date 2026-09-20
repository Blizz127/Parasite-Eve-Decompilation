# func_80042170

- **VRAM**: 0x80042170
- **File offset**: 0x32970 (size 0xB8)
- **Unit**: 307CC
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave4-a, agent/wave4-a)

## Behaviour
Memory-card save LOAD entry. `card` selects a 0x418-byte record at
`D_800A0ED4`; if record byte 0 is not 1 the function returns 0. Otherwise it
calls func_80042798, re-arms the record (0x14 = 0x2000, 0x16 = 0x0A, byte 7 =
2, byte 1 = 1, byte 0xB = 5), points record+0x18 at the 0x8009EED0 save block,
stores the slot byte at record+3, publishes the record to D_800A1854 and its
size to D_800A1858, then calls func_80071A24(D_8009EED0, 0x2000). Returns 0.

## Method
Read `glabel`..`endlabel` in `asm/disc1/307CC.s` and the port spec in
`pc_port/game/boot/func_80042020_port.c`. The `card * 0x418` stride
materialises as `((card<<5)+card)<<2 - card)<<3` automatically. One layout
lever was required: the `*(unsigned int *)(s0 + 0x18) = D_8009EED0` store
must be the **first** store after func_80042798(). cc1 then hoists the
`la $a0,D_8009EED0` to immediately after the call and keeps it live across
the five following stores until the closing call, matching retail; writing it
later sinks the `la` and shifts nine words.

## Evidence
Fresh complete retail build reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with the registered C leaf count
incremented (864 -> 867 across this wave). `VERIFY_US=PASS`.

## Divergences
None in the landed source.
