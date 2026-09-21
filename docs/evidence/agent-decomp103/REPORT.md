# agent-decomp103 — small-function sweep, slice 6 round 2 (ranks 753-1665)

Worktree `/tmp/pe-agent-decomp103`, branch `agent/decomp103`, base `d2ae6879`.

## Baseline gate (reproduced before any change)

- `bash scripts/split_us.sh` (host, not distrobox — splat is host-side) →
  `c: 724 split`.
- `build_us.sh` in `pe-mipsel` → `EXACT SHA-1
  452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
  `Matching claim: YES (724 registered C leaves)`.
- `verify_us.sh` → `VERIFY_US=PASS`
  (plan `1048 spans = 724 c + 322 asm + 2 rodata`).

## Result

**+18 newly matched C leaves: 724 → 742.**

Final fresh build on the final code commit `2df1dbf2`: `Compare: EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (742
registered C leaves)`; `verify_us.sh` → `VERIFY_US=PASS`
(plan `1075 spans = 742 c + 331 asm + 2 rodata`, plan SHA-256
`0b65f992456ed586740c06e5a9cb676c0d02431992b5fabadd296d78e86e1caa`).

Commit range: `d2ae6879..2df1dbf2` (18 leaf commits, one per leaf; each
carries its own regenerated `docs/generated/DISC1_MATCHING_STATUS.md`).

## Matched leaves

Every leaf runs on the **default `era_o2_g0` profile (`-O2 -G0`)** — no new
profile and no `disc1_build_profiles.json` change was needed. All 18 were
transcribed from a correct `endlabel`-bounded disassembly; the triage tool
(`try_leaf.py`) reported `WORDS MATCH` and the full build confirmed it.

| # | leaf | VRAM | file off | size | shape / lever | commit |
|---|------|------|---------:|-----:|---------------|--------|
| 1 | `func_80018774` | 0x80018774 | 0x8F74 | 0x4C | `func_8006F39C(**a0, D_8009D2F0)` then `*a0[1] = ret`, return 1 | `5e733634` |
| 2 | `func_8001A0CC` | 0x8001A0CC | 0xA8CC | 0x48 | double-deref lookup, `*a0[1] = ret << 4`, return 1 | `52ac617a` |
| 3 | `func_8001A114` | 0x8001A114 | 0xA914 | 0x48 | sibling of 2 via `func_80077DC4` | `e57ae422` |
| 4 | `func_80044DCC` | 0x80044DCC | 0x355CC | 0x48 | `func_80062F1C(func_80062A34(1, a0+0x29))`; non-null `func_80062CE4` | `3b409a76` |
| 5 | `func_8004C594` | 0x8004C594 | 0x3CD94 | 0x48 | conditional handle construct + callback store at +0x30 | `41375da5` |
| 6 | `func_8004D690` | 0x8004D690 | 0x3DE90 | 0x44 | `func_8005E8A4(0,2)` then `func_80062A7C(*(a0[1]+0x24)-0x11)` | `18fe1961` |
| 7 | `func_8004FDA4` | 0x8004FDA4 | 0x405A4 | 0x44 | `arg0 == 2 \|\| func_80042770()` short-circuit bool | `4bdfce39` |
| 8 | `func_80050C08` | 0x80050C08 | 0x41408 | 0x48 | ternary arg select `arg0 < 2 ? arg0+0x84 : 0x62` | `588ac7fd` |
| 9 | `func_80050C70` | 0x80050C70 | 0x41470 | 0x44 | `func_80064C54(a0+0x2E)`; `if (func_800527B4()==a0) func_80064C80()` | `27fed3fc` |
| 10 | `func_80050CB4` | 0x80050CB4 | 0x414B4 | 0x44 | sibling of 9 (`+0x31`, `func_80064A48`) | `570db0fd` |
| 11 | `func_80057C10` | 0x80057C10 | 0x48410 | 0x44 | nested guard calls, `bltz` test | `318fccc7` |
| 12 | `func_8006E454` | 0x8006E454 | 0x5EC54 | 0x44 | packed-decimal `(d2-0x30)*100 + (d3-0x30)*10 + (d4-0x30)` | `99f73423` |
| 13 | `func_80082DBC` | 0x80082DBC | 0x735BC | 0x44 | four-call sequence (`3,1` args; `&D_800A5AB0`) | `62a2bfa7` |
| 14 | `func_8008B084` | 0x8008B084 | 0x7B884 | 0x44 | save +0x4/+0x8, constant field init, call | `7d56d545` |
| 15 | `func_8008B124` | 0x8008B124 | 0x7B924 | 0x44 | two out-params (`sp+0x10`/`sp+0x14`) then apply | `844e975d` |
| 16 | `func_8008B580` | 0x8008B580 | 0x7BD80 | 0x38 | clear `D_8009D2A2`, `D_8009D2B4 = a0[2] << 16`, call | `9c215178` |
| 17 | `func_8008F178` | 0x8008F178 | 0x7F978 | 0x38 | store `+0x5A`, then 64-byte-stride table entry + call | `1b58e960` |
| 18 | `func_800C653C` | 0x800C653C | 0xB6D3C | 0x48 | two `func_800C62DC` calls, `a \| b`, second at `+8` | `2df1dbf2` |

## Method notes

- **The predecessor's own extraction recipe under-reports spans.** Slicing
  `glabel+1 .. glabel+1+size/4` from the `.s` misses instructions after an
  interior `.L` label and drops the last 1-3 words of a function. Every
  transcription here used the full `glabel … endlabel` block. This is why
  `func_8006E454`, `func_8008FC78`, `func_800E0024`, `func_80083790`,
  `func_8007C444` and `func_800C811C` looked "close but wrong" on the first
  pass — the missing tail words changed the whole register schedule. A
  missing `*(short *)(a0+0x84) = 1` store, for example, is what skewed
  `func_8008FC78`.
- **Leaves here are almost all "call-wrapper" or "straight-line arithmetic"
  shapes.** With the correct span, a faithful statement-order transcription
  of a `-O2 -G0` function reproduces retail immediately; no `volatile`, no
  register pins and no `aspsx 2.30` were needed. The default profile already
  covers this whole class.
- The slice's tiny end (4-byte `nop` pads, 12-byte `addiu $t2,$zero,0xA0;
  jr $t2; addiu $t1,$zero,N` BIOS trampolines, `lwc2/mtc2/ctc2` GTE stubs and
  `syscall` stubs) is not C-expressible and was skipped, as in 102.

## NEW lever: ASPSX fills the `jr $31` delay slot with the stack restore

`func_800755BC` (0x65DBC, 0x34) and its twin `func_80075AE8` (0x662E8, 0x34)
triage to **exactly 2 differing words**, and the two words are the epilogue:

```
retail                              candidate (cc1 + maspsx)
lw   $ra, 0x14($sp)                 lw   $ra, 0x14($sp)
lw   $s0, 0x10($sp)                 lw   $s0, 0x10($sp)
jr   $ra                            addiu $sp, $sp, 0x18
addiu $sp, $sp, 0x18                jr   $ra
                                    nop
