# func_800C6F4C

- **VRAM**: 0x800C6F4C
- **File offset**: 0xB774C (size 0x54)
- **Unit**: B76F8
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave4-a, agent/wave4-a)

## Behaviour
Reverse of func_800C6EF8: copies `*(u16 *)(mesh + 0xA)` 32-bit words from the
palette scratch `D_800E2370` into `mesh + *(u16 *)(mesh + 8)`. No return
value.

## Method
Read `glabel`..`endlabel` in `asm/disc1/B76F8.s` and the port spec in
`pc_port/game/boot/func_800C71E4_port.c`. Uses the same two levers as
func_800C6EF8 (dead 8-byte `tmp[2]` for retail's phantom frame; loop counter
pinned to `$a1`). One extra residual: the loaded word must live in an `int`
temp — typing the word temp `unsigned short` makes cc1 emit `lhu`, retail is
`lw`.

## Evidence
Fresh complete retail build reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with the registered C leaf count
incremented (864 -> 867 across this wave). `VERIFY_US=PASS`.

## Divergences
None in the landed source. The `tmp[2]` local is a frame-artifact reproducer.
