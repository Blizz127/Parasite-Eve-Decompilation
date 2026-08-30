# PE-B54K-N — BIOS `FlushCache` host adapter

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This small platform rung resolves `func_800726C4`, the retail BIOS A0(44h)
`FlushCache` veneer. It was exposed while auditing the caller continuation
that will follow a future B54K-M completion of `func_8006AD40`.

## Retail proof

The exact body at file `0x62EC4`, bounded by zero alignment words, is:

```text
0x800726C0  00000000  prior padding
0x800726C4  240A00A0  addiu t2,zero,0xA0
0x800726C8  01400008  jr    t2
0x800726CC  24090044   addiu t1,zero,0x44
0x800726D0  00000000  following padding
```

This is a BIOS A-table dispatch, with call number `0x44` in the jump delay
slot. The project already identifies this exact BIOS contract as
`FlushCache` in its GTE platform audit. The exact executable has 14 direct
`jal 0x800726C4` sites:

```text
80014EE0 80014F50 8006B78C 8006E938 8006F010 8006F148 8006F1F8
8006F460 8007A1BC 8007E280 8007E2E8 8007E454 8007E4C4 8007E560
```

The canonical post-`func_8006AD40` path in `func_8001220C` loads
`D_8009D280`, publishes it to `D_8009D1C4`, and for the initialized
`0xA9400048` value branches to `func_8006E834`. Retail 6E834 executes:

```text
0x8006E930  jal func_80072714  EnterCriticalSection
0x8006E938  jal func_800726C4  FlushCache
0x8006E940  jal func_80072724  ExitCriticalSection
```

After 6E834 returns, the next canonical main-loop call is the still-unresolved
`func_801909B4` at `0x800123B4`. That is a static/direct-test prediction for
the future completed caller path; production still stops earlier at B54K-L
until B54K-M is implemented and the two host DMA checkpoints are measured.

## Native contract

The host adapter is intentionally empty. Native code is not emitted into an
emulated R3000 instruction cache, so there is no guest cache authority to
flush and no game state to synthesize. The implementation lives with the
other generic libetc/BIOS adapters in `pe_libetc.c`; the former local
`BOOTSTRAP_RET` definition in `func_8006E834_port.c` is removed.

Two focused tests prove:

- direct strict execution returns without a bootstrap record, guest write,
  or critical-section-depth change;
- strict `func_8006E834` crosses the call and leaves its surrounding
  Enter/Exit pair balanced.

The independent oracle authenticates the retail executable, all three body
words and both boundaries, all 14 callers, the main-loop branch, and the
6E834 critical-section sequence.

```text
B54K-N FlushCache oracle: PASS (retail identity and caller path).
Results: 954 run, 954 passed, 0 failed, 0 skipped
fresh ASan/UBSan: 954 run, 954 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

The exact retail executable SHA-1 remains
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. No scheduler token,
persistence state, or field route was added.

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_80093126_archive_cut
FUNC_800726C4=NATIVE_PLATFORM_PROVIDER
FUTURE_POST_B54KM_CANONICAL_BOUNDARY=func_801909B4_pending_runtime_measurement
```
