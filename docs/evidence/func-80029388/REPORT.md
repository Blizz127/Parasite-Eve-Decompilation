# REPORT — func_80029388 matching C (27 words), 270 leaves

```text
MATCHING_C — func_80029388 era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80029388..0x800293F4 exclusive (0x6C / 27 words)
file        0x19B88
```

Slot-table clear + default-record init wrapper, pulled from the
pe-continuous-decomp PE-BTL141 evidence
(`~/dev/pe-continuous-decomp/docs/evidence/pe-btl141-func-80029388/`).

## Body

1. Frame `addiu $sp,-24` / `sw $ra,16($sp)`
2. `jal func_8002F658` (default-record init; still asm, now in `19BF4.s`)
3. Loop `i = 0..6` (`unsigned char`, `andi 0xFF`, `sltiu 7`): the same
   220-byte `SlotRecord` in-use clear as `func_8002F9CC`,
   `sw $zero, D_800A5D58($v1)` via 3-word `lui $at` / `addu` / `%lo`
4. `sb $zero, 0x530($gp)` = D_8009D2A0
5. `sb $zero, 0x57C($gp)` = D_8009D2EC
6. `jal func_80020EFC` (already C)

Back-branch delay slot is FILLED (`andi`), inverse of 2F9CC's nop —
slot fill is per-shape, not per-table (5FE note). `-G8` for the two gp
byte clears; 3-word knob for the indexed symbol store.

## Carve

Unlike the source repo (where `func_800293F4` is also C), this repo
keeps 293F4 in asm, so the mid-`11718` carve is: prefix `11718.s`
0x8470, C 0x6C, resume `19BF4.s` 0x63E4 to 0x1FFD8.

## Verification (2026-08-21)

```text
scripts/split_us.sh    # c: 270 split
docker run --rm -v "$PWD:/workspace" -w /workspace \
  --user "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh
# RESULT: EXACT MATCH
# Compare:  EXACT SHA-1 MATCH
sha1sum build/disc1.candidate.exe
# 452fb033f2eaa4b18aa20a5bca60b8125af3a37b (fresh, built 14:04 CDT)
```

The `pe-mipsel-img` docker image was rebuilt from `dev/mipsel/Dockerfile`
first — the previous image predated the Dockerfile's `python3` install and
aborted `build_us.sh` at the maspsx step. `build_us.sh` now `rm -f`s
`build/disc1.candidate.exe` at start so a stale candidate can never be
mistaken for a fresh build.

## Files

```text
src/func_80029388.c
configs/USA/disc1.yaml    [0x19B88, c, func_80029388] + [0x19BF4, asm]
scripts/build_us.sh       era -O2 -G8 + 3W store; 11718/19BF4 sizes, link order
```
