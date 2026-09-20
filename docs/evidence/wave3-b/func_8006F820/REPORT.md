# func_8006F820

- **VRAM**: 0x8006F820
- **File offset**: 0x60020 (size 0xCC)
- **Unit**: 5FB9C
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave3-b, agent/wave3-b)

## Behaviour
Two-arena id-record accessor. `a0` selects the arena: 0..0xA uses the
pointer global `D_800942E4` with stride 0xA0C; 0xB..0x15 uses
`D_800942E8` with stride 0x10C; anything larger returns -0xD. The handler
byte at record+1 is clamped to 0x55 and indexes the `D_800942E0` pointer
table; a null entry returns -0xF. Otherwise `a1 == 0` either writes the
record byte (when `a2 < 6`) or `a1 != 0` stores the byte through `a2`; the
record byte is returned either way.

## Method
Read the `glabel`..`endlabel` block in `asm/disc1/5FB9C.s` and the sibling
`src/func_8006F224.c`/`func_8006F2C4.c` for the arena globals. Two layout
levers were needed:
- the `a0 < 0xB` arena split is written with the **large** arena in the
  `if` arm, so cc1 emits retail's `bnez` to the small-arena block;
- the `a1 == 0` test is written **first** so the emitted branch is retail's
  `bnez a1` store layout (writing `a1 != 0` first inverted it).

## Evidence
Fresh complete retail build reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with the registered C leaf count
incremented (855 -> 856). `VERIFY_US=PASS`.

## Divergences
None in the landed source.
