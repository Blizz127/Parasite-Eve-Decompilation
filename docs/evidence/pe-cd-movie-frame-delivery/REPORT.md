# PE-CD-MOVIE-FRAME-DELIVERY — complete STR frame at the published cursor (2026-09-19)

Status: **LANDED (live-verified, native suite green)**. `func_8010C89C` now
decodes a real FMV frame in bounds; the live wall moves past the old C89C
boundary to `func_80192CE8_80192E08_cut`.

## Symptom

On the base commit the port booted the real Disc 1, then stopped at the
bootstrap stub `func_8010C89C` after `func_8001220C`:

```text
[DISC] opened '...Disc 1.bin' (210685 user sectors)
[DISC] boot executable loaded into guest RAM
[TRACE 0002] call_func_8001220C
[STUB:BOOTSTRAP_RET] func_8010C89C (first invocation)
[HOST] stop_reason=unresolved-boundary
```

## Root cause (two independent defects)

1. **CD command device never enabled on the production path.** `port_main`
   called `PE_CdReg_EnableDevice(7u)` (DAY2-158i) but the decomp-port
   refactor dropped it; a grep of `pc_port` found no caller. With no device
   `PE_CdReg_ServiceDevice` is inert, so no sector is ever delivered.

2. **E0 published an unpopulated record.** `func_801924F8`'s E0 poll called
   `func_8007C214()` directly every iteration. `7C214` is the *DMA3
   completion* callback: it marks `D_800C0DC8 + D_800BE9E4*32` state 2, and
   `func_8007C484` then hands its body pointer to `func_80191B64`. Calling it
   without advancing the drive skips the retail assembly chain
   (`data-ready IRQ -> func_800813E8 -> func_8007C564 -> arm DMA3 ->
   7C214`), so the published body was all zeros:

```text
[E0DBG] got_frame s1=80142900 BE9E4=0 BE998=0 B89F4=0
[E0DBG] body: 00000000 00000000 00000000 00000000
```

The VLC decoder has no output bound except the bitstream's own
pad/terminator, so those zeros walk `a1` past the 2 MiB guest window (the
MV1d trap, `docs/evidence/pe-mv1d-c89c/REPORT.md`). The boundary stop was
the honest workaround.

## The retail delivery invariant

`func_8007C564` sets `D_800B89F4 = 1` exactly once per frame, when the chunk
it just assembled is the frame's last (`record[6]-1 == record[4]`), and the
DMA3 transfer it arms with `interrupt = last` completes into `7C214` while
that flag is still set. Instrumented proof on the real disc:

```text
[DBG564] enter BE998=0 B89F4=0   [DBG564] idx=0 last=0
...                              [DBG564] idx=8 last=1
[DBG214] enter BE9E4=0 BE998=9 B89F4=1
```

So **`D_800B89F4 == 1` at `7C214` time ⇔ the published body is a complete
frame**. Feeding that body to the decoder gave `rc=0` (pad exit), telemetry
`a0=80142900 a1=80132700 a2=80162100 ret=0` — in bounds.

## Fix

- `pc_port/src/port_main.c`: restore `PE_CdReg_EnableDevice(7u)` after the
  EXE load (and include `pe_cdreg.h`).
- `pc_port/game/boot/func_801924F8_port.c`:
  - E0 poll: when the device is enabled, advance the modeled drive with
    `HostFB_StreamTick()` (device time + IRQ service), which runs the real
    assembly chain. Fixtures with no device keep the documented direct
    `func_8007C214()` surrogate. The give-up/cd-ready spins also pump, so
    they cannot trap.
  - `got_frame`: run the authenticated retail tail (bump `[B0DBC]`, load
    `a1 = [0x801D1464 + ([146C]^1)*4]`, toggle `[146C]`, `a2 = [0x801D0DF8]`,
    `func_8010C89C(s1,a1,a2,0)`, `func_8007C394(s1)`, EC stores
    `[B0DBD]=0` / `[B0DBA]++` / `[B0DBC]=1`). It refuses to decode unless the
    publish was a last-chunk publish (loud `func_8010C89C` boundary
    otherwise), and it range-checks the arena/table before entry.
- `pc_port/platform/pe_libcd.c` + `pe_sdk.h`: host-only
  `PE_Movie_LastPublishComplete()` / `PE_Movie_ResetPublishState()` capture
  the `D_800B89F4` state at `7C214` (never guest authority).

## Test

`pc_port/tests/test_movie_production.h` — `DAY2_movie_production_frame`
(disc-gated). It builds the production record/buffer/table state, loads the
libpress module from Disc 1, calls the **real `func_801924F8(0)`** on
`\FMV1\FMV001.STR;1`, and asserts:

1. the worker returns without a boundary stop;
2. `func_8010C89C` was entered at the assembled body cursor (`tel.a0 == body`)
   and exited via the pad path (`tel.ret == 0`);
3. the assembled 9×2016-byte body hashes to the on-disc frame
   (`MOVCOMPLETE_cases[0].input_hash = 0x0D17397B6FEAB122`);
4. the decoded RLE hashes to `0xAAED494150FD7400` and the 16 guard bytes past
   the declared 7300-byte output stay untouched (in-bounds invariant);
5. the exit state is `B0DBC=1`, `B0DBA=1`;
6. the gate predicate: `7C214` reports "complete" only with `B89F4` set.

Discriminating power verified: reverting the E0 pump to the direct
`func_8007C214()` call makes the test **FAIL** (`production frame worker
stopped`).

## Verify block

```text
cmake -S pc_port -B pc_port/build && cmake --build pc_port/build -j
./pc_port/build/pe-native-tests
#   Results: 1375 run, 1375 passed, 0 failed, 0 skipped

./pc_port/build/parasite-eve-port --headless --max-frames 1200 --auto-quit \
  --disc-image "<Disc 1>.bin" --trace /tmp/pe.log
#   ... [TRACE 0003] call_func_8001220C
#   [STUB:BOOTSTRAP_RET] func_80192CE8_80192E08_cut (first invocation)
#   [HOST] stop_reason=unresolved-boundary
#   (no func_8010C89C stub: the decoder ran for real)
```

## Non-claims

- The decoded frame is not uploaded/rendered: the movie upload loop after
  `0x80192E08` is still the untranslated `func_80192CE8` remainder, which is
  now the live frontier. The screenshot is still black.
- No pixel/hardware golden; the decoder RLE hash is the transcribed-instruction
  oracle already used by `DAY2_movie_complete_frame`.
- Published runtime 128 is unchanged; nothing was rebuilt or republished.
