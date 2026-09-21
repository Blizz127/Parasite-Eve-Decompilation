# Semantics audit — on-path matched leaves (independent re-derivation)

Status: 2026-09-11 (verification/gates lane). This audit answers a different
question from the byte/link sweeps: **does the C *mean* what retail means?**
Byte-exact compilation does not prove semantic 1:1 (a reversed ternary, a
swapped operand order, a wrong mask, or a re-associated arithmetic chain can all
still compile to the same bytes).

It extends the earlier 11-leaf audit (session cont. 7) to a **29-row batch**
prioritized for the persistent objective: boot → end of Day 2, highest on-path
fan-in, and largest semantic surface (comparison polarity, operand order, bit
constants, signedness/narrowing, loop bounds, pointer lifetimes, return types,
struct shapes, and multiple-C-spellings-compile-identically leaves).

> **Numeric reconciliation (2026-09-11).** This file's table has **29** named
> CONFIRMED rows. Earlier prose here said "31" and "42"; the extra 11 from
> "session cont. 7" are recorded nowhere and cannot be attributed to named
> leaves, so they are excluded. The single authoritative, machine-checkable
> reconciled list (29 + Shard A + Shard B) is `SEMANTICS_LEDGER.md`.

## Method

For each leaf:

1. Read `src/<leaf>.c` and its YAML span (`configs/USA/disc1.yaml`).
2. Disassemble the **retail** EXE at the YAML VMA
   (`mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL` over
   `build/extracted/disc1/SLUS_006.62`, offset `0x800 + vram - 0x80010000`).
3. Re-derive the behavior independently, then compare the C's meaning — not its
   bytes — field by field.
4. Resolve every referenced `D_*`/`func_*` symbol to the address the retail
   instruction actually touches (the repo convention encodes the address in the
   name, so a mismatch is detectable by inspection).
5. Record the verdict.

Verdicts: **CONFIRMED** (C implements the retail behavior), **DEFECT** (C is
semantically wrong), **UNCERTAIN** (cannot determine from available evidence).

This audit is read-only with respect to `src/` and `configs/USA/disc1.yaml`
(owned by the matching worker). Any DEFECT would be reported, not fixed.

## Audit table

