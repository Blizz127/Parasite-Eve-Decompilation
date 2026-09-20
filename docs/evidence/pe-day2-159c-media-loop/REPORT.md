# PE-DAY2-159c — func_80192CE8 post-E08 media loop re-land

Status: **RE-LANDED + DISCRIMINATED**. The authenticated post-E08 media loop
in `func_80192CE8` is back in the native port, `func_80192934`'s got-frame
tail is wired behind the same complete-frame gate as `func_801924F8`, and the
live opening-FMV wall moved off the removed `80192E08_cut`.

## Identity (carve authority — unchanged)

```text
func_80192CE8              [0x80192CE8,0x80192F98)
complete size              0x2B0 / 172 words
complete SHA-256           ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7
post-E08 span              [0x80192E08,0x80192F98)
post-E08 size              0x190 / 100 words
post-E08 SHA-256           77218c9c335f6b4a24416a39d21d09876d7bc9f74778fefb26d28d501dbfd01a
```

Disassembly: `docs/evidence/pe-92ce8-post-e08/func_80192CE8.s.txt`.
The tail is transcribed from that carve; nothing was invented.

## What changed

### `pc_port/game/boot/func_80192CE8_port.c`

Replaced the `Bootstrap_ReturnVoid("func_80192CE8_80192E08_cut", …)` +
`RequestStop` cut with the matched CFG:

1. `D_800B0DBA == 0` → epilogue (0x80192E08).
2. Loop while `(int16)D_800B0DBC > 0` (0x80192E24):
   - `func_8003EB04()` (0x80192E34)
   - `status = func_80192934()` (0x80192E3C)
   - low byte of `status` zero (`sll 24`, 0x80192E44) → zero
     `801D0DE8/DEC/DFC/DF8/DF0/DF4` + `D_800B0DBA`
   - else if `D_8009D26C & 0x20000004` (0x80192E90) → abort path:
     `DBA--`, `870F0(0)`, `C0D8(0)`, `7A2A4()`, `80DC4(9,0,0)`, clear,
     and when the **reloaded** `(int16)D_800B0DBC < 1400`: `VSync(0)`,
     `SetDispMask(0)`, `s3 = 1`.
   - `func_80070E54()` (0x80192F44); loop while `D_800B0DBA != 0`.
3. Epilogue (0x80192F60): `D_800B0CD8 &= ~0x200`; return `s3`.

The frame-count compare reloads `D_800B0DBC` at 0x80192EE8, after the abort
calls (92934's got-frame bumps DBC), matching the carve rather than reusing
the loop-top value.

### `pc_port/game/boot/func_80192934_port.c`

The got-frame tail at 0x80192A68 now mirrors `func_801924F8`:

- E0-style delivery pump on the device path (`HostFB_StreamTick()` when
  `PE_CdReg_DeviceEnabled()`, else the documented `func_8007C214()`
  synchronous surrogate).
- `PE_Movie_LastPublishComplete()` gate before entering the decoder; a
  surrogate / partial publish keeps the named boundary (MV1d in-bounds
  invariant preserved).
- bumps DBC, toggles 146C, `out = [0x801D1464 + (146C^1)<<2]`,
  `table = [0x801D0DF8]`, ranged checks, `func_8010C89C(stream, out, table, 0)`
  then `func_8007C394(stream)`, and falls into the success path with
  `status16 = 0`.
- Unlike `func_801924F8`, retail 92934 has **no** EC stores after `7C394`;
  none were added.

## Discriminating test

`DAY2_92ce8_media_loop_tail` (`pc_port/tests/test_movie_production.h`) loads
the real 133-sector PE.IMG overlay, calls `func_80192CE8(47)` (index ≥ 47
makes `func_801924F8` return at its record-range guard, so `D_800B0DBC = 0`
survives and the loop guard is taken), and asserts:

- return `0` and no stop;
- overlay bit `0x200` cleared.

Measured discrimination (rebuild between):

```text
loop re-landed : TEST DAY2_92ce8_media_loop_tail... PASS
loop re-cut    : TEST DAY2_92ce8_media_loop_tail... FAIL: post-E08 media tail did not run to the epilogue
```

## Live observation (real Disc 1, headless)

Command:

```sh
./pc_port/build/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" \
  --headless --max-frames 1200 --trace /tmp/pe.log
```

Before this change the run stopped at `[STUB:BOOTSTRAP_RET] func_8010C89C`
(the C89C gate inside `func_801924F8`).

After re-landing the loop:

```text
[TRACE 0004] func_80192934_enter
[STUB:BOOTSTRAP_RET] func_80074520_dma_indirect_call
stop_reason=unresolved-boundary
```

`func_801924F8` completed the first frame and returned; the post-E08 loop ran
the input handler and `func_80192934`. The new wall was the DMA1 dispatcher
returning a boundary for the title DecDCTout handler `0x80191DC8` that
`func_801924F8` registers via `func_8010C0D8(0x80191DC8)`.

## Follow-on: 0x80191DC8 dispatch

`pc_port/platform/pe_irq_delivery.c` `DispatchDmaCallback` handled
`0x801214D4`, `0x8007C214` and the GPU pump, but not the title twin
`0x80191DC8` (ported in `pc_port/game/boot/func_80191DC8_port.c`, carve
`docs/evidence/pe-day2-158-91dc8/`, same return shape as `1214D4`). Wiring it
advanced the live run to:

```text
[TRACE 0004] func_80192934_enter
[TRACE 0005] func_80192934_enter
[STUB:BOOTSTRAP_RET] MDEC_decode_busy
stop_reason=unresolved-boundary
```

Two media-worker entries now execute before the next host MDEC boundary.

## Verification commands

```sh
cmake -S pc_port -B pc_port/build
cmake --build pc_port/build -j2
./pc_port/build/pe-native-tests     # 1376 run / 1376 passed / 0 failed / 0 skipped
```

## Remaining / blockers

- Live wall is now `MDEC_decode_busy` (host MDEC busy predicate). The
  DAY2-158u supersede-on-DMA0-commit work addressed this family; the port
  currently rejects on busy rather than superseding the completed orphan.
- The media loop's abort path (`D_8009D26C & 0x20000004`, VSync(0) +
  SetDispMask(0)) is in the port but not yet exercised live.

## Non-claims

- No claim that the opening FMV plays through; only that the post-E08 loop is
  reached and the wall moved twice.
- No claim about Day 2 completeness.
