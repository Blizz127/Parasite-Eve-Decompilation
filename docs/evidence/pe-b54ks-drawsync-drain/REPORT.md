# PE-B54K-S — deterministic DrawSync DMA drain

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

> **B54K-T supersession note (2026-08-30):** B54K-T now enters
> `func_80190660`, uses this DrawSync implementation for its image records,
> and advances the real-disc frontier to `func_80075358 from func_80190660`.
> The 971-test and prior-frontier measurements below remain B54K-S history.

This rung translates the execution-proven DrawSync wrapper and queue/DMA drain
needed by the B54K-R display prefix. It preserves the retail owner boundaries:
the guest-ring pump remains the only ring consumer, and
`PE_Port_ServiceDmaIrqCheckpoint` remains the only native owner of DMA2
completion, DICR/CPU-IRQ publication, callback dispatch, and post-callback
pumping. DrawSync polling does not itself evolve GPU hardware.

No destination token, scheduler state, `m0360i` special case, or
`persist[0] |= 4` store was introduced.

## Retail identity and function geometry

```text
executable SHA-1          452fb033f2eaa4b18aa20a5bca60b8125af3a37b

func_80074DC0 wrapper     [0x80074DC0,0x80074E28)
file range                [0x000655C0,0x00065628)
size                      0x68 / 26 words
SHA-256                   7b6e49da8c7c3721966165e7c54af37e6f4bf81fca08422ebc1a03badf6b0e98

func_80077294 drain       [0x80077294,0x800773D0)
file range                [0x00067A94,0x00067BD0)
size                      0x13C / 79 words
SHA-256                   4351d2c705b84f1524116d7eca95adbe416603e81351c90bab22544fac3e71a8

func_80077404 wait        [0x80077404,0x80077548)
file range                [0x00067C04,0x00067D48)
size                      0x144 / 81 words
SHA-256                   50930b6c4a1a412ef01274469773764ee5a45daa13e94464a0a446523b12a59a
```

All three ranges are authenticated directly from the accepted executable.
The DrawSync wrapper dispatches through jump-table slot 15:

```text
D_80095740 / jtb[15]      0x80077294
drain direct-call order   800773D0, 80076EE4, 80077404,
                          80077404, 80076EE4
```

The normal wait-poll path is inside `func_80077404`; its destructive timeout
recovery is structurally separate and remains fenced.

## Native contract

For mode zero, `func_80077294` resets the retail timeout bookkeeping and
repeats the same three classes of work until all are clear:

1. consume a pending guest-ring entry through `func_80076EE4`;
2. wait for an active DMA2 token to complete;
3. wait until the GPU reports GP0 ready.

At each authenticated wait-helper poll, native may admit exactly one DMA2
token that was already active before the poll. It does so only through
`PE_Port_ServiceDmaIrqCheckpoint`. The poll never calls the lower-level DMA
completion routine directly, and register/status reads have no side effect.
The translated pump remains the only code that advances the ring consumer.

For nonzero mode, the implementation preserves retail's pending/status query:
it may give the pump one ordinary opportunity when entries are pending, then
returns the pending count while DMA is busy or the GPU is not ready; an idle
zero-pending GPU returns zero, while an idle but not-ready zero-pending GPU
returns one.

The execution-proven normal half of `func_80077404` preserves signed deadline
comparison, the `0x000F0000` poll threshold, and one poll-word increment per
successful poll. Two paths deliberately remain named boundaries:

```text
func_80077404_timeout_recovery_cut
    destructive timeout recovery is not yet translated

func_80077404_wait_cut
    no external progress source exists for the observed state
```

These cuts stop execution instead of synthesizing completion or spinning
forever. `HostFB_DrawSync` remains telemetry/presentation policy only.

## Focused contracts

Four B54K-S tests prove:

1. idle mode-zero completion and exact nonzero pending/status returns;
2. one direct active DMA transfer completes through the checkpoint owner;
3. a queued second transfer is consumed by the retail pump and both transfers
   drain in order;
4. GPU-not-ready state with no external progress reaches the named cut and
   does not mutate hardware into readiness.

The queued case also proves that a worker's own `func_800773D0` reset is
observed: the final poll count is one, not an invented cumulative two.

```text
focused B54K-S:    971 run, 4 passed, 0 failed, 967 skipped
normal full suite: 971 run, 971 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  971 run, 971 passed, 0 failed, 0 skipped
sanitizer diagnostics: 0
```

`pc_port/tools/b54ks_drawsync_oracle.py` imports no production code. It
authenticates the 26-, 79-, and 81-word retail bodies and jump-table target;
independently models pending/status and timeout-threshold behavior; checks the
single-token event bridge and both source fences; runs all four focused
contracts; and measures the real-disc frontier.

```text
  OK retail: 26-word wrapper, 79-word drain, 81-word wait
  OK model: pending/status returns and wait threshold
  OK source: one-token event bridge; timeout paths fenced
  OK focused: idle, direct DMA, queued DMA, no-progress cut
  OK runtime: 26 DMA events drained; func_80190660 remains next

B54K-S DrawSync oracle: PASS.
```

## Production trace and supersession

Strict real-disc execution reaches the same next provider as B54K-R:

```text
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80190660
       called from: func_801909B4
```

Normal real-disc execution records:

```text
[STUB:BOOTSTRAP_RET] func_80190660
[FB] vsyncs=6 drawsyncs=3 presents=3 mask=0 main_iters=1
[HOST] stop_reason=unresolved-boundary
[DMA_CHECKPOINT] calls=27 queries=26 services=26 captured=26 serviced=26
```

B54K-M's earlier `2/2` caller-checkpoint trace is therefore historical. Its
complete `func_8006AD40` semantic contracts remain valid, but DrawSync now
drains the active DMA at the retail synchronization owner.

## Gates and disposition

```text
git diff --check:       PASS
disc1.candidate SHA-1:  452fb033f2eaa4b18aa20a5bca60b8125af3a37b
FUNC_80074DC0=DRAWSYNC_WRAPPER_ADAPTED
FUNC_80077294=EXECUTION_PROVEN_PATHS_TRANSLATED
FUNC_80077404=NORMAL_POLL_TRANSLATED_TIMEOUT_RECOVERY_FENCED
DMA_PROGRESS=ONE_ALREADY_ACTIVE_TOKEN_PER_RETAIL_WAIT_POLL
RING_CONSUMER=FUNC_80076EE4_ONLY
PRODUCTION_REACHABILITY=blocked_at_func_80190660_from_func_801909B4
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=translate_func_80190660_prefix_to_first_DrawPrim
```
