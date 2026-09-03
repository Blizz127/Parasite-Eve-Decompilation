# PE-CDS1 — CD completion-selector tail translated; stop moves to func_8007FCFC

Authority: retail Disc 1 executable `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; disassembly
`asm/disc1/6F684.s:340-414` (7F0C8 tail), `asm/disc1/6E6C0.s:855`
(7E8F4), `asm/disc1/7018C.s:144` (7FB44), `asm/disc1/704BC.s:34`
(7FCFC, read but NOT translated).

## Retail facts (re-verified word by word)

- 7F0C8 tail `0x8007F394..0x8007F3EC` (22 words): `FBF0(0)`;
  `bne → 3EC` falls through with `addu $v0,$s5,$zero` in the delay
  slot (`0x8007F3A4 = 0x02A01021`); head `*24` via `sll 1 / addu /
  sll 3`; `bne rec[0],seq → 3EC` also falls through with
  `addu $v0,$s5,$zero` (`0x8007F3D4`); passing arm `jal 7E8F4`
  (result discarded) then `j 3EC` with `v0 = seq`. **Both
  non-passing arms return the sequence, not 0** — the old port
  returned 0 on both.
- `func_8007E8F4` (28 words, `0x8007E8F4..0x8007E964`): `FBF0(0) !=
  1 → 0`; `rec = 0x800A3540 + 24*head` (same sll/addu/sll shape);
  `rec[0] == 0 → 0`; else `7FB44(rec.byte4, rec.word+0xC) != 0`
  (`sltu $v0,$zero,$v0` = `0x0002102B`).
- `func_8007FB44` (31 words, `0x8007FB44..0x8007FBC0`):
  `[0x8009B598] > 0 → 0` (**signed** `bgtz`);
  `[0x8009B554] == 0 → 0`; `[0x8009B574] != 1 → 0`; else latch
  `0x1F @ 0x8009B570`, `2 @ 0x8009B574`, `0xB @ 0x8009B578` (the 0xB
  store is the `jal` delay slot, ahead of the call), then
  `7FCFC(cmd & 0xFF, data, 0x8009B598, 0x8009B554)` and return its
  value. (7FCFC ignores incoming a2/a3 — it zeroes a2 itself.)
- `func_8007FCFC` (74 words) calls `7B9EC`, `80950`, and `7B558`
  over drive-state bytes with multi-branch control flow: NOT plain
  bookkeeping, so per the rung rule it becomes the new named stop
  instead of being translated. `7B558`/`7C564` out of scope.

## Port changes (`pc_port/platform/pe_libcd.c`, `pe_sdk.h`)

- 7F0C8 tail: both arms return `seq`; passing arm calls the real
  `func_8007E8F4()` (discarded) and returns `seq`. The old
  `func_8007F0C8_completion_selector` boundary is gone (deleted, not
  moved: the tail is now fully real).
- New `func_8007E8F4()` / `func_8007FB44(cmd, data)` exactly as
  above; 7FB44's passing arm records `func_8007FCFC` from
  `func_8007FB44` + `PE_PORT_STOP_UNRESOLVED_BOUNDARY` and returns 0
  (never consumed past the stop). Lane state untouched at the new
  boundary.
- Incidental production fix (strict-run crash): `func_8006AD40`
  indexed the guest DISPENV pair by `[0x800ACDDC]`, a word that is
  wild at boot (`0xAC33E2F8` → `FATAL` in `PE_LoadU16`). Retail
  (`asm/disc1/5B1E4.s:633-635`) indexes by `D_8009CDDC` (`*20` via
  sll/addu/sll); the port now does the same. The old host-pointer
  form read the wild index silently.

## Oracle

`pc_port/tools/pe_cds1_selector_tail_oracle.py`: the three windows
(22/28/31 words) by SHA-256, the full jal chain
(FBF0 → 7E8F4 → 7FB44 → 7FCFC), both `addu seq` delay slots, the
`addu zero` word, the `sltu` tail, both `*24` triples, record
offsets +4/+0xC, and the 0x1F/2/0xB/cmd-mask immediates. Exit 0 =
green; proven to fail (perturbed `0x8007FBF0` → `FAIL: F394 FBF0
call`, exit 1) and restored. Note: asm comments print bytes
MSB-first; the oracle pins true LE words (e.g. `0x02A01021`, not
`2110A002`).

## Tests

6 new `CDS1_*`: seq returns on both non-passing arms (no stop, no
dispatch, no latch); 7E8F4 null-head 0 without stop; live head
dispatches (latches + 7FCFC boundary; the stop truncates 7E8F4's
value to 0, documented in-test); all three 7FB44 early arms;
signed-`bgtz` pin (`0x80000000` flag still dispatches). Migrated
with intent intact: both CDQ1 boundary tests, B54KAD/AE, and the
B54KY real-disc test now expect `func_8007FCFC` (+ the live
0x1F/2/0xB latches where asserted).

## Strict run

`parasite-eve-port --headless --strict-stubs --disc-image <Disc 1>`
now travels the whole translated boot and stops honestly at the
first unresolved provider:
`FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
func_8007FCFC / called from: func_8007FB44`.
Zero `HOST_ADAPTED` lines on that path.

## src/ attempts (parked, G6)

`func_8007E8F4` (2 phrasings: scalar externs, unknown-size arrays)
and `func_8007FB44` (2 phrasings: negative-index stores, explicit
gate locals + hoisted a3), all era `-O2 -G0` via
`tools/analysis/era_leaf_match.sh`, diverge on shared-`lui`
lifetime/coloring (7E8F4: second base for A3540; 7FB44: fresh `lui`
for the store block). Parked in
`docs/ai_context/parked_blockers.json`
(`shared-lui-base-lifetime-coloring-7e8f4`,
`store-block-base-reuse-coloring-7fb44`). No `src/` or YAML
changes; matching count stays 560. NOT matching leaves.

## Verify

```
Results: 1054 run, 1036 passed, 1 failed, 17 skipped
Results: 1054 run, 1054 passed, 0 failed, 0 skipped
```

First line gateless (1 failure = pre-existing B54KY environmental
case). Second line with `PE_DISC1_BIN`. ASan/UBSan CTest with the
disc env: `100% tests passed, 0 tests failed out of 2`, zero
sanitizer diagnostics. `--bootstrap-disc --max-frames 1
--screenshot` byte-deterministic across runs. `shim_inventory.md`
needs no update (it carries no CD rows — CDQ1 precedent).
