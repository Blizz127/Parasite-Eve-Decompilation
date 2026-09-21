# FMV autonomous stream — gap closure report

Scope: the `pc_port/` gap called out as "retail CD/DMA pointer-table
provenance + XA ReadS (bit6 / 0x50) so `121C04` finishes autonomous frames".
This report states precisely what was reproduced, what is proven by test, and
what remains unproven. It does not claim a full boot→Day-2 run.

## 1. What the gap actually was

`func_80121C04` opens the opening FMV as a raw CD stream through
`func_80081314` with read mode `0x1E0` (double-speed `0x80` | XA-select `0x40`
| 2340-byte `0x100`). Two independent things stopped the port from delivering
a frame:

1. **Read-mode gate.** The modeled CD controller treated modes carrying the XA
   select (`0x40`) / double-speed (`0x80`) bits as a boundary and refused to
   serve sectors. The movie path therefore never received a sector, and the
   player fell into `movie_player_start_wait` / `movie_retry_wait`.
2. **Stream-wait time quantum.** The guest's per-iteration poll
   (`func_80121270`) stands for far more host device time than a single
   1024-cycle counter query. With a 1024-cycle quantum the modeled 2× drive
   delivered only ~3 sectors inside the 2000-iteration retry budget, so a
   multi-sector 2016-byte slice could not assemble before the player restarted
   its read from sector 0.

Additionally the port harness starts mid-boot, so the CD/DMA register-pointer
globals it reads (`D_8009B32C` … `D_8009B35C`) are not populated unless the
test seeds them. Their retail provenance had to be pinned to show that seeding
is a harness artifact and not a semantic invention.

## 2. Provenance: the CD/DMA pointer tables are static `.rodata`

The pointer globals the movie/CD code dereferences are statically initialized
in the EXE's rodata, not written at runtime. In
`asm/disc1/data/818A0.rodata.s`:

| Symbol | Address | Words (`/ 4`) | Value | Meaning |
| --- | --- | --- | --- | --- |
| `D_8009B32C` | `0x8009B32C` | 2 | `0x1F801800`, `0x1F801801` | CD register base / index |
| `D_8009B334` | `0x8009B334` | 1 | `0x1F801802` | CD parameter/data |
| `D_8009B338` | `0x8009B338` | 1 | `0x1F801803` | CD interrupt enable/flag |
| `D_8009B33C` | `0x8009B33C` | 1 | `0x1F801018` | MDEC command |
| `D_8009B340` | `0x8009B340` | 1 | `0x1F801020` | MDEC control |
| `D_8009B344` | `0x8009B344` | 1 | `0x1F8010F0` | DMA DPCR |
| `D_8009B348` | `0x8009B348` | 1 | `0x1F8010F4` | DMA DICR |
| `D_8009B34C` | `0x8009B34C` | 4 | `0x1F801098` … `0x1F8010A0` | DMA1 CHCR/MADR/… |
| `D_8009B35C` | `0x8009B35C` | 6 | `0x1F8010B8` … `0x1F801074` | DMA3 CHCR/MADR/…, I_STAT/I_MASK |

(rodata offsets `8BB2C`–`8BB70`.) These are `.word` initializers in the
image, so at retail boot the pointers exist before any code runs; the port's
`MOVAU_SeedRegisterPointers` / `B54K` seeding reproduces the boot-time values
in the test harness. No new global was invented.

## 3. XA read modes are served, not gated

`pc_port/platform/pe_cdreg.c` now serves every read mode from the raw sector.
Commands `6` (ReadN) and `27` (ReadS) set `g_device.reading=1` unconditionally;
mode bit 5 selects the 2048/2340-byte FIFO window and the XA-select `0x40`,
double-speed `0x80` and `0x10` bits do **not** change the bytes the drive hands
over. The comment at `pe_cdreg.c:109` records the movie stream's `0x1E0`
open. The remaining `CdDeviceBoundary` sites are unrelated guards
(media change, sector overrun, FIFO underflow, DMA range).

## 4. Stream-wait quantum

