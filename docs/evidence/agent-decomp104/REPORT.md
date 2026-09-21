# agent-decomp104 — slice 5 medium-function sweep (ranks 425-752)

Worktree `/tmp/pe-agent-decomp104` · branch `agent/decomp104` · base `d2ae6879`.
Scope: worklist ranks 425-752 (328 functions / 101,896 bytes; every body
0xE4-0x1A4, i.e. 57-105 words; 10/328 contain COP2). Stop rule: 12 leaves /
5 parks in a row / 4 h.

## 1. Baseline gate (reproduced before any edit)

`bash scripts/split_us.sh` → `c: 724 split`, then inside
`distrobox enter pe-mipsel`:

```
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (724 registered C leaves)
VERIFY_US=PASS          (7/7 steps)
```

## 2. Final gate (fresh build + verify, final commit)

```
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (736 registered C leaves)
VERIFY_US=PASS
```
Plan `1063 = 736 c + 325 asm + 2 rodata`, plan SHA-256
`15469e6fc0630fb37794b3e68f735c4b43dfeb87bf4b50697e7396dddd6dc8b6`.
Leaf commits land in `d2ae6879..66b65fee`; `func_8002FE78` was reverted in
`7c4c0faf`, `func_800CABC8` added in `9ad3da7a`, the `func_800C8D34` store
order fixed in `2c39410f`, docs in `d2db5710`.

## 3. Newly matched leaves (12)

| # | leaf | VRAM | file | size | lever | commit |
|---|------|------|-----:|-----:|-------|--------|
| 1 | `func_8007A930` | 0x8007A930 | 0x6B130 | 0x104 | **return-type lever** | `354f48de` |
| 2 | `func_80080B44` | 0x80080B44 | 0x71344 | 0x104 | twin of #1 | `8fbbb843` |
| 3 | `func_80019DF4` | 0x80019DF4 | 0xA5F4 | 0x110 | `return 1;` required | `476b6509` |
| 4 | `func_8003DBE4` | 0x8003DBE4 | 0x2E3E4 | 0x124 | `short` index + fresh `a1->table[idx]` per statement | `73a90c6e` |
| 5 | `func_800CABC8` | 0x800CABC8 | 0xBB3C8 | 0x10C | getter/filler template (D_800E27A8) + trailing `0x224` store | `9ad3da7a` |
| 6 | `func_800C7E50` | 0x800C7E50 | 0xB8650 | 0x110 | call `func_80071A54()` inline in the `%` expressions | `d8a7285f` |
| 7 | `func_800C7F60` | 0x800C7F60 | 0xB8760 | 0x104 | outer-pointer temp assigned *between* sum stores 2 and 3 | `6f5b739f` |
| 8 | `func_800C815C` | 0x800C815C | 0xB895C | 0x10C | **aggregate-typed destination parameter** | `cf07ee5f` |
| 9 | `func_800C8D34` | 0x800C8D34 | 0xB9534 | 0x110 | `return 0;` + statement order hoists `$a1=0x80` and orders the 22F0/22F2 stores | `2bb482ce` + `2c39410f` |
| 10 | `func_800C90A4` | 0x800C90A4 | 0xB98A4 | 0x104 | getter/filler template (D_800E27A0) | `cd6abc8e` |
| 11 | `func_800C9D9C` | 0x800C9D9C | 0xBA59C | 0x104 | getter/filler template (D_800E27A4) | `b22679bf` |
| 12 | `func_800CA934` | 0x800CA934 | 0xBB134 | 0x104 | getter/filler template (D_800E27A8) | `66b65fee` |

`func_800C7E50` sets up three values from `func_80071A54() % 3/5` and calls
`func_80078C34`. The `func_800C7F60`/`90A4`/`9D9C`/`A934`/`CABC8` family loads
an outer pointer global, calls
`func_80078C34(OUTER->p + 0x260, table + (short)(n-1)*8, &buf)`, sums three
global halfwords with the returned buffer into the destination and copies an
8-word aggregate. `func_80080B44` is instruction-identical to
`func_8007A930`.

### NEW LEVERS (all proven on this slice)