```

cc1 2.7 always emits the stack restore *before* `jr`; retail's ASPSX moved
it **into** the delay slot (and removed the `nop`). This is the same class as
the existing per-leaf `MASPSX_FILL_STORE_DELAY_SLOT` /
`MASPSX_FILL_INDEXED_STORE_DELAY_SLOT` / `MASPSX_FILL_REGISTER_STORE_DELAY_SLOT`
patches, but for `addiu $sp,$sp,N` instead of a store. **It must be a per-leaf
opt-in flag**, because both shapes exist in retail: the matched leaf
`func_80090AAC` (0x812AC) keeps `addiu $sp; jr $ra; nop`.

A patch `MASPSX_FILL_SPRESTORE_DELAY_SLOT=1` that rewrites the sequence
`addiu $sp,$sp,N` / `jr $31` / `nop` → `jr $31` / `addiu $sp,$sp,N` would
close `func_800755BC`, `func_80075AE8`, and the three leaves agent-decomp102
parked for this exact reason: `func_80075B4C` (0x6634C), `func_8007DD74`
(0x6E574), `func_80075C04` (0x66404). **117 of the 913 slice-6 functions
carry this "hard" epilogue shape** (`jr $ra` immediately followed by
`addiu $sp,$sp,-…`), so the lever is worth ~100 leaves across the slice.

## Confirmed next-step candidate (not committed; 18-leaf cap reached)

- **`func_8004FC3C` (0x4043C, 0x44) — `WORDS MATCH`.** Declaring the wrapper
  `int` and writing an explicit `return 0;` after the guarded call makes cc1
  materialise `addu $v0,$zero,$zero` **in the `beqz` delay slot**, exactly
  like retail:
  ```c
  extern int func_80054288(void);
  extern int func_800556E8(int);
  extern int func_8004324C(int);
  int func_8004FC3C(int arg0) {
      if (arg0 < func_80054288()) {
          return func_8004324C(func_800556E8(arg0));
      }
      return 0;
  }
  ```
  A `void` transcription leaves a `nop` there (1 word off). This is a small
  but reusable lever: **an `int` return value of 0 is hoisted into the branch
  delay slot; `void` is not.**

## Parked / not attempted (two-attempt rule; divergences)

| leaf | file/size | divergence |
|------|-----------|------------|
| `func_800755BC` / `func_80075AE8` | 0x65DBC / 0x662E8, 0x34 | 2 words — the `jr $ra` delay-slot stack-restore fill (see NEW lever). Parked pending the maspsx patch. |
| `func_8008B040` | 0x7B840, 0x44 | 4 words — retail keeps the field in `$v1` and materialises `$v0=0` in the `beqz` delay slot, then `addiu $v0,$v1,-1`; cc1 computes the whole `?:` in `$v0`. `int v0=0; if (v1) v0=v1-1;` still leaves `addu $0,$0,$s0`. |
| `func_80090AEC` / `func_80090B5C` | 0x812EC / 0x8135C, 0x44 | 4 words — retail `lbu $v0; bnez $v0,L; addiu $v0,$v0,1; li $v0,0x101` reuses `$v0`; `v = v ? v+1 : 0x101` makes cc1 pick the opposite branch polarity and `$v1` for the result. |
| `func_800525EC` + `…52634`/`…5267C`/`…526C4` | 0x42DEC…, 0x48 ×4 | 14 words — 5th-arg stack store and argument evaluation order: retail sets `a1/a2`, stores the 5th arg at `0x10(sp)`, loads `a0` last, and puts `a3` in the `jal` delay slot; cc1 hoists the `D_800B0E08` load and reorders. |
| `func_8007BBB0` | 0x6C3B0, 0x4C | 5 words — retail puts `lui $at` + `sw $zero,%lo(D_8009AFC4)` in the `jal` delay slot; cc1 emits the store before the call and a `nop` slot. |
| `func_800E0024` | 0xD0824, 0x3C | 6 words — the `(short)d < 0 ? -d : d` abs pattern: retail uses `$a0` with the negated value in `$v1`; cc1 keeps the negated value in `$a0` (`negu $a0,$a0`). |
| `func_80083790` | 0x73F90, 0x38 | 15 words — my first transcription mis-ordered the `(b*5+3)&0xFFC+4` chain; not re-attempted after the corrected-span method succeeded elsewhere. |
| `func_8007C444` | 0x6CC44, 0x34 | 13 words — retail is frameless with a per-iteration `lw D_800C0DC8` and `sll …,5`; cc1 creates an 8-byte frame and strength-reduces the index. |
| `func_8008FC78` | 0x80478, 0x3C | 11 words — after adding the missed `sh 1,0x84(a0)`, retail keeps the pointer in `$v0` and the incremented value in `$v1`; cc1 swaps them. |
| `func_800C811C` | 0xB891C, 0x40 | 9 words — retail stores `0x7F,0x224` between the two symbol loads and ends with `sh $v1,0xC(a2)` in the delay slot; cc1 reorders the symbol loads. |

## Reproduce

```
cd /tmp/pe-agent-decomp103
bash scripts/split_us.sh                                   # host: splat lives outside the container
distrobox enter pe-mipsel -- bash -lc \
  'cd /tmp/pe-agent-decomp103 && bash scripts/build_us.sh && bash scripts/verify_us.sh'
```
