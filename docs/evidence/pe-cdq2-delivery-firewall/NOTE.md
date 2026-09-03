# CDQ2 analysis — the delivery firewall behind the movie path (2026-09-03)

## Question

What populates streaming slots (state 2) and drives the lane
word to -1, so that `func_801924F8`'s E0 poll (`91B64 != 0 →
got_frame`) and give-up wait (`7F72C() == -1`) terminate?

## Proven this turn

- `func_8007C564` (583 words, `asm/disc1/6CD60.s:18-642`) fully
  mapped. ~10 segments, all exits write a status (1/3/4/5/6/7/
  8/9/10) to `[D_8009B374]`. Calls: 7A488 (head gate, ==5 early
  exit), 7CE80 (pure 8-word copy, in-file), 7CEAC (HARDWARE:
  0x1F801080/88 DMA spins + 71A74 print, in-file), 7C444
  (translated clear helper), 7C214 (translated, ONE conditional
  path at CE68 — needs `[C0DB8] != 0` and `[B89F4] != 0`; 7C564
  itself sets `[B89F4] = 1` on the CD9C path but then exits via
  CD78/CE14 without calling). **No lane-word (B574) write, no
  -1, no slot-state publish anywhere in 7C564.**
- Interrupt handlers `func_8007F7E8` / `func_8007F88C`
  (`asm/disc1/6FFE8.s`): lane read-only (7FBF0), descriptor
  writes, callback chains. **No lane writes.**
- `func_8007C214`: translated, publishes state 2, **no lane
  writes, never invoked in-port** (8010C0D8 registers only;
  81314 installs without pumping).
- Whole-EXE scan for absolute-addressed stores to
  `[0x8009B560..0x8009B5A0]`: exactly ONE site (`sw` to B570).
  All lane writes are `$gp`-relative or pointer-based.
  `$gp` is BIOS-provided (no `lui $gp` in the EXE), so static
  gp-relative mapping needs its own setup.

## Conclusion (firewall, unchanged from MV1b)

The -1 producer is still UNKNOWN. Transcribing 7C564 alone
does not unblock E0 (it neither writes -1 nor reliably invokes
7C214). The next rung must FIRST find the -1 writer
(gp-relative EXE analysis once `$gp` is known, or overlay-blob
scan with absolute addressing), THEN design the pump (7C214
invocation cadence during E0 — the interrupt-surrogate pattern
already established by 7ED58's synchronous reset), and only
then reland BD4C → C89C.

## Byproducts (no source changes this turn)

- `/tmp/mv1b_dis.py`: capstone MIPS disassembler for dump blobs.
- `/tmp/scan_st{,2}.py`, `/tmp/scan_gp.py`: EXE store scans
  (documented `%hi`-adjustment and `$gp` gotchas above).
