# PE-BTL141 REPORT — func_80029388 matching C (27 words)

```text
PE-BTL141 MATCHING_C — func_80029388 era -O2 -G8 + 3W store, 27/27 words
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80029388..0x800293F4 exclusive (0x6C / 27 words)
file        0x19B88
```

Immediate predecessor of matched `func_800293F4`. Native audit lived in
`pc_port/game/boot/func_80029388_port.c` (Phase 6E-B8).

## Body

1. Frame `addiu $sp,-24` / `sw $ra,16($sp)`
2. `jal func_8002F658` (default-record init; still asm)
3. `move $a0,$zero` — incoming a0 discarded
4. Loop `i = 0..6` (`unsigned char`, `andi 0xFF`, `sltiu 7`):
   same 220-byte strength-reduction as `func_8002F9CC`
   `sw $zero, D_800A5D58($v1)` via 3-word `lui $at` / `addu` / `%lo`
5. `sb $zero, 0x530($gp)` = D_8009D2A0
6. `sb $zero, 0x57C($gp)` = D_8009D2EC
7. `jal func_80020EFC` (already C)
8. Epilogue

Back-branch delay slot is FILLED (`andi`), inverse of 2F9CC's nop —
slot fill is per-shape, not per-table (5FE note).

Same `SlotRecord` typing as 2F9CC/2F970. `-G8` for the two gp byte
clears; 3-word knob for the indexed symbol store.

```text
scripts/build_us.sh
# RESULT: EXACT MATCH
# SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
# yaml C entries: 229
# func_80029388.c.o .text: 0x70→0x6C
```

```text
scripts/build_us.sh
# RESULT: EXACT MATCH
# SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
# yaml C entries: 229
# func_80029388.c.o .text: 0x70→0x6C
```

## Files

```text
src/func_80029388.c
configs/USA/disc1.yaml          [0x19B88, c, func_80029388]
scripts/build_us.sh             era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1
```
