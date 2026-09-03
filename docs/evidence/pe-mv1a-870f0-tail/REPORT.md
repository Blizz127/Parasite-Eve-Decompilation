# PE-MV1a — func_800870F0 translated; 801924F8 tail extended to the 7A88C frontier

## What moved

`func_800870F0` (42 words, `0x800870F0..0x80087198`) is transcribed in
`pc_port/game/boot/func_800870F0_port.c`: branch on
`[D_8009D2C0] & 2`, the sll/addu/subu/srl scale chain as one
32-bit `((2903 * a0) >> 13) & 0xFF`, four `sb` stores per arm to
`D_8009D1C8..D_8009D1CB`, tail call to `func_8007A88C` with
`&D_8009D1C8` (return ignored — void, matching retail's bare
`jr $ra`). The callee is a named stop (no asm in tree).

`func_80191B64` (slot-table poll worker) is transcribed in
`pc_port/game/boot/func_80191B64_port.c` from live dumps: 2000-try
countdown over `func_8007C484`, 719E4-then-7C394 accept path,
fill_rect compare + real `func_80074F44` (ClearImage, pre-existing)
on the taken-unequal arm, 2000-word copy + re-feed on the
slot-2 fast path. Stack-temporary cells live at `0x801FFF20+`
(scratch map verified free).

`func_801924F8`'s tail (`0x801927B0..0x80192933`) is transcribed in
place: the `cdready_wait` label (B0-exhaustion re-polls), 870F0
issue, BD4C decode as a named stop (MV1b), the E0 2000-try poll
loop over 91B64 with the give-up re-issue path (copy D0DC4→D0DDC,
`-1`-gated waits, Setloc + ReadN(480), `s0 = 2000` on both beqz
outcomes), and the got_frame entry-stop at `func_8010C89C` (the
`s3 = 0` sole-caller value makes its `a1` a KUSEG vector-range
load the port cannot execute; the EC post-decode half resumes in
MV1b). The old `func_801924F8_801927B0_cut` is deleted.

## Corrections during the rung

- 74F44 needed no new file: it is the real ClearImage wrapper in
  `psx_compat.h` (duplicate deleted; 91B64 builds a host RECT).
- 870F0 never touches `[801D146C]` (disassembly overruled a false
  memory of an increment there); two test clauses asserting
  `[146C] == 1` were reverted to `0`.

## New frontier

Strict, real Disc 1, canonical CDS1 command
(`parasite-eve-port --headless --strict-stubs --disc-image <Disc 1>`)
now travels the whole translated boot and stops honestly at the
first unresolved provider:
`FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
func_8007A88C / called from: func_800870F0`.
In-process, B54KAD/AE now observe `func_8007A88C == 1` with
`7B9EC == 0` and the 870F0 stores live. Zero `HOST_ADAPTED` lines.

## Test migrations (intent intact, nothing relaxed)

B54KAD/B54KAE expect the new boundary (`func_8007A88C`, sole
caller 870F0, so count 1 pins execution). No other test moves.

## Verify

```
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
```

Both with `PE_DISC1_BIN` (absolute path; gateless run leaves only
the B54KY env case). Normal CTest with disc: `100% tests passed, 0
tests failed out of 2`. Fresh ASan/UBSan CTest with disc: `100%
tests passed, 0 tests failed out of 2`, zero sanitizer
diagnostics. New oracle `pc_port/tools/pe_mv1a_870f0_oracle.py`
green (870F0 window SHA, predicate, chain words, stores, 7A88C
call, scale math x256). Leaf count 560; no src/YAML changes.
