# `func_80021D4C` — matching C leaf

Outcome: **MATCHED** and integrated. Leaf count **768 → 769**.

## Span

- VRAM `[0x80021D4C,0x80021DE0)`, file `[0x1254C,0x125E0)`, size `0x94` (37 words).
- Carve inside the `120D8` asm run: prefix asm `0x120D8-0x1254C` (`0x474`),
  C `0x94`, resume asm `0x125E0-0x19B88`.
  `0x474 + 0x94 + 0x7A8 = 0xCAE`… equivalently the previous single span
  `0x120D8-0x19B88` is now three spans.

## Source (`src/func_80021D4C.c`)

Command-cancellation walk: while the 8-bit index `D_8009D1D4` is below the
8-bit command count `D_8009CE3C`, read the 16-bit action at
`D_800BE834[index]` (stride 8), and for `action-3 < 0x180` sign-extend and call
`func_80053D2C(action-3)`. Then clear both bytes and drop bit 8 of
`D_8009D1A0`.

## cc1 flags / maspsx gates

```
-O2 -G8
MASPSX_THREE_WORD_SYMBOL_STORE=1
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D1A0
```
Profile `era_o2_g8_three_word_force_d8009d1a0_absolute` in
`configs/USA/disc1_build_profiles.json`.

## What diverged and how it was fixed

1. **Table addressing.** The natural `unsigned short D_800BE834[]` index form
   made cc1 hoist `la $17,D_800BE834` into the loop preheader (extra callee-save,
   `0x20` frame). Retail addresses the table as `lui $at,%hi` / `addu $at,$at,$idx`
   / `lhu $a0,%lo($at)` — the three-word indexed-symbol load. Turning on
   `MASPSX_THREE_WORD_SYMBOL_STORE=1` (the gate is extended to indexed loads)
   reproduces `lui`/`addu`/`lhu %lo` and drops the hoist.

2. **gp-relative vs absolute words.** Retail keeps `D_8009CE3C` / `D_8009D1D4`
   gp-relative (`lbu $s0,0x464($gp)`) but `D_8009D1A0` absolute
   (`lui $v0,%hi` / `lw $v0,%lo($v0)`). `-G8` makes the two bytes small-data
   (gp-relative); forcing `D_8009D1A0` absolute strips its `.extern …,4` so
   maspsx leaves the direct `lw`/`sw` pseudo for gas (2-word lui/op), matching
   retail. This is the same lever recorded for `func_80021054`.

3. **Loop rotation.** `while ((index & 0xFF) < D_8009CE3C) { … }` produced a
   top-tested loop (`j` at the back edge). Retail rotates the test to the bottom
   with the entry jumping to it, and reloads the count outside the test block.
   Writing the loop as an explicit `goto test` / `body` shape with the count in
   a reloaded local reproduced retail exactly: entry `lbu index`, `lbu count`,
   `j test`, count re-read after the call immediately before the shared test.

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_80021D4C.c 0x1254C 0x94 \
    --flags "-O2 -G8" --env MASPSX_THREE_WORD_SYMBOL_STORE=1 \
    --force-absolute D_8009D1A0
=> WORDS MATCH (+12 pad bytes, trimmed by the build)
```

## Cumulative authority (fresh build)

```text
bash scripts/build_us.sh
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (769 generated C entries)
Matching claim: YES (769 registered C leaves)
bash scripts/verify_us.sh => VERIFY_US=PASS ("all 769 packed C spans equal retail")
```
