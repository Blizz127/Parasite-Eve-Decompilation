# func_80030584 — angle helper matching C (17 words)

```text
MATCHING_C — func_80030584 era -O2 -G0, 17/17 words
branch      phase5fm-main-barrier-revisit
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80030584..0x800305C8 exclusive
file        0x20D84 size 0x44
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 233
func_80030584.c.o .text: 0x44
```

`lh` `a0+0xB4`/`+0xB8` shifted `<<16`, subtract `a1[0]` and `a1[2]`
(`lw` 0 / 8), `jal func_80079FB4` with the second `subu` in the delay
slot, then `+2048` truncated to i16 (`sll 16` / `sra 16` with `lw $ra`
between). `addiu $sp` is in the `jr` delay.

Head of former `20D84.s`: C `0x44`, resume `20DC8.s` `0x78` (`func_800305C8`),
then existing `func_80030640`.

## Files

```text
src/func_80030584.c
configs/USA/disc1.yaml          [0x20D84, c, func_80030584]
                                [0x20DC8, asm]
```
