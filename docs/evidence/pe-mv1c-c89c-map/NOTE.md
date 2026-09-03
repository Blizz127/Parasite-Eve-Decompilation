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
- `a2` = `[0x801D0DF8]` — LOADED at the call site (`lui a2,0x801D;
  lw a2,0xDF8(a2)` @80192814; = `0x80162100` on the strict path).
  C89C's first use is `a2 += 0x800` (@8010C8A4): the passed word is
  the table base minus `0x800`.
- `a3`: DEAD on entry — C89C computes `a3 = a2+0x10000`
  (`lui at,1; add a3,a2,at` @8010C8A8) before any read, so the
  80191FB8-exit leftover in `a3` never matters.

## Static state (11 words, plain RAM — `0x8011`, not `0x8012`:
corrected 2026-09-03; the `lui` is `0x8012` but every `addiu` is
negative, e.g. `lui t0,0x8012; addiu t0,t0,-0x1474` = `0x8011EB8C`)

- `0x8011EB8C`: saved `t1`.
- `0x8011EB90..0x8011EBB0`: saved
  `a0/a1/v0/v1/t4/t5/t7/t8/t9` (CBC8 exit writes all 9).
- `0x8011EBB4`: saved bound `t1` (CB8C reads).
- `a0 == 0` entry resumes from saved state (C8B8 defaults:
  8 words from `0x8011EB90`, `t1 *= 2`, `t6 = a1+t1`).

Module-wide scan (authenticated 38-sector carve, `lui`/`addiu`/
move tracking, reset at `jr $ra`) finds exactly 22 accesses to
`[0x8011EB8C,0x8011EBB4]`, all inside C89C except the `lw v0,[EB8C]`
@`0x8010C87C` — which belongs to the `[0x8010C868,0x8010C898)`
scalar-take helper (returns old `[EB8C]`, stores `a0-1` or `-1`).
Sibling helpers sharing the wider state: `[0x8010C82C,0x8010C85C)`
(entry-save of `a0/a1/v0/v1/t4/t5/t7/t8/t9` to `[EB60..EB80]`,
returns 1) and `[0x8010C7E8,0x8010C828)` (`0xFE00`-pad loop to
limit `[EB84]`, COP0 IEc touch, returns 0). Retail reaches the
module by DIRECT `jal`: the verified tail has `jal 0x8010BD4C`
@`801927D8` and `jal 0x8010C89C` @`80192850` (delay `move a0,s1`).
(The earlier "no jals" claim came from the void zero-carve scan
and was already disproved by the tail itself.) The port mirrors
these as direct C calls, matching the transcriptions.

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

## Blockers (RESOLVED 2026-09-03 — both were forensic errors; see
Correction record. Transcription rung unblocked on inputs.)

1. ~~`a1 = lw[s3 + ([146C]^1)*4]`, `s3 = 0` → `a1 = lw[4]`,
   BIOS-resident~~ → WRONG `s3`. The tail sets its own
   `s3 = 0x801D1464` (`lui s3,0x801D; addiu s3,s3,0x1464`
   @`801927C0`, before the E0 loop, which never touches `s3`;
   callee-saved across the `91B64` polls). So
   `a1 = [0x801D1464 + ([0x801D146C]^1)*4]`; first got_frame
   pass toggles `[146C]` 0→1 and loads `a1 = [0x801D1468]`
   (= `[0x801D0DEC]` copy = `0x80132700` on the strict path —
   a movie-buffer pointer, fully modeled in-port).
2. ~~`a2` = 80192CE8's pre-jal setup~~ → WRONG lifetime. The
   call site loads it: `lui a2,0x801D; lw a2,0xDF8(a2)`
   @`80192814` → `a2 = [0x801D0DF8]` (= `0x80162100`,
   BD4C's frame dst, fully modeled in-port).
3. Verification basis (replaces the structural-only refusal):
   real strict-path inputs (`s1`, `[1468]`, `[D0DF8]`, real
   slot stream bytes) + an independent Python model of the
   decoder cross-checked against the C transcription +
   resume-equivalence (fresh-to-bound then `a0 = 0` resume ≡
   uninterrupted run). Documented in the transcription report.

## Tail stop (current)

`got_frame` stops at C89C entry before the `lw a1,[4]`
(which would FATAL in-port). The `[B0DBC]++` and `[146C] = 1`
stores also stay deferred — retail order preserved on reland.

## s1-path forensics (2026-09-03 — SUPERSEDED by the Correction
record below; kept as the error record)

