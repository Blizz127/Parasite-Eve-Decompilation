# DAY2-158 — `CD_device_sector_overrun` / B0CD0 pump stall

Status: **DIG FOLDED** (`dig/cd-sector-overrun` @ `355af810`) as **DAY2-158w**.
No dual-sector buffer invent; no silent overwrite of unread sectors.

## Live chronology

| Tip | Result |
|-----|--------|
| `a899b2d` (158u) | Past MDEC; STOP `CD_device_sector_overrun` after C89C×319 |
| `222bd95b` (158v / PR #42) | Overrun cleared; **hang** ~319× `func_80192934_enter`, no TRACE/STOP (90s/180s budgets) |

158v pe_cdreg hold alone: pump stalls until BFRD, but BFRD never arrives when
`7C564` took the A801C+DMA1 early-out (B0CD0 set, INT1 already acked by AAB4).

## Retail contract (existing decomp — not invented)

`func_8007C564` (`cd_stream_port.c`):

1. Early return if `D_800B89F4 == 1`.
2. **DMA1 early-out:** if `D_800A801C` and MDEC DMA1 CHCR busy:
   - store **`D_800B0CD0 = 1`** (retry latch)
   - `StreamAdvanceMemory` / `9B374=1`
   - **return without BFRD / without DMA3**
3. Otherwise BFRD (`+3 ← 0x80`) then DMA3 body copy.

Title `func_80191DC8` / player `func_801214D4`:

```c
if ((int8_t)PE_LoadU8(0x800B0DBBu) && (int16_t)PE_LoadU16(0x800B0CD0u)) {
    func_8007C564();
    PE_StoreU16(0x800B0CD0u, 0u);
}
```

MDEC output busy **defers** stream work via B0CD0; DMA1 callback **retries**.

## Why host Pump raced / hung

`func_8007AAB4` acknowledges CD INT1 **before** the data-callback chain reaches
`813E8` → `7C564`. DMA1 early-out leaves:

- `g_sector_pending == 1` (no BFRD)
- response tag clear (already acked)
- `B0CD0 == 1` (retry scheduled)

`HostFB_PumpCdProgress` previously always advanced one sector period and only
serviced device IRQs when `PE_MDEC_HasDecode()`. After a final-slice orphan,
HasDecode can be false while B0CD0 is set → **no** `91DC8` retry → no BFRD.

- Pre-158v: next cadence → `CD_device_sector_overrun` STOP.
- 158v hold-only: cadence holds forever → live hang at ~319×92934.

## Fix (DAY2-158w — dig draft applied)

In `HostFB_PumpCdProgress` / VSync IRQ preamble:

1. Always run `PE_MDEC_Service()`.
2. Service device IRQs when `HasDecode` **or** signed `B0CD0` pending.
3. If `B0CD0` still owns `sector_pending` after that drain attempt, **return
   without** `HostFB_DeviceTime` — stall CD cadence until DMA1→B0CD0 retry
   clears the sector.

Keep pe_cdreg single-sector **hold** (no STOP while pending) as VSync-path
safety. Expose `sector_pending` on `PE_CdReg_GetDeviceState`. Test:
`DAY2_cd_b0cd0_pump_stall`.

## Not claimed

Movie finish, Day2-complete, dual-sector HW queues, or retail overrun-bit
fidelity. Matt Disc1: expect C89C ≫319 or the next *named* wall.
