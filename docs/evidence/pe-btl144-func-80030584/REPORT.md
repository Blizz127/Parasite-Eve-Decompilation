# PE-BTL144 REPORT — func_80030584 matching C (17 words)

```text
PE-BTL144 MATCHING_C — func_80030584 era -O2 -G0, 17/17 words
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80030584..0x800305C8 exclusive
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 234
func_80030584.c.o .text: 0x50→0x44
```

`ratan2`-style helper: load two shorts at actor+0xB4/+0xB8, shift 16,
subtract `a1[0]` / `a1[2]`, `jal func_80079FB4`, add 2048, return as
i16. Adjacent to `func_800305C8` (1F814's angle callee, still asm).

## Files

```text
src/func_80030584.c
configs/USA/disc1.yaml          [0x20D84, c, func_80030584]
```