- ~~`s3 = 0` triple-proven~~: the three proofs covered
  80192CE8's `s3` lifetime (zero at `80192CF8`, no sets in the
  scanned spans, `move v0,s3` to `8003EB04`), but 801924F8's
  tail sets its OWN `s3 = 0x801D1464` @`801927C0`. The
  "full-function scan" found "zero sets" (zero-*zeroing*
  insns) and misread it as "`s3` equals zero".
- ~~`a1`/`a2`/`a3` are 80191FB8's exit leftovers~~: true only
  at the `jal 801924F8` @`80192E00` boundary. WITHIN the tail,
  `a2` is reloaded @`80192814`, `a1` @`8019284C`,
  `a0 = s1` in the `jal C89C` delay slot. Only entry-`a3`
  survives as a leftover — and C89C kills it at entry
  (`a3 = a2+0x10000`).
- ~~Bit-bucket hypothesis~~: moot — `a1` never pointed at low
  RAM. The real frame handoff stays `7C394(s1)` (pure record
  math, already transcribed; reads no decoder output).

## Correction record (2026-09-03) — how two parked blockers dissolved

Three compounding forensic errors, all caught by re-carving from
the disc with authentication instead of trusting dumps:

1. **Wrong-carve cascade.** The first overlay carve addressed
   PE.IMG-relative sector `0x3D2` as a RAW disc sector (978
   instead of 1013+978 = 1991), yielding 272384 zero bytes.
   Every scan on it (jal map, pointer map, "97-word tail")
   passed vacuously on zeros. The FNV mismatch against the
   port's loader fingerprint (`0x55EC1574DF7D6A3D`) was the
   tripwire — but it was waved away as "my FNV must differ"
   instead of treated as a failed authentication. Lesson:
   a check with a known-good value that fails is EVIDENCE,
   not tooling noise; oracle anchors (`0x34420200` @`80192D18`
   etc.) confirmed the correct carve.
2. **`s3`-lifetime error.** "Zero sets" (no zeroing insns) was
   misread as "`s3` equals zero". The tail sets
   `s3 = 0x801D1464` two dozen words before the E0 loop.
3. **Address slip.** `lui …,0x8012` + negative `addiu` was read
   as a `0x8012xxxx` address; every low-half addend is negative,
   so the state lives at `0x8011EBxx`. A module-wide `lui`/`addiu`
   resolution pass (authenticated carve) fixes all of them and
   enumerates the sibling helpers.
4. **Boundary confusion.** Leftover analysis at the
   `jal 801924F8` boundary was carried INTO the callee past
   the reloads. Register claims must be re-derived at each
   call site, not inherited across one.

## Level 2: MoveImage (8007512C) — SUPERSEDED (kept as record)

The whole leftover chase is unneeded: the tail reloads `a2`
from RAM @`80192814`, and C89C kills entry-`a3` at entry. The
MoveImage path analysis below is still accurate per-path but
answers a question the goal path never asks.

Retail `MoveImage` (full body read from `asm/disc1/654C8.s`) has a single
exit (`800751DC`) with TWO entry paths:

- **Path B** (branches `beqz`@80075164 / `j`@8007517C, delay `v0 = -1`):
  `a2` UNWRITTEN on this path → inherits the **entry `a2`**, returns `-1`.
- **Path A** (`bnez`@80075174 taken → falls out of `jalr $v0`@800751C4):
  `a2 = 0x14` is set (`addiu`@800751A0) then clobbered by the callee at
  `v0 = [[D_80095744]+8]` → a **second leftover level** (callee-exit `a2`).

Per-call paths for 91FB8's two calls (retail args):

| call | RECT (x,y,w,h) | `[s0+4]`=y | `[s0+6]`=h | path | exit a2 |
|---|---|---|---|---|---|
| 1st (a2=`0x200`) | 320,0,192,256 | 0 | 64 | **B** (`beqz` taken) | entry `a2` = `0x200`, ret `-1` |
| 2nd (a2=`0x100`) | 0,448,320,64 | 448 | 64 | **A** (`bnez` taken) | `[[D_80095744]+8]`-callee-exit `a2` |

In-port `[D_80095744] = 0x80095704` (immutable GPU-table base; seeded the
same in `test_native.c`), so level 2 = whatever function the word at
`0x8009570C` points to at runtime — a third leftover level with
runtime-pointer resolution. **Parked here** (blocker
`mv1c-c89c-unknowable-inputs`): each level costs a full epilogue+path
analysis while `a1 = [arg+4]` (the KUSEG/low-RAM word) still blocks ANY
C89C transcription, so deeper a2 work has zero payoff until the `[4]`
scope decision lands.
