# B0CD0 catch-up re-landed onto the `HostFB_StreamTick` pipeline

Branch `agent/b0cd0-catchup` from `05dc2b21`. Scope: the CD
pending-sector / B0CD0 catch-up only.

## Baseline (HEAD, before this change)

```text
./pc_port/build/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" \
  --headless --max-frames 1200 --trace /tmp/b0_base.log
```

```text
[STUB:BOOTSTRAP_RET] CD_device_sector_overrun (first invocation)
[FB] vsyncs=115070 drawsyncs=1763 presents=801 mask=1 main_iters=1
[HOST] stop_reason=unresolved-boundary
func_80192934_enter count: 318
```

## Change

`pe_cdreg` no longer treats a still-unread sector at the next read period as
`CD_device_sector_overrun`; it holds the next publish (leaves `g_read_cycles`
at 0) until the retail BFRD path drains the pending sector, exactly the
DAY2-158v backpressure. `PeCdDeviceState` gains `sector_pending` so the host
pump can see the unread-sector ownership.

`HostFB_DeviceTime` first runs a new `HostFB_CdServicePending`, which services
`PE_Port_ServiceDmaIrqCheckpoint()` + `HostFB_ServiceDeviceIrq()` whenever
decode, the B0CD0 defer latch, or `sector_pending` is present, so the guest's
own data-ready IRQ path (`91DC8`/`1214D4` -> `func_8007C564` -> BFRD) retires
the sector. A pending sector that never clears is bounded (4096 service ticks)
into the named `CD_B0CD0_pending_unresolved` stop, so a genuine stall is never
a silent hang. A reentrancy guard covers `func_8007C564`'s own
`func_80073A44(-1)` poll, which re-enters `HostFB_VSync` -> `HostFB_DeviceTime`.

### Rejected variants (measured)

1. **Direct host `func_8007C564()` catch-up** (the literal 158z shape): calling
   the stream assembler outside its retail call context reads an unpopulated
   FIFO and stops at `CD_device_data_underflow` at the first catch-up
   (`[CATCHUP] DBA=1 C0DC8=80142100 C0DB8=00000000`). Not merged.
2. **Clear the pending sector when Stop/Pause invalidates the FIFO**:
   `CD_B0CD0_pending_unresolved` disappears, but the guest then spins in its
   own media-loop retry (attached backtrace
   `func_8007C484 <- func_80191B64 <- func_80192934 <- func_80192CE8 <- func_801909B4 <- func_8001220C`,
   no progress past 319 entries). This is a silent hang, so it is not merged.

## Live result (after)

```text
./pc_port/build/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" \
  --headless --max-frames 1200 --trace /tmp/b0_final.log
```

```text
[STUB:BOOTSTRAP_RET] CD_B0CD0_pending_unresolved (first invocation)
[FB] vsyncs=117143 drawsyncs=1763 presents=801 mask=1 main_iters=1
[HOST] stop_reason=unresolved-boundary
func_80192934_enter count: 319
PORT EXIT: 0   (no timeout / no silent hang)
```

Discrimination: the `CD_device_sector_overrun` STOP is gone; the run survives
the sector that used to overrun, finishes the 319th media step, and stops at a
named boundary instead of hanging. `func_8007C564` is driven by the serviced
IRQ path throughout (3000+ entries observed during diagnosis).

## Residual (honest)

The run still does not advance past ~319 media steps. Diagnosis of the held
state:

```text
[STUCK] lba=192937 reading=0 mode=e0 secs=3195 resp=3208 cmds=11
        b0cd0=0 DBA=2 DBC=319 B89F4=0 C0DC8=80142100 C0DB8=00000000
        BE998=35 A801C=00000001 dma1=00000200 dma0a=0
```

The guest issued a Stop (`reading=0`) with one sector still unread. The retail
reader has no reason to BFRD it, so it cannot be retired; the media loop
(`func_80192CE8` post-E08 / `func_80192934`) keeps waiting for a frame. That is
guest-side completion logic, not a CD-register or host-pump bug, and is left as
the next frontier rather than approximated.

## Tests

```text
./pc_port/build/pe-native-tests
Results: 1376 run, 1376 passed, 0 failed, 0 skipped
```

`test_cd_sector_device` now asserts the DAY2-158v hold contract (no overrun
STOP, `sector_pending` retained, BFRD exposes the held sector's bytes) instead
of the removed `CD_device_sector_overrun` stop.
