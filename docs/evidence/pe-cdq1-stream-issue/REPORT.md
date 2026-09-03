# CDQ1 — CdlReadS queue issue rung: evidence

## Claim
`func_8007F0C8` (CdlReadS queue issue, ~130 words) + small fry
(`7E6B0`, `80950`, `7C214`, `7C394`) + full `func_80081314` are
translated retail logic. The boot frontier moves from the 81314 cut to
the F394 completion selector with all queue/sequence/lane state real.

## Retail sources (all read, none invented)
- `asm/disc1/6F684.s`: `func_8007F0C8` (full body incl. jump table),
  `func_8007E6B0` (21 words, ring allocator over `D_800A3608`).
- `asm/disc1/71150.s`: `func_80080950` (18 words, copy-or-clear),
  full `func_80081314` (53 words, both arms).
- `asm/disc1/6C93C.s`: `func_8007C214` (35 words), `func_8007C394`
  (19 words, magic-divide record math).
- `asm/disc1/7018C.s`: `func_8007FB44` (read, NOT translated — see why).
- Disc image `rom/image/…Disc 1….bin`: bytes at EXE+0x71BE8 decoded
  `func_800813E8` = 8-word tailcall wrapper around `func_8007C564`
  (proves the completion callback's identity; 7C564 itself unread this rung).

## Machine-checked facts (not eyeballed)
1. Jump-table arms `.L8007F1AC` / `.L8007F2A4` are instruction-identical
   past labels (normalized diff, only arm A's trailing `j F394`
   differs) — one transcription serves all 25 counts.
2. `asm` file offsets are EXE-header-inclusive: prologue `27BDFFD8`
   found at post-header `0x71314`, not `0x71B14` (verified word read).
3. Magic-divide indices verified in Python before pinning in tests:
   `diff=4000 -> 1`, `diff=4064 -> 2`.
4. MIPS `mult` signedness: constant `0x82082083` is negative as int32;
   the port casts explicitly (a caught-and-fixed transcription bug).

## Proven-dead / proven-zero arms (with provenance)
- 7C214's `jalr D_800B0CC8`: dead because `func_8007C304` (only writer
  in the translated tree) is called with callback 0 by the movie
  prefix (translated code). Nonzero arm is an honest boundary.
- 7F0C8 gate word (slot+8): slot3+8 is cleared and never stored
  (proven by write census); other slots are retail spill area,
  zero-canonicalized. The 80950 arm is transcribed but cannot fire.
- `s7 = -1`: 7F0C8's fifth arg is 81314's own `sp+0x10` word, which
  81314 sets to -1 on both arms (proven frame layout).

## Deliberate non-translations (named, with resume points)
- F394 tail `7E8F4 -> 7FB44 -> 7FCFC -> 7B558` controller dispatch:
  CUT at the selector (conditions checked for real). Lanes stay idle,
  completion pending = retail mid-stream. 7B558 is a 259-line BIOS
  executor (next rung starts here: collapse-or-translate call).
- `7C564` 583-line delivery state machine behind `813E8`: the next rung.
- Movie suffix (disassembled live via capstone from guest RAM dump,
  97 insns, loop structure mapped) needs `870F0` (-> missing `7A88C`),
  overlay `8010BD4C`/`80191B64`/`8010C89C`, `7C394` (done this rung).

## Verification
- 8 new `CDQ1_` tests, all passing.
- Migrated: 4 test sites (B54KAD/AE/AH/AM) + 11 `.py` oracle pins;
  `b54kan_cdlreads_delivery_audit.py` and
  `b54kap_first_form2_sector_oracle.py` re-run green.
- Full suite WITH local Disc 1 image: **1036 run, 1036 passed**.
  Disc-free: 1018 passed + gated B54KY + 17 skipped (unchanged).
- Real-disc run: 483 presents, then exactly one boundary
  (`func_8007F0C8_completion_selector`); strict agrees.
