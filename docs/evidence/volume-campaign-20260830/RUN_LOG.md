# Matching-C volume campaign run log — 2026-08-30

Base: `634dd1b`, 335 exact matching-C leaves.

Pool: 1,088 current active spans — Tier 1 1, Tier 2 214, Tier 3 99,
SKIP/suppressed 774. Refresh evidence: `POOL_REFRESH.md`.

Consecutive bounded parks: 3. Matched leaves this campaign: 6.
Hard stop: **H5 fired after attempt 13**; no Tier-1 row was skipped to avoid it.

Close-out reset: a new explicit authorization resumes only the three remaining
Tier-1 rows after the historical H5. Current close-out streak: zero bounded
parks, one match, one proven-family suppression.

| attempt | target | function hood / screens | outcome | words | phrasings | commit or stash |
|---:|---|---|---|---:|---:|---|
| 1 | `func_80078C34` @ `0x69434` | canonical return; 25 direct callers; real boundaries; handwritten COP2 body | `SKIP-SDK-LIBRARY-COP2`; no C attempt or integration | 23 | 0 | docs-only commit |
| 2 | `func_8003C5D8` @ `0x2CDD8` | canonical return; 26 direct callers; real boundaries; no callees/globals/loop | MATCHED leaf 336; signed-short formal recovers entry copy; 24/24 object, packed span, full SHA exact | 24 | 2 | VOLUME-97 leaf commit |
| 3 | `func_80070D6C` @ `0x6156C` | canonical return; four direct callers; real boundaries; no frame/callees/loop; Stage-0 state census closed | `PARKED-REGISTER-HOME-AND-CROSS-BLOCK-SCHEDULING`; both natural shapes byte-identical at 24 versus retail 25 words | 25 | 2 | stash `park func_80070D6C register-schedule residual` |
| 4 | `func_8006346C` @ `0x53C6C` | canonical return; four direct callers; real boundaries; no frame/callees/globals/loop | MATCHED leaf 337; delayed default-result initialization frees `a2` for `mflo` and fills the late guard slot; 26/26 normalized object, packed span, full SHA exact | 26 | 2 | VOLUME-99 leaf commit |
| 5 | `func_8003F798` @ `0x2FF98` | canonical return; three direct callers; real boundaries; split marks handwritten; five `ctc2` effects | `SKIP-HANDWRITTEN-COP2`; no C attempt or integration | 26 | 0 | VOLUME-100 docs-only commit |
| 6 | `func_80012700` @ `0x2F00` | canonical return with live result slot; seven direct callers; real boundaries; no frame/callees/loop; Stage-0 freelist and serial census closed | MATCHED leaf 338; direct postincrement assignment preserves old/new serial homes and store order; 29/29 normalized object, packed span, full SHA exact | 29 | 2 | VOLUME-101 leaf commit |
| 7 | `func_80079304` @ `0x69B04` | canonical return with live result slot; two direct callers; real handwritten neighbors plus explicit two-nop alignment gaps; split marks handwritten COP2 operation | `SKIP-SDK-LIBRARY-COP2`; no C attempt or integration | 30 | 0 | VOLUME-102 docs-only commit |
| 8 | `func_8003DF50` @ `0x2E750` | canonical return with live store slot; one direct caller; immediate real boundaries; no frame/callees/globals/loop | MATCHED leaf 339 first phrasing; signed-short 16-byte typed record plus alias-sensitive repeated source expressions; 30/30 object, packed span, full SHA exact | 30 | 1 | VOLUME-103 leaf commit |
| 9 | `func_80078554` @ `0x68D54` | canonical return; two direct callers; preceding real return delay slot and one-nop gap before next real function; split marks handwritten COP2 operation | `SKIP-SDK-LIBRARY-COP2`; no C attempt or integration | 31 | 0 | VOLUME-104 docs-only commit |
| 10 | `func_800CE870` @ `0xBF070` | canonical return; 27 direct callers; immediate real boundaries; no frame/callees/globals/loop | MATCHED leaf 340 on phrasing 2; signed 16.16 high-half source fields recover three retail `lh` operations; 32/32 normalized object, packed span, full SHA exact | 32 | 2 | VOLUME-105 leaf commit |
| 11 | `func_800339A0` @ `0x241A0` | canonical return; six direct callers; immediate real boundaries; no callees/loop; sole writer of three Stage-0-censused gp fields | `PARKED-GP-LOAD-STORE-SCHEDULING-AND-REGISTER-HOME`; two source store orders compile byte-identically, retaining selected address in `v0` and hoisting the second `lhu` versus retail `v1` home and interleaved store | 32 | 2 | stash `park func_800339A0 gp-load-store scheduling residual` |
| 12 | `func_80080C48` @ `0x71448` | canonical return with live subtract; six direct callers; immediate real boundaries; no frame/callees/globals/loop; independent native `CdPosToInt` vectors | `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING`; both candidates are 32 words, but cc1 hoists byte 2 to entry and changes the BCD accumulator homes/schedule | 32 | 2 | stash `park func_80080C48 independent-load scheduling residual` |
| 13 | `func_800762BC` @ `0x66ABC` | canonical return with live frame teardown; four direct callers; immediate real boundaries; no callees/globals/loop; independently identified libGPU E2 texture-window packer | `PARKED-TEXTURE-WINDOW-CONTROL-FLOW-LOAD-SCHEDULE-AND-COLORING`; array retry recovers frame/stores/size, but null layout, X/W versus Y load order, and register homes differ; H5 stop | 32 | 2 | stash `park func_800762BC texture-window layout residual` |
| 14 / close-out 1 | `func_8007AA34` @ `0x6B234` | canonical return with live subtract; one direct caller; immediate real boundaries; full 32-word body byte-identical to `func_80080C48` | `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING-FAMILY`; exact duplicate of an exhausted residual, screened without redundant compilation | 32 | 0 | stash `park func_8007AA34 duplicate CdPosToInt residual` |
| 15 / close-out 2 | `func_80021850` @ `0x12050` | canonical return; three direct callers; immediate real boundaries; no callees/globals/loop; two argument-derived record addresses | MATCHED leaf 341 first phrasing; signed-byte indices and typed 12-byte aggregate recover the 16-byte stack temporary and exact copy schedule; packed span/full SHA exact | 34 | 1 | VOLUME-110 leaf commit |
