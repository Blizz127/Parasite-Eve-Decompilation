# PE-B54K-M — `func_8006AD40` completion readiness audit

Status: **HISTORICAL READINESS AUDIT; IMPLEMENTED BY PE-B54K-M**.

The dedicated completion rung has now landed.  Current implementation and
verification authority is
`docs/evidence/pe-b54km-6ad40-complete/REPORT.md`; the trial and migration
matrix below are retained as the pre-implementation record.

This evidence-only rung closes the retail and dependency audit for the final
suffix of `func_8006AD40`. A temporary native translation compiled and reached
the normal return, but it was reverted after the full suite exposed 23 test
contracts that deliberately describe the current B54K-L prefix boundary.
Production remains at B54K-L and `952/952`; this report is the migration plan
for a dedicated completion rung, not permission to erase those assertions.

## Exact retail identity

```text
executable SHA-1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
function range      [0x8006AD40,0x8006B35C)
function size       0x61C bytes / 391 words
function SHA-256    36c7674380e42a7cb5c13f3802689b936bf6e48b67994112c510abbeaaef5539
audited suffix      [0x8006B220,0x8006B35C)
suffix file range   [0x0005BA20,0x0005BB5C)
suffix size         0x13C bytes / 79 words
suffix SHA-256      1aed4d0f3bcaa751da044557dccd15e0f15a002addd7f0333ecaa635885749d8
last included       0x8006B358  nop (jr-ra delay slot)
next function       0x8006B35C  addiu sp,sp,-0x18
```

`pc_port/tools/b54km_6ad40_completion_readiness.py` imports no production
code. It authenticates the executable, compares all 79 suffix words, verifies
the full-function and suffix hashes, and checks the archive walk, all calls,
display-environment arithmetic, state stores, branches, epilogue, and next
boundary.

The suffix has these coherent owners:

```text
0x8006B220..0x8006B270  walk the completed +0x188 archive
0x8006B274..0x8006B2BC  stream F1 and graphics/display synchronization
0x8006B2C0..0x8006B32C  explicit state reset and D_800B0CD8 bit-0 clear
0x8006B330..0x8006B358  normal epilogue and return
```

The `+0x188` walk uses the established packed archive header, a zero-count
bypass, and a `0x14`-byte entry stride. Each entry calls
`func_8006E1C0(entry, base)`. Display environment selection is exactly
`D_800BCE80 + D_800ACDDC * 20`. The final state logic performs the retail
widths and ordering, conditionally resets fields selected by flag bits `0x40`
and `0x80`, and ends with `D_800B0CD8 &= ~1` before a conventional
`jr ra; nop` return.

## Dependency readiness

Every direct suffix callee now has an implemented authority:

| Call PC | Retail target | Current authority |
| --- | --- | --- |
| `0x8006B254` | `func_8006E1C0` | translated retail archive-entry helper |
| `0x8006B274` | `func_80087024` | native stream command `0xF1` provider |
| `0x8006B27C` | `func_80074DC0` | host `DrawSync` shim |
| `0x8006B284` | `func_80074A44` | native `ResetGraph`; argument `1` takes its light path |
| `0x8006B28C` | `func_80073A44` | host `VSync` shim |
| `0x8006B2B4` | `func_800755F0` | host `PutDispEnv` shim |
| `0x8006B2BC` | `func_80074D28` | host `SetDispMask` shim |

The stream command's known guest effects must be part of the future whole-RAM
contract: `D_800BCD80=0xF1`, one ring entry at
`D_800B8628 + D_8009D2F4*0x24`, incremented `D_8009D2F4`, and final
`D_8009D268=0`. The SDK shims are host-observable surfaces; the
`ResetGraph(1)` light path adds no guest-RAM mutation.

## Reverted implementation trial

A temporary implementation of all 79 words compiled successfully and executed
the final return. It was not retained. Against the unchanged historical test
contracts, its first full run was:

```text
Results: 952 run, 929 passed, 23 failed, 0 skipped
```

