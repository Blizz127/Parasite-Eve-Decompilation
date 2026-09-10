# Movie autonomous stream (stage 158)

Stage158 closes the recorded `CD_device_read_mode` frontier for movie
mode 0x1E0 and drives the physical delivery chain end-to-end on a
synthetic one-chunk STR sector.

## Provenance: DMA pointer table

`D_8009B32C..D_8009B35C` are pre-initialized in the retail Disc1 EXE image
(same provenance as `D_8009B27C..D_8009B28C`). Values match the gameover
fade EXE seed dump:

| Symbol offset | Value | Role |
| --- | --- | --- |
| 9B32C/330/334/338 | 1F801800..803 | CD index/request |
| 9B33C | 1F801018 | CD bus control |
| 9B340 | 1F801020 | result mailbox |
| 9B344/348 | 1F8010F0/F4 | DPCR/DICR |
| 9B34C | 1F801098 | DMA1 CHCR (StreamOutputChcr) |
| 9B35C | 1F8010B8 | DMA3 CHCR |

They are not written by SDK CD init or the movie path at runtime. Native
tests must re-plant them after `ResetTestState` zeroes RAM;
`B558_PlantPointers` now owns that seed set.

## Device mode bits

- Bit6 `CdlModeRT`: allowed. Drive delivers all raw sectors; `7C564`
  filters on magic `0x160` and channel.
- Bit4 `CdlModeSM`: still `CD_device_read_mode` (XA filter not modeled).
- Bit5 `CdlModeSize1` / bit7 speed: unchanged (2340-byte FIFO from raw+12,
  double-speed period).

## Player host adaptations

- First-frame success returns 1 (oracle graph).
- `HostFB_VSync(-1)` inside the 121270 poll so device/IRQ/DMA progress
  during the spin (hardware parallelism stand-in; updater already did this).

## Explicit non-claims

Not multi-sector STR frames, not retail opening-movie pixel goldens, not
14E30, not XA filter/audio, not Day1/Day2 scope completion.
