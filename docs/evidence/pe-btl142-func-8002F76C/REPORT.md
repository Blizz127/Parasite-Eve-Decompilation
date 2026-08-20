# PE-BTL142 REPORT — func_8002F76C matching C (27 words)

```text
PE-BTL142 MATCHING_C — func_8002F76C era -O2 -G0, 27/27 words
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x8002F76C..0x8002F7D8 exclusive (0x6C / 27 words)
file        0x1FF6C
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 230
func_8002F76C.c.o .text: 0x70→0x6C
```

Installs three default-record pointers then fires three already-ported
callees. Sits between `2F658` (still asm) and `2F7D8` (0x6F body create).

```
*a0 = &D_800B8A20
D_800B8A88 = &D_800B0CB0
D_800B8A8C = &D_8009D1B0
jal func_8005218C
jal func_80051980(0, D_800B8A88)   /* a0=0 in jal delay */
jal func_80051E64(D_800B8A8C)
```

era `-O2 -G0` (all lui). Unlinked diffs are reloc-only.

## Files

```text
src/func_8002F76C.c
configs/USA/disc1.yaml          [0x1FF6C, c, func_8002F76C]
```
