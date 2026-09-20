# `func_800254BC` — matching C leaf

Outcome: **MATCHED** and integrated. Leaf count **769 → 770**.

## Span

- VRAM `[0x800254BC,0x80025568)`, file `[0x15CBC,0x15D68)`, size `0xAC` (43 words).
- Carve inside the `125E0` asm run: prefix asm `0x125E0-0x15CBC` (`0x36DC`),
  C `0xAC`, resume asm `0x15D68-0x19B88`.

## Source (`src/func_800254BC.c`)

Unless `D_8009D1A0` bit 1 is set: latch `D_8009D254[0]` into `D_8009D278` and
call `func_8006DE80(0x453, 1, aya->2A, aya->2E, aya->32)` (sign-extended 16-bit
reads). Then a two-way command switch: `0x197` → `func_800218D8()`,
`func_800209F0()`, record byte `+0x12 = 13`; `0x198` → `func_80021AF8()`.

## cc1 flags / maspsx gates

`-O2 -G0` (default profile `era_o2_g0`). No maspsx gate: all four globals are
absolute because the function has no gp-relative access.

## What diverged and how it was fixed

1. **Load-delay scheduling.** Caching the base in a local
   (`unsigned int *aya = D_8009D254;`) made cc1 emit
   `lw $v0,D_8009D254; nop; lw $v1,0($v0); li $a0,0x453`, i.e. a nop in the
   load-delay slot and the `D_8009D278` store too early (one extra word,
   +1 branch-displacement). Writing the accesses directly as
   `D_8009D254[0]` / `((short *)D_8009D254)[21,23,25]` makes cc1 CSE the base
   load and schedule retail's shape: `lui/lw $v0,D_8009D254`, `li $a0,0x453`
   into the load-delay slot, `lw $v1,0($v0)`, the three `lh`s, `li $a1,1`,
   then `lui at` / `sw $v1,D_8009D278` just before the `jal`, with the fifth
   argument in the `jal` delay slot.

2. **Return value.** The `0x197` arm must leave `13` in `$v0` for the byte
   store; plain `*(unsigned char *)(D_8009D278 + 0x12) = 13` with `break`
   produces exactly retail's `li $v0,0xD` / `sb $v0,0x12($v1)`.

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_800254BC.c 0x15CBC 0xAC \
    --flags "-O2 -G0"
=> WORDS MATCH (+4 pad bytes, trimmed by the build)
```

## Cumulative authority (fresh build)

```text
bash scripts/build_us.sh
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (770 generated C entries)
Matching claim: YES (770 registered C leaves)
bash scripts/verify_us.sh => VERIFY_US=PASS ("all 770 packed C spans equal retail")
```
