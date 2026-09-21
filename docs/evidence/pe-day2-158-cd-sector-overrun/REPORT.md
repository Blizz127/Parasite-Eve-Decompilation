# DAY2-158 — CD sector overrun / B0CD0 hang

## Live chronology

| Tip | Result |
|-----|--------|
| `a899b2d` (158u) | Past MDEC; STOP `CD_device_sector_overrun` @ C89C×319 |
| `222bd95` (158v) | Overrun cleared; hang ~319×92934 |
| `733a8dc` (158w) | Dig IRQ/stall; hang unchanged |
| `331c945` (158x) | Idle-DMA1 catch-up; **FAILED** — same silent hang (no STOP) |
| `6cbeb1ee` (158y) | Dig `b0cd0-no-bfrd` folded — awaiting Matt Disc1 |

## Dig (`dig/cd-sector-overrun` @ `355af810`) → 158w

`7C564` A801C+DMA1 early-out sets `B0CD0` without BFRD after AAB4 acked INT1.
Retail reopen: DMA1 callback `91DC8`/`1214D4` retries `7C564` when B0DBB≠0.
158w: Pump stall + IRQ widen while B0CD0 owns pending. pe_cdreg hold kept.

## Why 158w/158x still hung

158w only helps when DMA1 **completes** and `91DC8` runs. Live can leave
`B0CD0` + pending with **DMA1 already idle** and/or `DBB==0` so `91DC8`
skips `7C564`. 158x catch-up then (1) gated `7C564` on A801C and (2) always
cleared B0CD0 → pending stuck, latch gone → silent hang (named STOP dead).

## DAY2-158y (dig `b0cd0-no-bfrd` @ `4026de2`, folded)

See `docs/evidence/pe-day2-158-b0cd0-no-bfrd/REPORT.md`. Always call
`func_8007C564`; clear B0CD0 only after pending clears; else STOP
`CD_B0CD0_retry_unresolved`. Tip `6cbeb1ee`.

## Not claimed

Movie finish / Day2-complete / dual-sector HW queues. Matt Disc1: C89C ≫319
or named `CD_B0CD0_*`.
