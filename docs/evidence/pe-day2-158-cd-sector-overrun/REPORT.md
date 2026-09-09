# DAY2-158 — CD sector overrun / B0CD0 hang

## Live chronology

| Tip | Result |
|-----|--------|
| `a899b2d` (158u) | Past MDEC; STOP `CD_device_sector_overrun` @ C89C×319 |
| `222bd95` (158v) | Overrun cleared; hang ~319×92934, no TRACE/STOP |
| `733a8dc` (158w) | **FAILED retest** — same hang; dig IRQ/stall insufficient |

## Dig (`dig/cd-sector-overrun` @ `355af810`)

`7C564` A801C+DMA1 early-out sets `B0CD0` without BFRD after AAB4 acked INT1.
Retail reopen: DMA1 callback `91DC8`/`1214D4` retries `7C564` when B0DBB≠0.

## Why 158w still hung

158w stalled Pump while B0CD0 owned `sector_pending` and widened IRQ service
on B0CD0. That only helps when DMA1 **completes** and `91DC8` runs the retry.

Live hung with BFRD never arriving: `B0CD0` + pending can persist with **DMA1
already idle** (DMA IRQ not delivered to `91DC8`, or `91DC8` B0DBB gate
skips `7C564` after format XOR / 16bpp). Pump then waited forever.

## DAY2-158x fix

`HostFB_PumpCdProgress`:
1. `ServiceDmaIrqCheckpoint` after MDEC when B0CD0 set.
2. Idle DMA1 + B0CD0 + pending → `CD_B0CD0_pump_retry` + `func_8007C564` + clear B0CD0.
3. DMA1 busy ≥64 stalls → named STOP `CD_B0CD0_dma1_starved`.
4. pe_cdreg single-sector hold retained.

Tests: `DAY2_cd_b0cd0_pump_stall` (dma1 armed), `DAY2_cd_b0cd0_pump_retry_idle_dma1`.

## Not claimed

Movie finish / Day2-complete / dual-sector HW queues.
