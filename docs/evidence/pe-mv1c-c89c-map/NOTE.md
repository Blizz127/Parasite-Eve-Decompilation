# MV1c-map — func_8010C89C fully mapped, parked on inputs (2026-09-03)

## Shape

Resumable VLC-style block decoder, `[0x8010C89C..0x8010CBF8]`,
~225 words, disassembled from `/tmp/ov_10C89C.bin` (capstone).
ZERO calls in range (the single `jal` at `0x8010CC78` belongs
to whatever follows). Pure RAM + one COP0 touch.

## Arguments (from the s1-path bytes, verified in
`/tmp/pe_801924F8_tail.bin`)

- `a0`: input cursor (frame stream; 0 = resume-from-saved).
- `a1`: output cursor AND arena base: tables at `a1+0x800`
  (`a2`) and `a1+0x10800` (`a3 = a2+0x10000`). Output grows up
  from `a1`, tables sit above — single arena, no collision
  while output stays under 0x800 bytes.
- `a2`/`a3`: NOT set by the tail call — 801924F8's inherited
  registers (80192CE8's pre-jal setup, dump ends before it).
  Contents UNKNOWN (see blocker).

## Static state (11 words, plain RAM)

- `0x8012EB8C`: saved `t1`.
- `0x8012EB90..0x8012EBB0`: saved
  `a0/a1/v0/v1/t4/t5/t7/t8/t9` (CBC8 exit writes all 9).
- `0x8012EBB4`: saved bound `t1` (CB8C reads).
- `a0 == 0` entry resumes from saved state (C8B8 defaults:
  8 words from `0x8012EB90`, `t1 *= 2`, `t6 = a1+t1`).

## Control flow

- Main loop at `CA7C`: bit-buffer (`v0`/`v1`) VLC decode from
  `a0`, table lookups through `a2`/`a3`, halfword output to
  `(a1)`, `a1 += 2`.
- `0x7C1F` value → write + continue (NOT an exit).
- `0xFE00` value → back to `C960` (inner restart).
- Bound exit `CA74` (`a1 >= t6`) → `CBC8`: save 9 state
  words, return 1.
- `CB84` (taken when `(v0>>22... ^ 0x3FF) == 0` at `C96C`) →
  pad output with `0xFE00` up to saved bound, touch COP0,
  return 0.
- Caller ignores the return (falls into `7C394`).

## COP0 touch (CBAC–CBBC, skip with comment)

`mfc0 t1,$4; or t1,2; mtc0 t1,$4` — sets Status bit 1 (IEc,
interrupt enable). No port meaning (synchronous delivery);
conceptually re-arms the delivery the pump models.

## Blockers (why not transcribed now)

1. `a1 = lw[s3 + ([146C]^1)*4]`, `s3 = 0` (80192CE8 zeroes;
   801924F8 full-function scan: one spill, zero sets),
   `[146C] = 0` (prefix zeroes; sole writer) → `a1 = lw[4]`,
   a BIOS-resident vector word. No game code writes RAM[4]
   (zero absolute low-RAM stores in EXE + all overlay dumps).
   Unknowable without a BIOS/low-RAM model.
2. `a2` (table base) = 80192CE8's pre-jal setup — the
   `/tmp/fn_192CE8.bin` dump ends at `0x80192DFC`, before the
   `jal 801924F8`. Needs a bigger dump to resolve.
3. Verification would be structural-only (synthetic arena);
   no output oracle exists. Per the BB0 lesson, parked until
   (1) and (2) resolve.

## Tail stop (current)

`got_frame` stops at C89C entry before the `lw a1,[4]`
(which would FATAL in-port). The `[B0DBC]++` and `[146C] = 1`
stores also stay deferred — retail order preserved on reland.

## s1-path forensics (2026-09-03, TEMP dump probe reverted)

- `s3 = 0` triple-proven: 80192CE8 zeroes (`move s3,zero`
  at `80192CF8`); 801924F8 full-function scan (prefix dump +
  97-word tail: one spill, zero sets); post-jal
  `80192E1C move v0,s3` passes 0 to `8003EB04`.
- `jal 801924F8` at `80192E00` (delay `sra a0,a0,16`;
  `a0 = index`): `a1`/`a2`/`a3` are 80191FB8's exit leftovers
  (nothing reloads them between the jals) — C89C's `a2`
  table base is 80191FB8's last-`a2` state (needs its
  epilogue dump to resolve; TEMP probe dumped only 192CE8).
- `a1 = lw[4]` therefore stands on the goal path with TWO
  unknowables (`a1` BIOS word, `a2` caller leftover).
- Bit-bucket hypothesis (unproven): KUSEG low RAM is never
  read by hardware post-boot (no MMU/TLB use in game mode),
  so C89C's output writes there may be benign, with the real
  frame handoff via `7C394(s1)` + the EB90 state saves + COP0
  IEc. Would need: C89C transcription + EB90-save pins +
  proof the display path never reads the a1 output. NOT
  attempted — structural-only verification per the BB0
  lesson.