1. **Return type/value selects register coloring.** `func_8007A930`/`B44`
   transcribed as `void` gave a word-for-word identical instruction sequence
   with *rotated* coloring (magic1→`$v0`, magic2→`$v1`, out→`$a1` vs retail
   magic1→`$v1`, magic2→`$a1`, out→`$v0`). Declaring `unsigned char *` and
   `return arg1;` makes era cc1 copy the output pointer into `$v0` early
   (retail's `addu $v0,$a1,$zero` at +0x10), freeing `$a1` for the
   `0x88888889` magic → byte-exact. Same mechanism for `func_80019DF4`
   (`return 1;`) and `func_800C8D34` (`return 0;`). Try it first when the
   instruction sequence matches and only the register coloring is rotated.
2. **Aggregate-typed destination parameter is a coloring/scheduling lever.**
   In `func_800C815C`, `unsigned char *a2` made cc1 reload `D_800E279C` into
   `$v0` after the halfword store and emit an extra load-delay nop (29-word
   skew); typing the parameter `struct Sprite *a2` with the exact field layout
   flipped the reload to `$a0` and scheduled it into the `lhu` slot.
3. **Pointer-temp placement between stores controls base-register lifetime.**
   `func_800C7F60` needed `op = D_800E279C;` inserted *between* the 2nd and
   3rd sum stores; only that position keeps `D_800E279C` live across the 3rd
   `sh` so the allocator colors it `$a0` and sched2 fills the load-delay slot.
   Before store #2 or after store #3 both break it.
4. **Call-in-`%` preservation.** In `func_800C7E50`, binding
   `func_80071A54()` to an `int` local inserts a `$v0 → $a0` copy; keeping the
   call inline in the `%` expression preserves retail's `mult $v0,$s1`.
5. **Bitfield union for `srl`+`andi` same-register coloring.** In
   `func_8002FE78`, retail `srl a2,v0,9; andi a2,a2,1` (result reg on the
   `srl`) only appears when the field is accessed as a bitfield union member;
   an explicit `(x >> 9) & 1` gives `srl v0,v0,9; andi a2,v0,1`. (The leaf
   itself was parked in the full build — see below.)
6. **Jump-table leaves need `ERA_ASPSX_VER=2.30`** (or
   `MASPSX_PASSTHROUGH_SYMBOL_LOAD=1`) to expand the indexed symbol load as
   `lui $at / addu $at,$at,$v0 / lw $v0,%lo($at)` (3-word) instead of the
   4-word form; without it the whole function shifts by one word.
7. **Statement order selects both constant hoisting and store order**
   (`func_800C8D34`: moving `D_800E22F0 = 0;` up next to
   `D_800E22F2 = 0x80;` made cc1 hoist `$a1=0x80` into retail's slot *and*
   re-emit the two halfword stores in retail's order).

**Triage caveat (important).** `tools/analysis/try_leaf.py` zeroes relocation
words, so two *different* symbol-store instructions that differ only in the
symbol/immediate (e.g. `sh $a1,%lo(D_800E22F2)` vs
`sh $zero,%lo(D_800E22F0)`) compare equal. `func_800C8D34` (swapped store pair)
and `func_8002FE78` (switch-table placement) both printed `WORDS MATCH` yet
failed the full build. For symbol-store-heavy leaves, confirm with
`scripts/build_us.sh`, or compare the object's reloc-symbol sequence to the
retail instruction/symbol sequence.

## 4. Parked / not attempted

| leaf | file/size | divergence |
|------|-----------|------------|
| `func_8002FE78` | 0x20678 / 0x100 | try_leaf matched byte-exact under `ERA_ASPSX_VER=2.30` (jump-table 3-word symbol load + bitfield union), but the **full build** failed at +0x20: retail's 23-entry switch table lives at `0x80010AC8` (file 0x12C8, inside the first text unit), so the `lui $at,%hi`/`%lo` resolve to 0x8001 there. A standalone C leaf emits the table in its own object's rodata and cannot place it at file 0x12C8, so the carve was reverted (commit `7c4c0faf`). |
| `func_80078CC4` | 0x694C4 / 0x138 | retail reads packed words with `lw` + `andi 0xFFFF; sll 16; sra 16` and multiplies with **`multu`** (funct 0x19); era cc1 folds `(short)(x & 0xFFFF)` into a sign-extending `lh` and emits `mult` (funct 0x18) — never register-register `multu; mflo`. ~12 source shapes + full flag matrix produced no candidate. Semantics: unrolled 9-short 16.16 scale, `m[i] = (short)(((unsigned)(short)m[i] * scale[i%3]) >> 12)`, packed 2 shorts/word; already ported in `pc_port/game/boot/func_800CEE20_port.c`. |
| `func_800C6FA0` | 0xB77A0 / 0xF8 | scratchpad volume scaler; retail keeps MMIO base `0x1F800000` + offset 0x30 and a second pointer `q = p+2` (`-1(q)`/`0(q)`); cc1 either folds the full `0x1F800030` constant at offset 0 or rotates registers. 46-62 words differ. |
| `func_8008B698` | 0x7BE98 / 0xE8 | two-way record scan; cc1 strength-reduces the two record bases into `(p-0x1C, p)` where retail keeps one base `p` with offsets `0/-0x1C/-0x80/-0xC8`. 53 words differ. |
| `func_80019170` | 0x9970 / 0xF0 | list-search predicate; loop rotation + `a0/a2` copy skew (a sibling of the unmatched `func_80018080`). 36-52 words differ. |
| `func_8007D074`, `func_800625B8`, `func_80048F24`, `func_80056B24`, `func_8008068C` | — | inspected and judged out of quick reach (`$gp`-relative globals, `$at`-absolute store blocks, indirect `jalr`). |

## 5. Reproduce

```
cd /tmp/pe-agent-decomp104
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp104 && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp104 && bash scripts/verify_us.sh'
```
