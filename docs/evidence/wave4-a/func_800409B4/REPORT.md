# func_800409B4

- **VRAM**: 0x800409B4
- **File offset**: 0x311B4 (size 0x1CC)
- **Unit**: 307CC
- **Build profile**: era_o2_g0_three_word (`-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave4-a, agent/wave4-a)

## Behaviour
Card-subsystem boot init. Guards on `D_800A1850`; on first call sets it,
calls `func_80072714`, opens the eight card events with `func_800726E4`
(class `0xF4000001` for events 0-3, `0xF0000011` for 4-7; specs
4/0x8000/0x100/0x2000; mode 0x1000; handlers `func_80042BD8`..`func_80042C64`),
storing each handle at `D_800BCDA8[0..7]`. Then runs the four card bring-up
calls, enables each handle with `func_80072704`, and calls `func_80072724`.
Finally clears `D_800A0ED4[0x418]` and `D_800A0ED4[0]` via a descending
0x418-stride loop.

## Method
Read `glabel`..`endlabel` in `asm/disc1/307CC.s` and the loop spec in
`pc_port/platform/pe_libcard.c` (`kCardEvents`). The eight registrations are
explicit unrolled source statements (retail carries the masks as immediates
and the callbacks as `lui/addiu` of distinct symbols, not table loads). Two
levers:

- **era_o2_g0_three_word** (`MASPSX_THREE_WORD_SYMBOL_STORE=1`): the trailing
  `D_800A0ED4[j]` store needs retail's
  `lui $at,%hi; addu $at,$at,$v0; sb $zero,%lo($at)` form.
- the clear loop needs its **own index variable**. Reusing the enable-loop
  counter keeps that pseudo in callee-saved `$s1` across all the calls, so cc1
  also colours the clear index `$s1`; retail uses the call-clobbered `$v0`.
  A second `int j` gives `$v0` and matches.

## Evidence
Fresh complete retail build reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with the registered C leaf count
incremented (867 -> 868). `VERIFY_US=PASS`.

## Divergences
None in the landed source.
