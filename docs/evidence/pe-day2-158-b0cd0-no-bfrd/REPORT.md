# DAY2-158 — B0CD0 / no BFRD after 158x idle-DMA1 catch-up

Status: **DIG CLOSED (residual root cause + smallest next change)**.
Reconciles with Auto **DAY2-158x** `@331c945` / `9324384` (idle-DMA1 Pump
catch-up). Matt Disc1 on that tip **still** hangs ~319× `func_80192934_enter`
with no named `CD_B0CD0_*` STOP. No dual-sector invent.

## Live chronology

| Tip | Result |
|-----|--------|
| `a899b2d` (158u) | STOP `CD_device_sector_overrun` @ C89C×319 |
| `222bd95` / `11c5e94` (158v) | Overrun held; silent hang ~319×92934 |
| `733a8dc` (158w) | IRQ widen + Pump stall; **same silent hang** |
| `331c945` / `9324384` (158x) | Idle-DMA1 catch-up; **same silent hang** (no `CD_B0CD0_dma1_starved` / no `CD_B0CD0_retry_unresolved`) |

## What 158x already got right (fold)

158w assumed DMA1 completion → `91DC8` → `7C564` BFRD. Live can leave
`B0CD0` + `sector_pending` with **DMA1 already idle** (IRQ miss, or `91DC8`
signed-`DBB` gate skips retry after bank XOR / 16bpp). Dig and 158x agree:
Pump must catch up with the existing `7C564` leaf when DMA1 is idle — not wait
forever. 158x also runs `ServiceDmaIrqCheckpoint` on B0CD0 and times out a
busy-DMA1 wait as `CD_B0CD0_dma1_starved`. Keep those.

## Why BFRD still never fires after 158x (residual)

`HostFB_PumpCdProgress` idle arm (`9324384`):

```c
Bootstrap_ReturnVoid1("CD_B0CD0_pump_retry", ...);
if (PE_LoadU32(0x800A801Cu)) {          /* (1) A801C gate */
    func_8007C564();
    if (PE_Port_ShouldStop()) return;
}
PE_StoreU16(0x800B0CD0u, 0u);           /* (2) clear latch always */
/* refresh pending / b0cd0 */
if (b0cd0 && pending) {                 /* (3) dead after (2) */
    /* CD_B0CD0_retry_unresolved STOP */
}
```

Three host bugs, all visible in-tree (no invent):

1. **A801C gate is not in retail `91DC8`/`1214D4`.** Those leaves call
   `7C564` whenever signed DBB && B0CD0. `7C564` itself owns the A801C+DMA1
   early-out. Guarding the catch-up with A801C **skips BFRD entirely** when
   the stream option is already clear, while the unread `sector_pending` from
   the earlier defer remains.

2. **B0CD0 is cleared even when `7C564` did not BFRD** (skipped by (1), or
   early-returned on `B89F4==1` / poll / occupied-record / response bit2 —
   all existing `cd_stream_port.c` paths that return *before* `+3 ← 0x80`).

3. **`CD_B0CD0_retry_unresolved` is dead.** It requires `b0cd0 && pending`
   *after* the unconditional clear, so it never fires. Pump then falls
   through to `DeviceTime`; `pe_cdreg` hold blocks the next publish →
   **same silent ~319×92934 hang**, matching Matt Disc1 (no named STOP).

Unit test `DAY2_cd_b0cd0_pump_retry_idle_dma1` currently asserts only
`pump_retry` TRACE + B0CD0 cleared — it never sets A801C and never checks
`sector_pending`. It green-lights the residual bug.

Ruled out as primary for the *post-158x* wall:

| Hypothesis | Why not primary now |
|------------|---------------------|
| DMA1 still busy forever | Would hit `CD_B0CD0_dma1_starved` (≥64); live has no such STOP |
| `91DC8` still unbound | C89C×~319 already; 158x also checkpoints DMA IRQ on B0CD0 |
| Need dual-sector buffer | Would mask missing BFRD; Stage149 STOP contract forbids |
| pe_cdreg overwrite | Hold path does not clear pending without BFRD |

## Smallest honest next change

In the 158x idle catch-up arm only:

1. **Always** call `func_8007C564()` (drop the A801C if-gate; leaf has its own).
2. Re-read `sector_pending`.
3. **Clear B0CD0 only if pending is clear** (BFRD happened).
4. If pending still set → keep B0CD0 (or leave it) and **STOP**
   `CD_B0CD0_retry_unresolved` (make the existing named wall reachable).

No dual-sector invent. Do not rewrite matched `91DC8` DBB beq. Keep busy-DMA1
stall + starved STOP.

## Expect on Matt Disc1

- Happy path: catch-up `7C564` BFRDs → C89C ≫319 / next named movie wall.
- Unhappy path: **named** `CD_B0CD0_retry_unresolved` (or `dma1_starved`)
  instead of silent timeout — then dig that early-out (`B89F4` / poll /
  occupied) with evidence. No Day2-complete claim. Linux-first.

## Filters

- `CD_B0CD0_pump_retry|CD_B0CD0_retry_unresolved|CD_B0CD0_dma1_starved`
- `800A801C|800B0CD0|800B89F4|sector_pending|HostFB_PumpCdProgress`
- `func_8007C564|func_80191DC8|func_80192934_enter`
