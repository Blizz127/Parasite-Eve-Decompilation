# CD XA / CdlModeRT read-mode coverage

Stage158 allows Psy-Q `CdlModeRT` (mode bit6 / `0x40`) on the mounted-disc
device model and documents the retail provenance of the stream/DMA register
pointer table that `7C564` / `7CEAC` dereference.

## Provenance: who writes `D_8009B32C..D_8009B35C`?

Not CdInit and not the movie path. The SHA-1-exact Disc 1 EXE image
pre-initializes the companion pointer table in `.data`, the same way it
seeds `D_8009B27C..B28C` (WIRE report). Values pinned by the gameover-fade
EXE seed cases and planted for harness resets by `B558_PlantPointers` /
`CdDeviceSeed`:

| Symbol | EXE initializer |
| --- | --- |
| `D_8009B32C` | `0x1F801800` (CD REG0) |
| `D_8009B334` | `0x1F801802` (CD REG2 / data) |
| `D_8009B338` | `0x1F801803` (CD REG3) |
| `D_8009B33C` | `0x1F801018` (bus control) |
| `D_8009B340` | `0x1F801020` (mailbox) |
| `D_8009B344` | `0x1F8010F0` (DPCR) |
| `D_8009B348` | `0x1F8010F4` (DICR) |
| `D_8009B34C` | `0x1F801098` (DMA1 CHCR / MDEC) |
| `D_8009B35C` | `0x1F8010B8` (DMA3 CHCR) |

Production `PE_GuestImage_LoadExe` loads the same words from the retail
image. Tests that `ResetTestState` must re-plant them through
`B558_PlantPointers`.

Public khasinski/parasite-eve-decomp `dma_execute` notes independently
treat `D_8009B32C` / `B344` / `B348` as hardware pointer views for
`7CEAC`; adopted here only as corroboration of the EXE-seed model, not
as source to copy.

## Device mode policy

| Bit | Name | Model behavior |
| --- | --- | --- |
| 5 (`0x20`) | Size1 | 2340-byte FIFO from raw offset 12 (already supported) |
| 6 (`0x40`) | CdlModeRT | **Allowed** — deliver raw sectors; `7C564` filters |
| 7 (`0x80`) | Speed | Double-speed cadence (already supported) |
| 4 (`0x10`) | Size0 | **Boundary** `CD_device_read_mode` (still unimplemented) |

Movie `81314(..., 0x1E0)` sends low byte `0xE0` = Size1|RT|Speed. The
previous `mode & 0x50` guard rejected RT; it is now `mode & 0x10`.

## Tests

- `DAY2_cd_sector_device`: mode `0xE0` delivers a full Size1 FIFO; mode
  `0x10` still stops at `CD_device_read_mode`.
- `DAY2_cd_xa_stream`: after `CdDeviceSeed` plants the EXE table, opens
  `81314(..., 0x1E0)` on a crafted STR sector and asserts autonomous
  `device → IRQ → 7C564 → 7C214 → record status 2` with payload/location
  bytes (no manual `7C564` pump).
- `DAY2_movie_player`: enabled path now reaches `movie_retry_wait`
  (fixture `PE.IMG` is not a STR stream); callback + pointer-table
  assertions remain.

XA channel filter / ADPCM audio, `14E30` real path, libpress wait
fidelity, and opening→Day2 acceptance remain open. Runtime 128 unchanged.