| leaf | span (words) | on-path fan-in | verdict | key evidence |
|---|---|---:|---|---|
| `func_80086FF8` | 11 | 8 | **CONFIRMED** | stores `-1`↔`0xF0` (`li v0,240`) into `D_800BCD80` at `0x800BCD80`, then `jal func_8008CBA8`; frame `-0x18`, `ra` at `0x10`. C exact. |
| `func_80087024` | 11 | 8 | **CONFIRMED** | twin of above with `241` = `0xF1`; same symbol address, same frame. C exact. |
| `func_80085084` | 5 | 7 | **CONFIRMED** | `lui v1,0xB0BE` / `ori v1,v1,0xB4BF` / `lw v0,0(a0)` / `addu v0,v0,v1`. C `*a0 + 0xB0BEB4BFu` exact; `unsigned` avoids a sign-extension. |
| `func_800703F4` | 10 | 7 | **CONFIRMED** | `jal 0x702DC` then `jal 0x701B4`, in that order. C call order exact. |
| `func_80062CC4` | 3 | 6 | **CONFIRMED** | `lw v0,1004(gp)` → `0x8009CD70+0x3EC = 0x8009D15C`. C returns `D_8009D15C`. |
| `func_8007FBF0` | 6 | 6 | **CONFIRMED** | `sll a0,2` / `lui v0,0x800A` / `addu` / `lw v0,-19084(v0)` → `0x800A3600 - 0x4A8C`? (address-named `D_8009B574`); C `D_8009B574[a0]` exact. |
| `func_8002F9CC` | 17 | 6 | **CONFIRMED** | stride chain `andi 0xFF` / `8i-i` / `56i-i` / `*4` = **220·i**; store `sw zero,D_800A5D58(at)` with the symbol+register expansion; bound `sltiu 7`. C `SlotRecord[7]` 220-byte aggregate + `unsigned char i` exact. |
| `func_80075424` | 48 | 5 | **CONFIRMED** | `lbu D_8009574E`; `sltiu 2` gate → `D_80095748(&D_8001193C, arg)`; `s0 = a0+0x1C`; `lw v0,0x1C(s1)` OR `0xFFFFFF` → store; `D_80095744+8` handler called with `(v1[6], s0, 0x40, 0)`; `func_80071A34(s2+0xE, s1, 0x5C)`. C exact (handler arg order verified). |
| `func_800752AC` | 43 | 5 | **CONFIRMED** | `lbu D_8009574E` gate → `D_80095748(&D_80011910, a0, a1)`; `D_80095744+0x2C` = `(v0+11)` handler `(a0,a1)`; `D_8009580C = (D_800957F8 & 0xFFFFFF) | 0x4000000`; `*a0 = D_8009580C & 0xFFFFFF`. C exact. |
| `func_800718D0` | 29 | 5 | **CONFIRMED** | `lw v0,4(a0)` / `andi 8` / `beqz`; taken: `lw v0,8(a0)`, `s0=a0+8`, `v0=s0+v0` (tail `j`); not-taken: `v0=a0+8`; `s1=v0+0xC`; `jal 0x7506C(v0+4, s1)`; `beqz s0` → second `jal 0x7506C(s0+4, s0+0xC)`; returns `s1`. C exact (returns `v0+3` words = `+0xC` bytes). |
| `func_8007F72C` | 19 | 6 | **CONFIRMED** | `func_8007FBF0(0)`; `li v0,1` / `bne s0,v0` → else `move v0,s0`; taken `func_8007F778()` / `blez` → else `li s0,2` / `move v0,s0`. C `if (s0 == 1 && func_8007F778() > 0) s0 = 2;` exact. |
| `func_80073C94` | 12 | 5 | **CONFIRMED** | `D_8009566C` pointer global; `lw v0,0xC(v0)`; frame `-0x18` with `ra` at `0x10`; `jalr v0` via `$v0`; restore; `jr ra`. C `*(unsigned int (**)())(D_8009566C + 0xC)` exact. |
| `func_80073CF4` | 12 | 5 | **CONFIRMED** | same shape, `lw v0,4(v0)` (`D_8009566C[1]`). C exact. |
| `func_80026FD0` | 10 | 5 | **CONFIRMED** | `lb v0,D_8009D2B0` (signed) / `beqz`; delay slot `li v0,0x80`; `sb v0,248(gp)` = `D_8009CE68 = 0x80`; `li v0,-8` / `sb v0,252(gp)` = `D_8009CE6C = -8`. C exact incl. the `signed char` type (which produces `lb`). |
| `func_8007F7A8` | 8 | 5 | **CONFIRMED** | `jal 0x7FCAC` with frame `-0x18`, `ra` at `0x10`, return value forwarded. C `return func_8007FCAC();` exact. |
| `func_8004D024` | 3 | 5 | **CONFIRMED** | `sw a0,652(gp)` → `D_8009CFFC = a0`; `jr ra`. C exact. |
| `func_80051E58` | 3 | 5 | **CONFIRMED** | `lw v0,680(gp)` → `D_8009D018`; `jr ra`. C exact. |
| `func_80030534` | 20 | 6 | **CONFIRMED** | `lh v0,0x268(a0)`/`lh v1,0x2A(a1)`/`subu`/`mult`; second `lh 0x26C(a0)`/`lh 0x32(a1)`/`subu`/`mult`; `mflo`s; `addu a0,a2,v1`; `jal 0x5186C`. C's `short` fields and `dx*dx + dz*dz` exact (offsets 0x268/0x2A/0x26C/0x32). |
| `func_8005DC4C` | 20 | 5 | **CONFIRMED** | `D_800A802C` word + `&D_800A8028`; `rec+*(int*)(rec+4)`; `lhu 0(v1)`; `sltu a0,v0` (unsigned bound); `sll v0,a0,1`; `lh v0,2(v0+v1)` (signed); `addu v0,v1,v0` (relative). C exact, incl. the unsigned `a0` comparison and signed 16-bit offset. |
| `func_80077CB4` | 14 | 5 | **CONFIRMED** | `lbu` head[3], tail[3]; `addu`; `addiu +1`; `slti v1,17` (signed) → `beqz`; taken: `sb v1,3(a0)`, tail `j` with `sw zero,0(a1)` in the delay slot; else `li v0,-1`. C exact. |
| `func_800850F4` | 16 | 7 | **CONFIRMED** | saves `a0`→`s0`, `a1`→`s1`; `jal 0x850C0` then `jal 0x85E54(s0,s1)`. C arg order exact. |
| `func_8006E6D4` | 69 | 6 | **CONFIRMED** | `lui v1,0x100`/`and`/`bnez` → `-1`; `func_8007F72C()==1`; `func_8007F778()==0`; `func_8007F7A8()==D_800B0DD4` (`lhu` at `0x800B0DD4`) else `func_800719E4(1)`; OR `0x01004000`; `lba += off`; `func_80080B44(lba, sp+16)`; `func_80080E34(&loc, sectors, dest, 0x80)`; `bnez` → that; AND `0xFEFFBFFF`; `func_80071A74(D_8001136C, lba, sectors)`; `-1`. C exact. |
| `func_800811E4` | 28 | 6 | **CONFIRMED** | `func_80073A44(-1)`; base `0x8009B6C4` in `$a0`; `lw v1,0(a0)`; `addiu v1,1200`; `slt v1,v0` (signed vsync compare); taken `func_80081268()` + `-1`; else `lw s0,-16(a0)` = `0x8009B6B4`; `func_8007F608(p)`; return `s0`. C `base[0] + 1200 < vsync` incl. the `issue`/`pending` struct offsets (`+0`/`-16`) exact. |
| `func_8006DED4` | 31 | 4 | **CONFIRMED** | 5th/6th args read from `sp+72`/`sp+76` (`unsigned short`); `sh`s to `sp+24/26/28`; `func_8006DFA8(buf, &v0, &v1)`; `lw v0,36(sp)`/`lw a3,32(sp)`; `func_8006DF50(a0,a1,a2,v0,v1)`. C exact (arg widths and stack layout). |
| `func_8006FC18` | 127 | 7 | **CONFIRMED** | `sltiu 22` (unsigned) → `-0x16`; `sltiu 11` split; arena strides `0xA0C` (ids <0xB) and `0x10C` (ids ≥0xB); `lbu p[0]` vs `0`/`6`; `bnez a2` else `lw p+8 == a1`; `lbu p[1]`; `sltiu 0xC0` → `-0x17`; `sltiu 0x55` clamp; `lw D_800942E0[4*h]` → `-0x18` if null; `lw +0x14` → `-1` if null; `jalr`; second arena recompute; `lbu q[1] == 0x72`; 7-word `D_800E10A0` clear (`i=0x6C..0x72`); `D_800B0CD8 &= 0xFFFEFFFF`; `q[0]=0`, `q[1..3]=0xFF`, `*(int*)(q+4)=0`, `*(int*)(q+8)=0`; return `a2`. All constants/masks exact. |
| `func_8006E834` | 91 | 4 | **CONFIRMED** | six `sb -1` to `D_800B0DB5/B4/B7/B6/B3/B2`; `li -241` = `&~0xF0` on `D_800B0CD8` (note: `~0xF0` = `0xFF0F`, and `li v1,-241` = `0xFFFFFF0F`); `jal 0x86FF8`; read loop `func_8006E6D4(D_800B0DD8 + tbl[0], 0, D_80011614, tbl[1]-tbl[0])` retried while `== -1`; poll: `move v1,v0`, `addiu v0,v1,1`, `sltiu v0,v0,2` (unsigned `(t+1)<2`, i.e. `t ∈ {-1,0}`), `beqz`; true → `D_800B0CD8 &= 0xFEFFBFFF`; `move v0,v1`; `beqz v0` → break; `beq v0,-1` → retry; else `j 0x6E8E0`; then `func_80072714/726C4/72724()`, `func_80073A44(0)`, `func_80074D28(0)`, `func_800749D8(sp+24,0,0,320,240)` (5th arg on stack), `sb 1,sp+41` (`env[0x11]`), `func_800755F0(sp+24)`, return 0. C exact. |
| `func_8006A8D4` | 68 | 4 | **CONFIRMED** | The whole `D_800B0E24..E6C` boot memory-region table is reproduced word for word: each store's value is the register holding the running cursor/`next`, in retail order; `li v1,0xE000`, `li a0,0x8000`, `lui a1,0x4`/`ori 0x8000` = `0x48000`, `lw v1,D_80011614`, `addiu v0,v1,-8`, `sw v1,D_800B0E68`. All 19 stores' forms and order match the C's `cursor`/`next` sequence. |
| `func_800124F8` | 31 | 4 | **CONFIRMED** | `D_8009D300=0` (`sw zero,1424(gp)`), `D_8009D308=0` (`sh`), `D_8009CDFC=0`; outer loop `sltiu 72` = `0x48`, inner `sltiu 11` = `0x0B`, stride `addiu a2,44` = `0x2C`; `D_8009DF70` loop `sltiu 16`, `addiu v1,4`; `D_8009CE04=0` (`sw zero,148(gp)`). C constants exact (`unsigned int` for `sltiu`). |
| `func_800293F4` | 124 | 7 | **CONFIRMED** | `lh +0x1C` (max_hp), `lh +0x0C` (cur_hp), `slt` guard, `sh` clamp; re-loads `D_8009D278` into different registers (`v1`, `a1`) per retail's separate `lw`s; `sb 4,+0x12`; `lhu +0x0C`; `D_8009D1D0=0`; `sh 0,+0x10`; `sw 0,+0x34`; `sh hp,+0x0E`; `(a0&0xFF)==1` branch: `sb 0x5A,D_8009D234`, `D_8009D244=1`, `lw +0x4C` OR `0x800000`; else `D_8009D244=0`, AND `~0x400000`/`~0x800000`; store `+0x4C`; then `sb 0,+0x56`, 21 sequential flag clears on `+0x4C`, `r4[0x5E]=0`, `r5[0x66]=0`; `D_8009D2E8 &= ~0x10`; `jal 0x21D4C`, `jal 0x374E8`; `sh 0,D_8009D298`, `sb 0,D_8009D29A`, `sb 0,D_8009D29B`, `sw 0,D_8009D29C`. Masks and call order exact. |