`HostFB_StreamTick()` (`pc_port/platform/host_framebuffer.{c,h}`) wraps
`HostFB_VSync(-1)` (device advance + MDEC/SPU/GPU service + DMA IRQ
checkpoint) and adds `3072` cycles of `HostFB_DeviceTime`, so one guest poll
iteration spans the device time an entire multi-sector frame needs. It is used
in the three `func_80121C04` wait loops (`movie_player_start_wait`,
`movie_slice_wait`, `movie_retry_wait`) in
`pc_port/game/boot/movie_overlay_port.c`. `HostFB_StreamTick` is read-only with
respect to guest state beyond the device/IRQ model.

## 5. End-to-end chain under test

`pc_port/tests/test_movie_autonomous.h` (`test_DAY2_movie_autonomous`) runs the
chain twice:

- **Synthetic fixture** — a `DiscFixture` whose searched file
  (`\FMV2\FMV018.STR;1`) carries the retail STR video chunk header shape
  (`raw[24..27] = 60 01 01 80`, chunk index at `+28`, total at `+30`, frame at
  `+32`).
- **Real Disc 1** — `\FMV1\FMV001.STR;1` at LBA `189742`; the test first
  checks `raw[24]==0x60 && raw[25]==1 && (raw[18]&0x20)==0`.

For each run the test asserts:

- `0x8009B374 != 0` — `func_8007C564` ran its assembly states;
- `PE_LoadU32(0x800A3494) == 1` — `func_8007C214` (DMA3 completion) promoted
  the stream record;
- `0x800B0DBA == 4 && 0x800B0DBC == 1` — `func_80121C04` handed off a complete
  assembled frame slice;
- `!PE_Port_ShouldStop()` — no unresolved boundary or stub was hit;
- for the real disc, `B54K_MdecFnv1a64(0x80150800, 2016) ==
  MOVAU_Fnv1a64(raw+56, 2016)`: the 2016-byte slice the player published is the
  on-disc MDEC payload (`raw+56`) byte for byte.

Delivery chain exercised: `70121C04 → 80081314 (CdReadS) → modeled raw-sector
delivery → 7C13C → 80778 → 7F88C → 813E8 → 7C564 (assembly) → DMA3 → 7C214
(promote) → 7C484 → 80121270`.

## 6. Evidence

Build (local toolchain; `cmake -S pc_port -B pc_port/build` is a no-op when the
cache is warm):

```
cmake --build pc_port/build -j12            # all targets built and linked
```

Focused run:

```
PE_TEST_FILTER=DAY2_movie_autonomous ./pc_port/build/pe-native-tests
  autonomous synthetic: stop=0 B374=10 B0DBA=4 B0DBC=1 A3494=00000001
  autonomous disc: stop=0 B374=10 B0DBA=4 B0DBC=1 A3494=00000001 sectors=10
    B89F4=0 BE998=9 BE9E4=9 A8018=0 C0DC0=0 B6918=1
  TEST DAY2_movie_autonomous... PASS
  Results: 1347 run, 1 passed, 0 failed, 1346 skipped
```

Full suite:

```
./pc_port/build/pe-native-tests
  Results: 1353 run, 1353 passed, 0 failed, 0 skipped
```

(The `1347` in the focused-run block above is this report's original run; three
later field-VM opcode ports — `0x97`/`0x9A`/`0x71` — and their tests raised the
full-suite total to the `1353` shown here. Full suite re-run at session end.)

## 7. Non-claims (must not be inferred)

- **XA audio is not decoded.** ReadS mode `0x1E0` is accepted and served; the
  port does not decode or mix interleaved XA audio channels. "XA ReadS bit6 /
  `0x50` handling" here means *the drive does not refuse the read mode*, not
  that XA audio is reproduced.
- **The autonomous scope is the frame initiation path.** `func_80121C04`
  opens the stream, receives the first assembled slice and hands it off; the
  multi-frame updater (`func_80122040`) is covered separately by the existing
  movie tests, not by this run. This is not proof of a full movie playback
  loop.
- **No hardware-exact pixels.** The slice bytes are checked against the disc;
  MDEC decode-to-GPU output is not compared to captured hardware.
- **Boot-time initialization of the pointer globals is not exercised by this
  test.** The values are proven static rodata; the test seeds them because the
  harness starts mid-boot.
- **Full boot→Day-2 is still not demonstrated end to end.** This closes the
  CD/DMA/stream half of the opening-FMV blocker only.
