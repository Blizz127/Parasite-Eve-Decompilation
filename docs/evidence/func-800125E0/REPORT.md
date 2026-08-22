# func_800125E0 — descriptor spawn loop matching C (35 words)

```text
MATCHING_C — func_800125E0 era -O2 -G8, 35/35 words
branch      merge/leaves-into-grind
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x800125E0..0x8001266C exclusive
file        0x2DE0 size 0x8C
scripts/split_us.sh RESULT: c: 276 split
scripts/build_us.sh RESULT: EXACT MATCH
scripts/verify_us.sh RESULT: EXACT MATCH
yaml C entries: 276
func_800125E0.c.o .text: 0x90→0x8C (trim)
```

The leaf calls `func_80074DC0(0)` (DrawSync), then walks the relocated
descriptor list at `D_8009CE04` (`gp+0x94`). Byte 0 of `*list` is the
unsigned count; each two-byte descriptor starts at offset 1. Each
iteration calls `func_80035038(desc + offset, 0, 1)`, reloads the list
pointer into `$a0`, increments the count, and adds 2 to the offset in
the `bnez` delay slot.

## Second attempt

The first attempt (`*D_8009CE04` with no local, no pins) was size-correct
(`0x8C`) but not exact: frame `0x20` vs retail `0x28`, `$s0`/`$s1`
swapped (offset vs count), and an extra gp reload at loop top because
the header lived in `$v0` rather than `$a0`.

A local `p = D_8009CE04` restores the retail `$a0` carry. Natural
if+do/while still emits `vars=0` and `$s0=offset` / `$s1=count`.
`register` pins keep `$s0=count` / `$s1=offset`. An unused `int` with
an empty `asm volatile("" : : "m"(unused))` forces the 8-byte vars home
(frame `0x28`) and emits no instructions. Linked span
`0x800125E0..0x8001266C` is byte-identical to retail.

## Files

```text
src/func_800125E0.c
configs/USA/disc1.yaml          [0x2D74, asm], [0x2DE0, c, func_800125E0], [0x2E6C, asm]
scripts/build_us.sh              C registration / era -O2 -G8 / ROM order
scripts/verify_us.sh             expected boundary and 276-leaf report
docs/ai_context/ACTIVE_HANDOFF.md
```
