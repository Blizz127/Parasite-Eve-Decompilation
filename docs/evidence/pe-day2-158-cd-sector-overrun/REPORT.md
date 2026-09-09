# DAY2-158v: `CD_device_sector_overrun` → pending-sector backpressure

## Live wall (Matt Disc1 tip `a899b2d` / DAY2-158u)

MDEC supersede dig cleared `MDEC_decode_busy`. New stop after ~319 FMV frames:

```text
C89C calls=319 ret=0 pad=319 out=101510
many func_80192934_enter
GPU fills=318
STUB: CD_device_sector_overrun
stop_reason=unresolved-boundary
```

## Site

`pc_port/platform/pe_cdreg.c` — `PE_CdReg_ServiceDevice` sector publish path:

Previously: if `g_sector_pending` when the next sector period elapsed →
`CdDeviceBoundary("CD_device_sector_overrun")` + STOP.

Cause: single-sector model + `HostFB_PumpCdProgress` retiring one full
non-XA period (`451584` cycles) per E0/`92934` poll can outrun
INT1 → BFRD → DMA3 drain. After hundreds of successful frames the race
hits; STOP was an artificial host-pump wall, not a missing Decomp leaf.

## Fix

Prefer **backpressure** over multi-sector invent or silent overwrite:

- While `g_sector_pending`, do **not** publish, do **not** STOP.
- Leave `g_read_cycles == 0` so the next `ServiceDevice` retries after BFRD
  clears pending (catch-up publish).
- Fall through to `PE_CdReg_ServiceDMA3` / IRQ assert as before.

No Decomp leaf change — device model only. `CD_device_sector_overrun` is
retired as a STOP name; tests assert it does not fire.

## Tests

`DAY2_cd_sector_device`: hold while pending → BFRD drain → catch-up second
sector. `HostFB_PumpCdProgress_sector_scale` unchanged.

## Not claimed

Movie finish, Day2-complete, retail overrun-bit fidelity, or multi-sector
hardware queues.
