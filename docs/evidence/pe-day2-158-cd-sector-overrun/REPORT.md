# DAY2-158 — `CD_device_sector_overrun` dig (tip `a899b2d`)

Status: **DIG CLOSED (root cause + smallest next change)**. Live Disc1 after
DAY2-158u MDEC clear reached deep FMV then STOP'd here. No dual-sector buffer
invent; no pe_cdreg overwrite of unread sectors.

## Live wall (post-158u)

```text
… MDEC_decode_busy cleared …
C89C final: calls=319 out=101510
[GPU] fills=318 …
STUB / boundary: CD_device_sector_overrun
stop_reason=unresolved-boundary
```

Site: `pc_port/platform/pe_cdreg.c:134` —
`if (g_sector_pending) CdDeviceBoundary("CD_device_sector_overrun", next_lba)`.

## Who sets / clears `g_sector_pending`

| Action | Effect |
|--------|--------|
| `ServiceDevice` sector cadence fires | reads raw sector, **`g_sector_pending=1`**, pushes INT1 |
| Index0 write `+3` with bit7 (BFRD) when FIFO empty + pending | copies sector→FIFO, **`g_sector_pending=0`** |
| `CdClearData` / Init / Reset / Init command | clears pending |
| `ServiceDMA3` | drains FIFO only; **does not** clear pending |

Host model is **one pending sector + independent FIFO**
(`DAY2_CD_SECTOR_DEVICE.md`). Unread pending + another cadence → explicit STOP
(test `DAY2_cd_sector_device` overrun case). Not dual-buffer HW fidelity.

## Retail stream contract (existing decomp — not invented)

`func_8007C564` (`cd_stream_port.c`):

1. Early return if `D_800B89F4 == 1`.
2. **DMA1 early-out:** if `D_800A801C` and MDEC DMA1 CHCR busy (`StreamOutputChcr()&0x01000000`):
   - store **`D_800B0CD0 = 1`** (retry latch)
   - `StreamAdvanceMemory` / `9B374=1`
   - **return without BFRD / without DMA3**
3. Otherwise BFRD (`+3 ← 0x80`) then DMA3 body copy.

Title `func_80191DC8` / player `func_801214D4` (ported):

```c
if ((int8_t)PE_LoadU8(0x800B0DBBu) && (int16_t)PE_LoadU16(0x800B0CD0u)) {
    func_8007C564();
    PE_StoreU16(0x800B0CD0u, 0u);
}
```

Documented in `DAY2_STREAM_RECORD_ASSEMBLY.md` / `DAY2_MOVIE_SLICE_CALLBACK.md`:
MDEC output busy **defers** stream work via B0CD0; DMA1 callback **retries**.

## Why INT1 ack makes overrun reachable on the host

`func_8007AAB4` **acknowledges** the CD interrupt (clears `g_response_tag`)
**before** the data-callback chain reaches `813E8` → `7C564`.

So the DMA1 early-out path leaves:

- `g_sector_pending == 1` (no BFRD)
- `g_response_tag == 0` (already acked)
- `B0CD0 == 1` (retry scheduled)

`ServiceDevice` only suppresses the next sector while
`g_response_tag` is set. After AAB4, the next full sector period **is allowed**
to attempt delivery → hits line 134.

On hardware, a multi-sector CD buffer absorbs that race. This host deliberately
does not implement that queue; the STOP is the loud single-buffer boundary.

## Host race that trips it after ~318 frames

`func_80192934` poll (and `924F8` E0) call `HostFB_PumpCdProgress()`:

1. `PE_MDEC_Service()` — may complete DMA1
2. `HostFB_ServiceDeviceIrq()` **only if** `PE_MDEC_HasDecode()`
3. `HostFB_DeviceTime(451584)` — one sector cadence attempt

After 158u, C89C/GPU run for hundreds of frames (`calls=319`, `fills=318`).
When a data-ready IRQ hits while DMA1 CHCR is still busy (common around
BFA0→C01C / mid-slice):

1. `7C564` takes the A801C+DMA1 early-out → **B0CD0=1**, sector stays pending
2. Next `PumpCdProgress` advances another sector period **before** the DMA1
   callback retries `7C564` (or IRQ service is skipped when `!HasDecode`)
3. `CD_device_sector_overrun`

Not a missing MDEC supersede (158u already applied). Not inventing a second
CD buffer. Root cause is **sector_pending left set by the existing 7C564 DMA1
early-out / B0CD0 retry**, racing a host pump that always advances CD time.

## Ruled out / low confidence as primary

| Hypothesis | Why not primary |
|------------|-----------------|
| Silent FIFO overwrite | pe_cdreg never clears pending without BFRD; STOP is intentional |
| DMA3 early-out alone | DMA3 only runs after BFRD; defer path never reaches DMA3 |
| B0CD0 never wired | 91DC8/1214D4 already call 7C564 when DBB && B0CD0 |
| Need dual-sector invent | Would mask the pump/retry race; contradicts Stage149 STOP contract |

## Smallest honest next change

**Do not** add multi-sector buffering in `pe_cdreg` (keep overrun STOP when
there is no retail retry outstanding).

In `HostFB_PumpCdProgress` (and the same preamble used before CD time):

1. Always run `PE_MDEC_Service()`.
2. Service device IRQs when `HasDecode` **or** signed `B0CD0` pending (so
   `91DC8`/`1214D4` can retry `7C564` → BFRD).
3. If `B0CD0` is still pending after that drain attempt, **return without**
   `HostFB_DeviceTime` — stall CD cadence until the existing DMA1→B0CD0
   retry clears the pending sector.

Uses only existing decomp (`7C564` / `91DC8` / `1214D4` / B0CD0). Keeps
`DAY2_cd_sector_device` overrun STOP for the no-consumer case. No admit-gate /
cursor invent. No Day2-complete claim.

## Expect on Matt Disc1 after fix

Pump during 92934/924F8 should no longer march CD time over a B0CD0-deferred
sector. Expect C89C/GPU to continue past ~319 or hit the next *named* wall
(movie end / title-cut / other). Still expect eventual
`post_movie_title_cut`. Linux-first.

## Filters

- `CD_device_sector_overrun|g_sector_pending|B0CD0|A801C`
- `func_8007C564|StreamOutputChcr|HostFB_PumpCdProgress`
- `func_80191DC8|func_801214D4|c89c_tel|fills=`