All 23 failures are attributable to the intentional transition from a named
prefix stop to full-function completion. They are useful migration alarms,
not assertions to delete:

| Contract group | Failing tests | Required replacement |
| --- | ---: | --- |
| Original 6AD40 boundary/finalization | 5 | Prove bit-0 guard, normal return, exact final fields, and absence of the former bootstrap-boundary stop. |
| Retained B54B/B54D counted/material paths | 4 | Preserve their loop and record assertions, then assert completion state instead of the old frontier hit. |
| B54C caller/provider relation | 1 | Measure the completed 6AD40 call and the caller's subsequent provider/checkpoint order. |
| B54E/F/G poll rungs | 6 | Preserve poll/reissue assertions while replacing the D_80093126-cut expectation with completion state. |
| B54KA caller integration | 1 | Re-measure the next strict frontier after the caller's two explicit DMA checkpoints. |
| B54KJ/K/KL retained paths | 6 | Preserve no-replay, empty-archive, transfer, and call-order checks; replace one frontier record with exact suffix effects. |

The future test rewrite must additionally establish:

1. Positive and zero-count `+0x188` archives, with exact
   `func_8006E1C0` arguments and no zero-count walk.
2. A complete `0x40`/`0x80` branch matrix for the conditional state fields.
3. First-call behavior that clears bit 0, followed by a second call that
   guards immediately and performs no additional work.
4. A whole-RAM allowlist containing every explicit suffix store and the
   stream-F1 authority above, while retaining canaries elsewhere.
5. A strict direct-call test proving normal return rather than a boundary
   record.
6. A production-caller test that continues through the two explicit DMA
   checkpoints and records the *next measured* strict frontier. It must not
   assume that full 6AD40 completion also completes its caller.

The historical test names and their failure messages are preserved in the
trial log used for this audit. The migration must rewrite each assertion into
its full-function equivalent; a broad removal or count adjustment would not
be acceptable.

## Readiness oracle output

```text
  OK identities: exact executable, full body, suffix, 79/79 words
  OK +0x188 archive walk: packed header, zero bypass, 0x14 stride
  OK call 0x8006b254: func_8006E1C0 (translated retail helper)
  OK call 0x8006b274: func_80087024 (native stream F1 provider)
  OK call 0x8006b27c: func_80074DC0 (host DrawSync shim)
  OK call 0x8006b284: func_80074A44 (native ResetGraph)
  OK call 0x8006b28c: func_80073A44 (host VSync shim)
  OK call 0x8006b2b4: func_800755F0 (host PutDispEnv shim)
  OK call 0x8006b2bc: func_80074D28 (host SetDispMask shim)
  OK display env: D_800BCE80 + D_800ACDDC*20
  OK state reset: exact widths/order, 0x40/0x80 gates, bit 0 clear
  OK completion geometry: retail epilogue and next-function boundary

B54K-M readiness oracle: PASS (all 7 callees available; no code changed).
```

The earlier whole-function oracle independently remains green, and the
unchanged B54K-L binary was rerun after the trial was reverted:

```text
B54A oracle: PASS (8/8 scenarios; build/disc1.candidate.exe)
Results: 952 run, 952 passed, 0 failed, 0 skipped
git diff --check: PASS
disc1.candidate.exe SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

## Disposition

The final suffix is statically closed and dependency-ready. Its integration
is intentionally deferred to a dedicated B54K-M implementation rung with the
test-contract migration above, a new independent runtime oracle, normal and
fresh ASan/UBSan suites, and a measured production-caller frontier.

No production source or test changed in this audit. At this historical audit
rung, the terminal state was:

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_80093126_archive_cut
SEMANTIC_IMPLEMENTATION=verified_through_0x8006B220
B54K_M_COMPLETION_SUFFIX=AUDITED_NOT_IMPLEMENTED
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```

That state is superseded by
`docs/evidence/pe-b54km-6ad40-complete/REPORT.md`; the completed function now
returns normally and production next stops at `func_801909B4`.
