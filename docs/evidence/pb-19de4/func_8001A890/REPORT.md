# func_8001A890 — bootstrap state clear (768 → 769)

- VRAM `0x8001A890`, file `0xB090`, size `0x88` (34 words), unit `ACAC`.
- Branch `agent/pb-19de4`, worktree `/tmp/pe-agent-pb19de4`.
- Fresh-build evidence: `scripts/build_us.sh` prints
  `RESULT: EXACT MATCH`, candidate SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; `scripts/verify_us.sh`
  prints `VERIFY_US=PASS` and `matching-C count: 769 (from YAML)`.

## Offset math

YAML span before the carve was one `[0xACAC, asm]` run continuing to
`[0x116FC, c, func_80020EFC]`. The carve splits it as

```
[0xACAC, asm]              prefix, 0xACAC..0xB090
[0xB090, c, func_8001A890] leaf,   0xB090..0xB118 (0x88)
[0xB118, asm]              resume, 0xB118..0x116FC
```

No other span moved; the plan geometry still closes at `0x1EE000`.

## Compiler profile

`era_o2_g8_three_word`: era `gcc-2.7.2-psx` with `-O2 -G8`, maspsx 2.21
`--dont-expand-li`, environment `MASPSX_THREE_WORD_SYMBOL_STORE=1`.
Registered in `configs/USA/disc1_build_profiles.json` under
`assignments.era_o2_g8_three_word`.

`-G8` is required for the run of `sw/sh $zero,NNN($gp)` scalar clears; the
two indexed tables and `D_8009DFB0` stay absolute (`lui`/`la`).

## What diverged and how it was fixed

The first draft produced a 4-word loop store instead of retail's 3-word
form. Raw cc1 already emits the correct compact line:

```asm
sh  $0,D_8009CE0C($3)
```

but maspsx's default ASPSX 2.21 `addiu_at` path rewrote it to

```asm
lui   $at,%hi(D_8009CE0C)
addiu $at,$at,%lo(D_8009CE0C)
addu  $at,$at,$3
sh    $0,0x0($at)
```

Retail keeps the ASPSX 2.30 three-word shape:

```asm
lui  $at,%hi(D_8009CE0C)
addu $at,$at,$3
sh   $0,%lo(D_8009CE0C)($at)
```

Setting `MASPSX_THREE_WORD_SYMBOL_STORE=1` selects exactly that branch.
The second loop (`D_8009DFB0`, 0x14 words) uses an absolute `la`, which is
unaffected by the gate.

Source shape that reproduces cc1's induction variable (`$3 += 4`,
`sltu $2,$3,8`) is a two-dimensional `short [][2]` array indexed
`D_8009CE0C[i][0]`; the byte-offset strength reduction then matches with
no extra `sll`.

## Authoritative checks

```
$ bash scripts/split_us.sh
disc1 plan: 1113 spans (769 c, 342 asm, 2 rodata), geometry=0x1EE000
$ bash scripts/build_us.sh
RESULT: EXACT MATCH
cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (769 registered C leaves)
$ bash scripts/verify_us.sh
VERIFY_US=PASS
```
