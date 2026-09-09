# DAY2-158 — B0CD0 catch-up miss after 158y (orphan pending)

Status: **DIG CLOSED (root cause + smallest next change)**.
Reconciles with Auto **DAY2-158y** `@6cbeb1ee` (fold dig/b0cd0-no-bfrd).
Matt Disc1 on that tip **still** silent-hangs at exactly 319× `func_80192934_enter`
with binary containing `CD_B0CD0_pump_retry` / `CD_B0CD0_retry_unresolved` but
**zero** firings — catch-up `if (b0cd0 && pending)` never entered.

## Live fact (Port-confirmed)

```text
tip 6cbeb1ee / DAY2-158y
~319× func_80192934_enter — Square logo → garbled FMV wall
CD_B0CD0_pump_retry         : 0
CD_B0CD0_retry_unresolved   : 0
CD_B0CD0_dma1_starved       : 0
```

158y’s clear-only-after-BFRD logic is unreachable if the arm’s predicate is
false. Residual is **why `b0cd0 && pending` is never both true at the check**,
not another A801C gate bug.

## Pump path during 92934 spin (existing decomp)

`func_80192934` poll (`pc_port/game/boot/func_80192934_port.c`):

```c
last_chunk = (B89F4 == 1);
if (last_chunk || ConsumeStreamPromote())
    func_8007C214();
else if (PE_CdReg_DeviceEnabled())
    HostFB_PumpCdProgress();  /* reached whenever !last_chunk */
```

Pump **is** on the spin path (DeviceEnabled; promote fixture-only). Hang is
inside poll waiting on `91B64`, not a missing Pump call.

`HostFB_PumpCdProgress` @158y order:

1. `PE_MDEC_Service()` (may retire DMA1 / clear HasDecode)
2. IRQ gate: `if (HasDecode || b0cd0) ServiceDmaIrqCheckpoint + ServiceDeviceIrq`
   — checkpoint can run **retail `91DC8`**, which always `sh r0, B0CD0` after
   `jal 7C564` (carve `docs/evidence/pe-day2-158-91dc8/func_80191DC8.s.txt`
   @80191DFC–80191E00), even when `7C564` returned without BFRD
3. Re-read `b0cd0` / `sector_pending`
4. Catch-up only if **`b0cd0 && pending`** (158y)
5. Else `HostFB_DeviceTime` → pe_cdreg **hold** while unread pending
   (`pe_cdreg.c` backpressure) — no TRACE, no STOP

## Why both flags are not true (root cause)

Unread ownership is **`sector_pending`**, not B0CD0. B0CD0 is only the
`7C564` A801C+DMA1 defer latch (`cd_stream_port.c`).

Two in-tree ways to reach **`pending && !b0cd0`** (catch-up miss → silent hold):

| Path | Evidence |
|------|----------|
| **91DC8 clear-without-BFRD** | Retail twin always clears B0CD0 after `7C564`. Pump preamble runs that callback *before* the catch-up re-read. If `7C564` early-outs (`B89F4==1`, response bit2, occupied record, `A488==5`, …) pending stays, latch gone → step 4 never enters. Same shape 158y fixed *inside* Pump’s own clear. |
| **Non-defer early-out** | `7C564` top `B89F4==1` / occupied / bit2 returns **before** the DMA1 arm that sets B0CD0. INT1 already acked by `7AAB4` in `7C13C`. Pending stuck, B0CD0 never set. |

IRQ gate `HasDecode \|\| b0cd0` (no `sector_pending`) compounds this: after
MDEC goes idle and latch is clear, a held pending sector does not even force
CD IRQ service on the next Pump.

Ruled out as primary for *zero TRACE* on 158y:

| Hypothesis | Why not |
|------------|---------|
| Pump never reached | 92934 poll calls it when `!B89F4` and device on |
| DMA1 busy forever | Would eventually `CD_B0CD0_dma1_starved`; live has none |
| 158y clear bug still | Arm never entered; clear code irrelevant |
| Need dual-sector buffer | Masks missing BFRD; Stage149 forbids |

## Smallest honest next change

In `HostFB_PumpCdProgress` only (keep matched `91DC8`/`1214D4` bodies):

1. Read `sector_pending` **before** the IRQ gate; also service when pending.
2. Idle-DMA1 catch-up on **`pending && !dma1_busy`** (B0CD0 optional).
3. Keep busy-DMA1 stall / `dma1_starved` only for `b0cd0 && pending && dma1_busy`.
4. Unchanged 158y: after catch-up `7C564`, clear B0CD0 only if pending gone;
   else `CD_B0CD0_retry_unresolved`.

No dual-sector invent. No retail `91DC8` clear rewrite (host Pump owns the
orphan recovery that retail’s successful BFRD made unnecessary).

## Tests

- Existing idle-dma1 + stall retained.
- New `DAY2_cd_b0cd0_pump_orphan_pending`: pending set, **B0CD0=0**, DMA1 idle,
  `B89F4==1` → must TRACE `pump_retry` and STOP `retry_unresolved` (proves arm
  no longer requires B0CD0).

## Expect on Matt Disc1

- Happy: orphan/ deferred sector BFRDs → C89C ≫319 / next named wall.
- Unhappy: **named** `CD_B0CD0_retry_unresolved` / `dma1_starved` instead of
  silent 319 timeout — then dig that `7C564` early-out with evidence.

## Filters

- `CD_B0CD0_pump_retry|CD_B0CD0_retry_unresolved|CD_B0CD0_dma1_starved`
- `800B0CD0|sector_pending|HostFB_PumpCdProgress|func_80191DC8|func_80192934_enter`
