# PE-B54K-M — complete `func_8006AD40`

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

> **B54K-S supersession note (2026-08-30):** B54K-S later translated the
> execution-proven DrawSync drain and moved deterministic DMA completion into
> its retail wait polls. The `2/2` caller-checkpoint service measurement below
> is therefore a historical B54K-M observation, not current runtime state.
> The completed `func_8006AD40` body and all non-DrawSync contracts remain
> unchanged.

This rung implements the final 79 retail words of `func_8006AD40`, migrates
all 23 historical prefix-boundary test contracts to complete-function
contracts without dropping their earlier assertions, and measures the real
production caller continuation.  `func_8006AD40` now returns normally; the
next canonical real-disc strict frontier is `func_801909B4` from
`func_8001220C`.

No scheduler destination, `m0360i` special case, persistence bit, synthetic
DMA completion, or planted retail state was added.

## Retail identity and geometry

```text
executable SHA-1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
function range      [0x8006AD40,0x8006B35C)
function size       0x61C bytes / 391 words
function SHA-256    36c7674380e42a7cb5c13f3802689b936bf6e48b67994112c510abbeaaef5539
new suffix range    [0x8006B220,0x8006B35C)
suffix file range   [0x0005BA20,0x0005BB5C)
suffix size         0x13C bytes / 79 words
suffix SHA-256      1aed4d0f3bcaa751da044557dccd15e0f15a002addd7f0333ecaa635885749d8
last instruction    0x8006B358  nop (jr-ra delay slot)
next function       0x8006B35C  addiu sp,sp,-0x18
```

The retained readiness oracle compares all 79 words and verifies all seven
direct calls, packed archive control flow, display-environment arithmetic,
state stores, conditional branches, epilogue, and next-function boundary.

```text
OK identities: exact executable, full body, suffix, 79/79 words
OK +0x188 archive walk: packed header, zero bypass, 0x14 stride
OK display env: D_800BCE80 + D_800ACDDC*20
OK state reset: exact widths/order, 0x40/0x80 gates, bit 0 clear
OK completion geometry: retail epilogue and next-function boundary
B54K-M readiness oracle: PASS
```

## Implemented suffix

The native body now implements each coherent owner in retail order:

```text
0x8006B220..0x8006B270  completed +0x188 archive walk
0x8006B274..0x8006B2BC  stream F1 and display synchronization
0x8006B2C0..0x8006B32C  exact-width state reset and bit-0 clear
0x8006B330..0x8006B358  normal epilogue and return
```

The archive walk reads `base + lw(base+4) + 0x28`, extracts the count from
bits 22..31 and the base-relative entry offset from bits 0..21, bypasses a
zero count, advances by exactly `0x14`, and calls
`func_8006E1C0(entry, base)` once per entry.

The call sequence is exactly:

```text
func_80087024()                 stream command F1
func_80074DC0(0)               DrawSync(0)
func_80074A44(1)               ResetGraph(1), light path
func_80073A44(0)               VSync(0)
func_800755F0(&D_800BCE80[D_800ACDDC])
func_80074D28(1)               SetDispMask(1)
```

The final stores use the retail widths and ordering.  Fields selected by
flag `0x40` and flag `0x80` are preserved when their bit is set and reset to
`0xFF` otherwise.  Common fields are reset unconditionally, and
`D_800B0CD8 &= ~1` is the final guest store.  A second invocation therefore
takes the original bit-0 guard and performs no repeated archive, stream, or
display work.

## Historical-contract migration

The temporary readiness trial produced exactly 23 failures because those
tests intentionally froze the B54K-L boundary.  Every failure was migrated;
no test was deleted and no earlier loop, poll, image-call, provider, transfer,
DMA, or ordering assertion was removed.

