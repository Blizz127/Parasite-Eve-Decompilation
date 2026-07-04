# Disc information — Parasite Eve (USA / NTSC-U)

Primary target release. All hashes below marked **TODO** must be filled in
during Phase 1 from a user-supplied dump and cross-checked against redump.

## Disc 1 — SLUS-00662 (verified 2026-07-04)

All values below were produced by `scripts/extract_us.sh 1` (which runs
`tools/verify/hashfile.py`, `tools/extract/psxiso.py`, and
`tools/analysis/psxexe_info.py`) against a user-supplied bin+cue dump.

### Image

| Field | Value |
| --- | --- |
| Serial | SLUS-00662 |
| Region | NTSC-U |
| Cue layout | single track: `TRACK 01 MODE2/2352` |
| Image size | 495,531,120 bytes (210,685 × 2352-byte sectors) |
| Image CRC-32 | `488641e0` |
| Image MD5 | `efae1df4faf4aadbfd4dc3aa022296cf` |
| Image SHA-1 | `c339455d5b1dae04f77c2ee847d0932adaf2e84b` |
| ISO9660 system id | `PLAYSTATION` |
| ISO9660 volume space | 210,685 sectors (equals image sector count) |

**Redump cross-check: PENDING.** redump.org (disc page
http://redump.org/disc/116/) was unreachable (ECONNREFUSED) on 2026-07-04.
The hashes above are computed facts about the local dump; whether they match
redump's Track 1 values is still unverified. Re-check when redump.org is
reachable and record the result here.

### Main executable — `SLUS_006.62`

Extracted from LBA 24 of the ISO9660 filesystem. `SYSTEM.CNF` confirms it is
the boot executable (contents, CRLF stripped):

```text
BOOT=cdrom:\SLUS_006.62;1
TCB=4
EVENT=16
STACK=801fff00
```

| Field | Value |
| --- | --- |
| Size | 2,025,472 bytes |
| CRC-32 | `7c10c01c` |
| MD5 | `cb095240a2ba358b8fdcbfd4d4f97f04` |
| SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| pc0 (entry point) | `0x80072534` |
| gp0 | `0x00000000` |
| t_addr (load address) | `0x80010000` |
| t_size | `0x1EE000` (2,023,424 = file size − 2048 header ✓) |
| s_addr | `0x801FFFF0` |
| region marker | `Sony Computer Entertainment Inc. for North America area` |

### Filesystem (25 files, from `tools/extract/psxiso.py list`)

```text
    LBA       size  path
     23         60  SYSTEM.CNF;1
     24    2025472  SLUS_006.62;1
   1013  206213120  PE.IMG;1
 101704         40  FMV1/PEDISC01.IDF;1
 101705    3074048  FMV1/FMV000.STR;1
 103206   20955136  FMV1/FMV002.STR;1
 113438   24150016  FMV1/FMV003.STR;1
 125230   12025856  FMV1/FMV004.STR;1
 131102    5259264  FMV1/FMV005.STR;1
 133670   15908864  FMV1/FMV006.STR;1
 141438    4374528  FMV1/FMV006C.STR;1
 143574    9601024  FMV1/FMV007.STR;1
 148262    4718592  FMV1/FMV008.STR;1
 150566    5799936  FMV1/FMV009.STR;1
 153398   10567680  FMV1/FMV010.STR;1
 158558   11763712  FMV1/FMV011.STR;1
 164302    7913472  FMV1/FMV012.STR;1
 168166    8208384  FMV1/FMV013.STR;1
 172174    9256960  FMV1/FMV014.STR;1
 176694    7487488  FMV1/FMV015.STR;1
 180350    2523136  FMV1/FMV016A.STR;1
 181582    2392064  FMV1/FMV016B.STR;1
 182750    9863168  FMV1/FMV017A.STR;1
 187566    4456448  FMV1/FMV017B.STR;1
 189742   42584064  FMV1/FMV001.STR;1
```

Structural observation (verified from the listing above): apart from the
boot files and FMV streams, **all game data lives in a single 206 MB packed
archive `PE.IMG`** starting at LBA 1013. Its internal format is a later
research task (Phase 5+); do not extract or commit its contents.

## Disc 2 — SLUS-00668 (not yet verified)

| Field | Value |
| --- | --- |
| Serial | SLUS-00668 |
| Region | NTSC-U |
| Main executable | `SLUS_006.68` |
| Image SHA-1 (redump) | TODO (Phase 1 — run `scripts/extract_us.sh 2` once the image is under `rom/image/`) |
| EXE SHA-1 | TODO (Phase 1) |
| EXE size / load address / entry point | TODO (Phase 1, from PS-X EXE header) |
| Filesystem listing | TODO (Phase 1) |

## Other releases (out of scope for now)

- Japan: SLPS-01291 (disc 1), SLPS-01292 (disc 2); Squaresoft Millennium
  Collection and PSone Books reprints exist.
- No PAL release.

## Rules

- Never commit images or extracted files; only record facts (hashes,
  offsets, names) here.
- Every value in this file must state how it was obtained (tool + command).
