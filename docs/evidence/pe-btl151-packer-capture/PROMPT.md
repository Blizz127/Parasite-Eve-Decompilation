# PE-BTL151 — PCSX packer-call capture prompt

## Objective

Run the authentic Disc 1 `SLUS_006.62` in PCSX-Redux from a fresh boot through
the opening minutes. No save file is required. Do not write guest RAM, force a
destination, or plant persist state.

The purpose is to observe the generic name-to-token path proven in BTL150:

```text
func_8006E3D4 @ 0x8006E3D4
    six-byte input name -> packed token in v0
D_8009D280 @ 0x8009D280
    destination-state writes
```

## Identity

Use the same identity gates as PE-BTL83:

```text
BOOT=cdrom:\SLUS_006.62;1
EXE SHA-1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
BIOS=SCPH-1001 / DTL-H1001 Version 2.0 05/07/95 A
```

The existing BTL83 PCSX Lua harness/API is the reference for
`PCSX.getMemPtr`, `PCSX.getRegisters`, `PCSX.addBreakpoint`, and guest-address
translation. Extend that harness or make a focused sibling; do not invent a
second emulator integration.

## Watches

Arm these watchpoints:

| Signal | Address | Width | Required record |
|---|---:|---:|---|
| packer entry | `0x8006E3D4` | Exec | PC, RA, caller, six input bytes, decoded printable name, register snapshot |
| destination state | `0x8009D280` | Write | PC, RA, old/new token, current field/package state |
| persist[0] | `0x800A77F0` | read/write if supported | value at each destination transition |
| persist[0x4A] | `0x800A7918` | read/write if supported | value at each destination transition |

At packer entry, `$a0` points to the six-byte buffer consumed by the retail
packer. Read exactly six bytes before any callback bookkeeping changes guest
state. Preserve the raw bytes and a printable rendering; non-printable bytes
must be escaped, not discarded.

Record the returned `$v0` by correlating the packer entry with the caller's
return or the subsequent destination write. If the debugger cannot safely arm
dynamic return breakpoints, retain the entry record and correlate by
`(caller RA, next D_8009D280 write)` in the post-processing script. Never infer
a return value from a guessed caller.

## Minimal Lua additions

Use the existing BTL83 CSV/log conventions. Add at least:

```text
PACKER_CALLS.csv
  tick,cycles,pc,ra,caller,a0,name_bytes,name_ascii,v0_at_entry,
  d800a7918,d800a77f0,dest_before

PACKER_RETURNS.csv
  tick,cycles,return_pc,caller,token,dest_after,
  d800a7918,d800a77f0

DEST_WRITES.csv
  retain the existing writer PC/RA and add the nearest packer-call id
```

The Lua must flush each row and write a capture log. It may use a heartbeat,
but the heartbeat is supplementary; event rows are authoritative.

## Human drive

1. Boot the retail image and skip the attract/title sequence.
2. Start New Game, or Continue only if using the project's own card/state.
3. Walk through the opening for roughly five minutes, crossing several field
   boundaries. A live battle is optional.
4. Stop after the log contains at least ten packer calls and several
   `D_8009D280` writes, or after the first stable opening route if fewer occur.
5. Do not use memory editing, scripted destination injection, or third-party
   save files.

## Required analysis

For every captured packer call, classify the six input bytes as:

- literal/static name bytes;
- copied from a table or package record;
- assembled from constants and state;
- numeric field derived from a neighboring token/name;
- unresolved.

Round-trip each recorded input with the BTL150 alphabet and compare the
observed `$v0` to the expected token. Specifically test whether any call
produces `0xA8066048`, and whether the caller derives it from
`0xA80663C8` (`m0367i`) by changing only the fifth 5-bit field.

## BTL151 evidence package

The completed evidence directory should contain:

```text
REPORT.md
PACKER_CALLS.csv
PACKER_RETURNS.csv
DEST_WRITES.csv
HEARTBEAT.csv
capture.log
IDENTITY.txt
```

`REPORT.md` must include image identity, capture route, row counts, the full
packer-call table, caller classifications, any m0360i result, and explicit
negative controls showing that no guest state was written by the harness.

If the opening capture shows only literal names and no computed caller, report
that result; it is still useful generic evidence. If it shows assembled names
or computed fields, preserve the exact caller PC, input bytes, source globals,
and resulting destination write for the next provenance rung.
