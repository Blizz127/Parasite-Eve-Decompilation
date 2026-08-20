# PE-BTL145 REPORT — func_800305C8 matching C (30 words)

```text
PE-BTL145 MATCHING_C — func_800305C8 era -O2 -G0, 30/30 words
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x800305C8..0x80030640 exclusive
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 235
func_800305C8.c.o .text: 0x80→0x78
```

This is `func_8001F814`'s angle callee. `ratan2` of actor +0x28/+0x30
deltas, `2048 - result`, add target+0x3A, wrap into [0,4096).
`register int ang asm("$3")` pins ROM's `$v1` coloring.

1F814 itself remains NONMATCHING_C: its jump table lives in the `0x800`
rodata pool at `0x800106E4`.

## Files

```text
src/func_800305C8.c
configs/USA/disc1.yaml          [0x20DC8, c, func_800305C8]
```
