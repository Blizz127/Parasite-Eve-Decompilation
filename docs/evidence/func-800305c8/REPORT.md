# func_800305C8 — angle-wrap helper matching C (30 words)

```text
MATCHING_C — func_800305C8 era -O2 -G0, 30/30 words
branch      phase5fm-main-barrier-revisit
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x800305C8..0x80030640 exclusive
file        0x20DC8 size 0x78
scripts/build_us.sh RESULT: EXACT MATCH
scripts/verify_us.sh RESULT: EXACT MATCH
yaml C entries: 234
func_800305C8.c.o .text: 0x78
```

The leaf is non-leaf with a 0x18-byte frame, `$s0` holding the second
record pointer, and `$ra` saved at `0x14($sp)`. It loads the two record
coordinate pairs at `+0x28`/`+0x30`, calls `func_80079FB4` with the second
subtraction in the delay slot, truncates `2048 - ratan2` to signed i16,
adds the signed halfword at `+0x3A`, and wraps by the exact negative-path
`+0xFFF` and 12-bit shift sequence before the final signed-i16 truncation.
Register pins for `$v0`/`$v1` preserve the retail allocation through the
branch and final subtraction.

The former `20DC8.s` chunk is now split as C `0x78` at `func_800305C8`,
resuming at `20E40.s` for the existing `func_80030640` leaf.

## Files

```text
src/func_800305C8.c
configs/USA/disc1.yaml          [0x20DC8, c, func_800305C8]
scripts/build_us.sh              C registration / era compile / ROM order
scripts/verify_us.sh             expected boundary and 234-leaf report
```