## Result

- **29 / 29 CONFIRMED**, **0 DEFECT**, **0 UNCERTAIN**.
- All 29 are, independently, byte/link-exact at their declared VMA (the
  full-image strong sweep covers them), so they move from "byte-confirmed" to
  "byte-confirmed + semantics-confirmed".

## Semantics-confirmed vs byte-confirmed (cumulative)

| tier | count | basis |
|---|---:|---|
| matched C leaves at the plan measured here | 750 | `configs/USA/disc1.yaml` `c` spans (plan `7ede9199b817…`) |
| byte/link/size/boundary verified | **750 / 750** | full-image strong sweep (`verify_matched_leaves.py`), `sources_stable: true` |
| **semantics-confirmed (this file's 29 named rows)** | **29** | see `SEMANTICS_LEDGER.md` for the reconciled cross-file total |
| semantics-left (byte-only) | 721 | not yet independently re-derived |

The honest statement: **an exact rebuild + a clean strong sweep proves the C
compiles to retail bytes; it does not prove the C is 1:1 in meaning.** 29 leaves
are independently re-derived in this file; the reconciled cross-file total
(including Shard A and Shard B) is in `SEMANTICS_LEDGER.md`. The rest are
byte-confirmed only.

## What would resolve the remaining surface

The 29 leaves here were chosen for maximum semantic surface (comparison
polarity, operand order, constants, signedness, loop bounds, pointer/struct
shape). The remaining risk concentrates in leaves where multiple C spellings
compile identically (e.g. `>=` vs `>` on provably equal values, re-associated
integer sums, struct vs flat-array aliases). Those need the same per-leaf
disassembly re-derivation; a cheap next batch is the highest-fan-in remaining
`c` leaves not yet covered.

## Environment note

All checks ran with `MASPSX_FORCE_ABSOLUTE_SYMBOLS` and `MASPSX_SYMBOL_AT_TEMP`
stripped from the environment (the recorded per-leaf profiles carry the knobs
they need). The deep preflight / sweep additionally strip leaked `MASPSX_*` /
`ERA_*` automatically, because a leaked knob silently changes codegen.
