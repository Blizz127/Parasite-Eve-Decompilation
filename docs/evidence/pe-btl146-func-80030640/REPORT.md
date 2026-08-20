# PE-BTL146 REPORT — func_80030640 matching C (40 words)

```text
PE-BTL146 MATCHING_C — func_80030640 era -O2 -G0, 40/40 words
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80030640..0x800306E0 exclusive
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 236
func_80030640.c.o .text: 0xA0
```

RNG gate over `D_8009D278`: if `*(rec+0x68)+0x10` has bit 16 set and
`func_80071A54() % 100` is signed-less than `lhu rec+0x22`, store 9000
at `rec+0x10`.

The leftover `andi 1` was the wrong mask: ROM `lui $v1,1` / `and` is
`0x10000`. The second `D_8009D278` load is `$v1` because the signed
`%100` expansion clobbers `$a0`; C reloads into a separate `rec2`.

Non-leaf: frame `-0x18`, `$ra` + `$s0` (threshold).

## Files

```text
src/func_80030640.c
configs/USA/disc1.yaml          [0x20E40, c, func_80030640]
                                [0x20EE0, asm]
```