| Retained contract family | Tests migrated | Completion replacement |
| --- | ---: | --- |
| Original B50/6AD40 contracts | 5 | normal return, exact final stores, strict direct completion, second-call guard, byte allowlist |
| B54B/B54D counted/material paths | 4 | prior counts/packs retained plus exact suffix state |
| B54C caller/provider relation | 1 | prior atlas/pack checks retained, no remaining provider |
| B54E/F/G poll paths | 6 | prior poll/reissue and no-checkpoint assertions retained plus completion |
| B54KA live 30894 path | 1 | prior complete graphics state retained plus completion |
| B54KJ/K/KL paths | 6 | prior no-replay/lookups/transfers retained plus completion and guard semantics |

The whole-RAM canary remains byte-granular.  Its new allowlist covers only:

- explicit final-state bytes at `D_800B0CD8+0x06`, `+0x09`, `+0x0B..0x0C`,
  `+0xDA..0xDF`, and `+0xE8..0xEB`;
- F1 command word `D_800BCD80`;
- one `0x14`-byte initialized ring record at `D_800B8628`;
- producer/busy words `D_8009D2F4` and `D_8009D268`.

All earlier canaries remain active.  The focused canary test passes after
tightening gaps between separately written bytes rather than allowing one
broad struct range.

## New suffix tests

Two new tests increase the suite from 956 to 958:

1. `B54KM_6AD40_final_archive_positive_and_zero`
   - crafts an independent two-entry +0x188 archive in the Disc fixture;
   - proves exactly two new `func_8006E1C0` dispatches with bases, source
     offsets, rectangles, order, and `0x14` stride;
   - proves a zero-count archive executes no final entry and does not replay
     the preceding E0 walk.
2. `B54KM_6AD40_flag_matrix_and_f1_display`
   - executes all four `0x40/0x80` combinations;
   - verifies preserved versus reset bytes individually;
   - verifies F1 command, one ring entry, zero payload inherited from command
     globals, producer increment, DrawSync, VSync, presentation, and mask 1.

The retained direct strict test now proves that `func_8006AD40` itself has no
bootstrap provider.  The retained repeat tests prove that its first call
clears bit 0 and its second call is a complete no-op.

## Production-caller measurement

`--dma-checkpoint-report` exposes the already-existing value-only checkpoint
trace at clean process shutdown; it does not change guest memory, scheduling,
or checkpoint authority.  A bounded real-disc run reaches both explicit
`func_8001220C` checkpoint calls after `func_8006AD40`:

```text
[HOST] stop_reason=frame-limit
[DMA_CHECKPOINT] calls=2 queries=2 services=2 captured=2 serviced=2
exit=0
```

An unbounded strict run then measures the next provider rather than inferring
it from static order:

```text
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_801909B4
       called from: func_8001220C
exit=1
```

`pc_port/tools/b54km_6ad40_completion_oracle.py` reproduces both runs, checks
the caller's two explicit checkpoint sites before destination dispatch, and
also authenticates the retail suffix and an independent archive/flag model.

```text
OK retail identity: exact executable and 79-word suffix
OK archive model: packed offset, zero bypass, 0x14 stride
OK state model: complete 0x40/0x80 branch matrix and bit-0 owner
OK caller source: two explicit checkpoints before destination dispatch
OK runtime caller: 2 calls/queries/services, token 2 serviced
OK strict continuation: next frontier func_801909B4 from func_8001220C
B54K-M completion oracle: PASS
```

## Gates

```text
focused complete family: 958 run, 24 passed, 0 failed, 934 skipped
focused new suffix:      958 run, 2 passed, 0 failed, 956 skipped
normal full suite:       958 run, 958 passed, 0 failed, 0 skipped
fresh ASan/UBSan:        958 run, 958 passed, 0 failed, 0 skipped
sanitizer diagnostics:   0
git diff --check:        PASS
disc1.candidate SHA-1:   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The exact matching executable is unchanged because this is native-port and
evidence work only.

```text
FUNC_8006AD40=COMPLETE_NATIVE_TRANSLATION
PRODUCTION_REACHABILITY=blocked_at_func_801909B4
FUNC_801909B4_BYTES=STATICALLY_RECOVERED_NOT_IMPLEMENTED
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
