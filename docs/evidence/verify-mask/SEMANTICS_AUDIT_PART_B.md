# Semantics audit — PART B (Shard B: second half of the remaining matched C leaves)

Status: 2026-09-11 (verification lane, adversarial semantic re-derivation).
This is the sibling of `SEMANTICS_AUDIT.md`. It covers **Shard B**: the second
half (by VMA) of the matched `c` spans in `configs/USA/disc1.yaml` after
removing every leaf already **CONFIRMED** in `SEMANTICS_AUDIT.md`.

Partition (deterministic, disjoint from the sibling auditor):
`configs/USA/disc1.yaml` `c` spans sorted by VMA → remove the 29 table rows
recorded CONFIRMED in `SEMANTICS_AUDIT.md` → 721 remain → Shard B is indices
`[ceil(721/2), 721) = [361, 721)` = **360 leaves**, first VMA `0x80071944`,
last VMA `0x800D4850`. The sibling owns the first 361.

**All 360 Shard-B leaves are audited here**, including all 100 on-path leaves.

> **Baseline discrepancy found (report to the owner of `SEMANTICS_AUDIT.md`).**
> That file's table has **29** rows, but its prose claims "42" semantics-confirmed
> (11 from a prior session + 31). The 11-leaf session total is not recorded as a
> list anywhere in the repo. The 29 named rows are all removed *before* the
> split, so **none** can appear in either shard, and Shard B is unaffected
> either way; but the real semantics-confirmed baseline is
> **29 + (the unrecorded 11)**, and the prose "42" should be re-derived. This
> audit therefore reports both a conservative and an upper-bound total below.

## Method

For each leaf:

1. Read `src/<leaf>.c` and its YAML span (`configs/USA/disc1.yaml`, read-only).
2. Disassemble the **retail** EXE at the YAML VMA
   (`mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL` over
   `build/extracted/disc1/SLUS_006.62`, file offset `0x800 + vram - 0x80010000`).
3. Re-derive the behavior independently and compare the C's **meaning** (not
   its bytes): branch polarity, comparison operand order/signedness, bit
   constants, narrowing/widening, loop bounds and sentinels, pointer lifetimes
   and base registers, return type/value, struct shape/strides, and every
   referenced symbol's address.
4. For every `D_*` symbol, confirm the name-derived address equals the address
   the retail instruction actually touches.

Verdicts: **CONFIRMED**, **DEFECT**, **UNCERTAIN**.

This audit is read-only with respect to `src/` and `configs/USA/disc1.yaml`.
Any DEFECT is reported for the owning worker, not fixed.

## Audit table

| leaf | VMA | span (words) | verdict | key evidence |
|---|---:|---:|---|---|
| `func_80073CC4` | 0x80073CC4 | 12 | **CONFIRMED** | `lui/lw D_8009566C` (`0x8009566C`), `lw 8(v0)`, `jalr` via `$v0`; C `*(f**)(D_8009566C+8)` argument-less fn-ptr exact. |
| `func_80073D24` | 0x80073D24 | 13 | **CONFIRMED** | `lw 0x14(v0)`; `a1=a0` before frame; `li a0,4`; `jalr v0` with `(4, a0)` in delay slot. C `f(4,a0)` exact (arg order). |
| `func_80073D58` | 0x80073D58 | 12 | **CONFIRMED** | `D_8009566C+0x14` argument-less `jalr`; C exact. |
| `func_80073DE8` | 0x80073DE8 | 4 | **CONFIRMED** | `lhu D_800945E6` (`0x800945E6`), return; C `unsigned short` getter exact. |
| `func_80073E10` | 0x80073E10 | 6 | **CONFIRMED** | `lhu 0(v1)`, `sh a0,0(v1)` on `D_80095674`; C exchange returns old. `sh` + `lhu` = 16-bit value exact. |
| `func_800749D8` | 0x800749D8 | 15 | **CONFIRMED** | Store order `0,2,4,6(h from sp+0x10),8,0xA,0xC,0xE`, then `sb 0x11,0x10,0x13,0x12`; C struct field order (`x,y,w,h,sx,sy,sw,sh,isinter,isrgb24,pad0,pad1`) reproduces exactly. h read from caller stack at `sp+0x10` (5th arg). |
| `func_80074A28` | 0x80074A28 | 4 | **CONFIRMED** | `lw D_800956EC`; C `int` getter exact. |
| `func_80074D28` | 0x80074D28 | 38 | **CONFIRMED** | State gate `lbu D_8009574E`/`sltiu 2`; log `D_80095748(&D_80011870, a0)`; `s0==0` → `func_80077A28(s1+0x6A,-1,0x14)`; `D_80095744+0x10` handler with `0x3000000`/`0x3000001` (delay-slot `ori`); C ternary order exact, `s0`/`s1` pins match retail `$16`/`$17`. |
| `func_80074DC0` | 0x80074DC0 | 26 | **CONFIRMED** | Gate `D_8009574E>=2` → `D_80095748(&D_80011884,a0)`; then `D_80095744+0x3C` handler with `a0`; C exact. |
| `func_8007506C` | 0x8007506C | 24 | **CONFIRMED** | `func_80074E28(&D_800118D4,a0)`; `D_80095744` base `$v0`; `lw a0,0x20(v0)`, `lw v0,8(v0)`, call `(v0[0x20/4], a0, 8, a1)`; C `v0[8]`, handler `v0+2` exact. |
| `func_800750CC` | 0x800750CC | 24 | **CONFIRMED** | Twin of 7506C: `&D_800118E0`, arg word `+0x1C`; C `v0[7]` exact. |
| `func_80075358` | 0x80075358 | 23 | **CONFIRMED** | `D_80095744+0x3C` handler called `(0)`; then reload `D_80095744`, `+0x14` handler with `(a0+4, a0[3])` (lbu). C exact incl. reload and `unsigned char s1`. |
| `func_800753B4` | 0x800753B4 | 28 | **CONFIRMED** | Gate `D_8009574E>=2` → `D_80095748(&D_80011928,a0)`; `D_80095744+8` handler `(v0[6],a0,0,0)`; C exact. |
| `func_800754E4` | 0x800754E4 | 54 | **CONFIRMED** | Gate/log `D_80011954` `(s2,s1)`; `func_80075EE0(s1+0x1C,s1)`; element-form 24-bit splice `((int*)s1)[7]=(...&0xFF000000)|(s2&0xFFFFFF)`; `D_80095744+8` handler `(v0[6],s0,0x40,0)`; `func_80071A34(s3+0xE,s1,0x5C)`; C exact (operand order row7 load/store, `s0` arg). |
| `func_80075B4C` | 0x80075B4C | 14 | **CONFIRMED** | `a0[3]=2`; `*(int*)(a0+4)=func_800762BC(a1)` (a1 passed in `$a0`); `*(int*)(a0+8)=0`; C exact. |
| `func_80076150` | 0x80076150 | 8 | **CONFIRMED** | `0xE1000000` base in `$v1`; `arg1`→`|0x200`; `arg0`→`|0x400`; `arg2&0x9FF`; final `or`. C full expression identical (both flags, 0x9FF). |
| `func_800762A0` | 0x800762A0 | 7 | **CONFIRMED** | `y=(a1&0x7FF)<<11` computed first (holds `$a1`), then `x=(a0&0x7FF)|0xE5000000`; return `y|x`. C operand order exact (load-bearing per REPORT). |
| `func_8007633C` | 0x8007633C | 6 | **CONFIRMED** | `lw D_80095854` then `lw 0(v0)`; C `*D_80095854` exact. |
| `func_80076B20` | 0x80076B20 | 9 | **CONFIRMED** | `*D_80095854=a0`; `srl v0,a0,24`; `sb a0,D_800A3348(v0)` (indexed store via `$at`). C `D_800A3348[a0>>24]=a0` exact — `unsigned` index selected `srl`, and the stored value is the **full word** truncated to `sb`. |
| `func_80076B44` | 0x80076B44 | 5 | **CONFIRMED** | `lui v0,0x800A`; `addu v0,v0,a0`; `lbu %lo(D_800A3348)(v0)`; C `D_800A3348[a0]` 8-bit getter exact. |
| `func_80076B58` | 0x80076B58 | 16 | **CONFIRMED** | `*D_80095854=0x4000000`; if a1 → do/while `c=a1-1; c--` copying `*a0`→`*D_80095850`; return 0. C loop bound/sentinel (`c!=-1`) exact; `D_80095850` reloaded each iter as retail does. |
| `func_80076B98` | 0x80076B98 | 18 | **CONFIRMED** | `*D_80095854=0x4000002` (`lui 0x400; ori 2`); `D_80095858=a0`; `D_8009585C=0`; `D_80095860=0x1000401`; C four stores/constants exact. |
| `func_80076BE0` | 0x80076BE0 | 12 | **CONFIRMED** | `*D_80095854 = a0|0x10000000`; return `*D_80095850 & 0xFFFFFF`; C exact (`0x10000000` not `0x01000000`). |
| `func_80076C10` | 0x80076C10 | 9 | **CONFIRMED** | args: `a3=a2`, `a2=0`, `jal 0x76C34`; C `func_80076C34(a0,a1,0,arg2)` exact (4th arg wins, 3rd zeroed). |
| `func_80077A28` | 0x80077A28 | 9 | **CONFIRMED** | Byte memset down-count loop `sb a1,0(a0)`, bound `c = n-1; --c != -1`; record-asm pins for `$v0`/`$v1`; C exact. |
| `func_80077A64` | 0x80077A64 | 15 | **CONFIRMED** | `(tp&3)<<7 | (abr&3)<<5 | ((y&0x100)>>4) | ((x&0x3FF)>>6) | ((y&0x200)<<2)`. Retail uses `sra` for the `0x100`/`0x3FF` shifts; C's operands are `int` and non-negative after masking, so `sra`==`srl` here; constants/bit positions exact. |
| `func_80077AA4` | 0x80077AA4 | 6 | **CONFIRMED** | `(a1<<6) | ((a0>>4)&0x3F)`, then `&0xFFFF`; C exact. `sra` vs `srl` immaterial for the masked result. |
| `func_80077AC4` | 0x80077AC4 | 15 | **CONFIRMED** | `*a1 = (*a1 & 0xFF000000)|(*a0 & 0xFFFFFF)`; then `*a0 = (*a0 & 0xFF000000)|(((unsigned)a1) & 0xFFFFFF)` (retail masks the **pointer value**, not `*a1`). C reproduces both the element-form OR and retail's odd pointer-mask exactly; constants pinned `$6`/`$7`. |
| `func_80077B04` | 0x80077B04 | 10 | **CONFIRMED** | `lbu 7(a0)`; `a1!=0` → `|2`; else `&=0xFD`; `sb`; C exact incl. mask `0xFD` (bit1). |
| `func_80077B34` | 0x80077B34 | 10 | **CONFIRMED** | Twin of 77B04 for bit0: `|1` / `&=0xFE`; C exact. |
| `func_80077B64` | 0x80077B64 | 5 | **CONFIRMED** | `a0[3]=4`, `a0[7]=32` (0x20); C exact. |
| `func_80077BA4` | 0x80077BA4 | 5 | **CONFIRMED** | `a0[3]=9`, `a0[7]=44` (0x2C); C exact. |
| `func_80077BC4` | 0x80077BC4 | 5 | **CONFIRMED** | `a0[3]=8`, `a0[7]=56` (0x38); C exact. |
| `func_80077C04` | 0x80077C04 | 5 | **CONFIRMED** | `a0[3]=4`, `a0[7]=100` (0x64); C exact. |
| `func_80077C44` | 0x80077C44 | 5 | **CONFIRMED** | `a0[3]=3`, `a0[7]=96` (0x60); C exact. |
| `func_80077C64` | 0x80077C64 | 5 | **CONFIRMED** | `a0[3]=3`, `a0[7]=64` (0x40); C exact. |
| `func_80077C84` | 0x80077C84 | 11 | **CONFIRMED** | `a0[3]=1`; `0xE1000000 | (arg2?0x200:0) | (arg1?0x400:0) | (arg3&0x9FF)`; C expression and `$2`/`$3` pins exact — note `arg1`→0x400 and `arg2`→0x200, matching `func_80076150`'s arg mapping. |
| `func_80077CF4` | 0x80077CF4 | 15 | **CONFIRMED** | `if (a0<0) return -func_80077D30((-a0)&0xFFF); else func_80077D30(a0&0xFFF)`; C exact (signed compare, negate, mask 0xFFF). |
| `func_80077D30` | 0x80077D30 | 36 | **CONFIRMED** | Piecewise sine table: `<0x801`→`<0x401 ? D_8009589C[a0] : D_8009589C[0x800-a0]`; `<0xC01`→`-D_8009489C[a0]`; else `-D_8009589C[0x1000-a0]`. C symbols/offsets/negations exact (all `lh` signed). |
| `func_80077DC4` | 0x80077DC4 | 40 | **CONFIRMED** | Normalize `a0<0`→negate; `&=0xFFF`; `<0x801`→`<0x401 ? D_8009589C[0x400-a0] : -D_8009509C[a0]`; `<0xC01`→`-D_8009589C[0xC00-a0]`; else `D_8009409C[a0]`. C exact incl. which table/negation each arm uses. |
| `func_80078C94` | 0x80078C94 | 9 | **CONFIRMED** | Three `lw` from `a1` (`t0/t1/t2` pins `$8/$9/$10`) stored to `a0+0x14/0x18/0x1C`; return `a0`; C exact. |
| `func_8007D054` | 0x8007D054 | 8 | **CONFIRMED** | `li a0,0; jal func_8007D074`; C exact. |
| `func_8007DC5C` | 0x8007DC5C | 10 | **CONFIRMED** | `*D_8009B410 = (*D_8009B410 & 0xF0FFFFFF) | 0x20000000`; C exact (`lui 0x2000`). |
| `func_8007DC84` | 0x8007DC84 | 10 | **CONFIRMED** | Same with `| 0x22000000`; C exact. |
| `func_8007DD14` | 0x8007DD14 | 9 | **CONFIRMED** | `li a0,4; a1=value; jal func_80073CF4`; C `func_80073CF4(4,value)` exact. |
| `func_8007DEB0` | 0x8007DEB0 | 4 | **CONFIRMED** | `lw D_8009B4AC`; C `int` getter exact. |
| `func_8007E594` | 0x8007E594 | 12 | **CONFIRMED** | `*(u32*)a0=0`; `a0[4]=0`; down-count `i=3..0` clearing `q[5]` with `q=a0+3` then `q--`; then `+0xC/0x10/0x14` words = 0. C loop direction/pointer exact (parallel `q` load-bearing). |
| `func_8007F778` | 0x8007F778 | 4 | **CONFIRMED** | `lw D_800A3608`; C `int` getter exact. |
| `func_8007F788` | 0x8007F788 | 8 | **CONFIRMED** | `jal func_8007FC54`, return `$v0&0xFF`; C `unsigned char` return exact. |
| `func_8007FBC0` | 0x8007FBC0 | 3 | **CONFIRMED** | `lui $at; jr; sw a0,%lo(D_800A36A0)`; C callback setter exact. |
| `func_8007FBCC` | 0x8007FBCC | 3 | **CONFIRMED** | `sw a0,D_800A36A4`; C callback setter exact. |
| `func_8007FBD8` | 0x8007FBD8 | 3 | **CONFIRMED** | `sw a0,D_800A36A8`; C callback setter exact. |
| `func_8007FBE4` | 0x8007FBE4 | 3 | **CONFIRMED** | `sw a0,D_800A36AC`; C callback setter exact. |
| `func_8007FC28` | 0x8007FC28 | 3 | **CONFIRMED** | `lui v0,%hi(D_8009B582); jr; addiu %lo`; C `&D_8009B582` exact. |
| `func_8007FC54` | 0x8007FC54 | 4 | **CONFIRMED** | `lbu D_8009B56C`; C `unsigned char` getter exact. |
| `func_8007FC64` | 0x8007FC64 | 9 | **CONFIRMED** | `li a0,1; a1=arg0; jal func_8007B010`; C `func_8007B010(1,result)` exact. |
| `func_8007FC88` | 0x8007FC88 | 9 | **CONFIRMED** | `li a0,1; a1=arg0; jal func_8007B290`; C exact. |
| `func_8007FCAC` | 0x8007FCAC | 4 | **CONFIRMED** | `lw D_8009B590`; C `int` getter exact. |
| `func_80080930` | 0x80080930 | 4 | **CONFIRMED** | `li v0,1; sw D_8009B554`; C exact. |
| `func_80080940` | 0x80080940 | 4 | **CONFIRMED** | `lw D_8009B554`; C `int` getter exact. |
| `func_80080950` | 0x80080950 | 18 | **CONFIRMED** | Both non-null → copy exactly 4 bytes (`slti v1,4`); `a1==0` → `*a0=0`; C structure/bounds exact. |
| `func_80080998` | 0x80080998 | 18 | **CONFIRMED** | Twin of 800950 with 8-byte copy (`slti v1,8`); C exact. |
| `func_80080CC8` | 0x80080CC8 | 5 | **CONFIRMED** | Exchange `D_8009AFC0`, return old; C `int` (readers use `blez`/`slti`) exact. |
| `func_800812F4` | 0x800812F4 | 7 | **CONFIRMED** | `sltiu a0,2` unsigned gate → `sw D_8009B6B8`; C `value < 2` on `unsigned` exact. |
| `func_800816F4` | 0x800816F4 | 8 | **CONFIRMED** | `jal func_80071A04(left,right,12)`; return `sltiu $v0,1` = `result==0`; C exact (size 12, `== 0`). |
| `func_80081E5C` | 0x80081E5C | 5 | **CONFIRMED** | Exchange `D_8009B708` via `$3` pointer; C exact. |
| `func_800822AC` | 0x800822AC | 4 | **CONFIRMED** | `lw D_8009B70C`; C `int` getter exact. |
| `func_800824C8` | 0x800824C8 | 5 | **CONFIRMED** | Exchange `D_800B8AB4` via `$3`; C exact. |
| `func_800824DC` | 0x800824DC | 5 | **CONFIRMED** | Exchange `D_800B8AB8` via `$3`; C exact (same global as `func_8007F960` reads, consistent `void(*)(int)` reader). |
| `func_800824F0` | 0x800824F0 | 9 | **CONFIRMED** | `li a0,3; a1=value; jal func_80073CF4`; C exact. |
| `func_80082534` | 0x80082534 | 8 | **CONFIRMED** | `jal func_80082CF0`; C exact. |
| `func_800835A4` | 0x800835A4 | 3 | **CONFIRMED** | `sw a1,0x28(a0)`; `sb a2,0x34(a0)`; C exact. |
| `func_80083E50` | 0x80083E50 | 8 | **CONFIRMED** | `a0[0x36]=0x43`; `*(char**)(a0+0x2C)=a0+0x24`; `a0[0x24]=a1`; `a0[0x35]=1`; C exact. |
| `func_80083E70` | 0x80083E70 | 5 | **CONFIRMED** | `a0[0x36]=0x45`; `*(u32*)(a0+0x2C)=0`; `a0[0x35]=0`; C exact. |
| `func_80083E84` | 0x80083E84 | 8 | **CONFIRMED** | `a0[0x36]=0x4C`; same pointer/arg/flag shape as 83E50; C exact. |
| `func_80083EC4` | 0x80083EC4 | 8 | **CONFIRMED** | `a0[0x36]=0x47`; same shape; C exact. |
| `func_80084B44` | 0x80084B44 | 13 | **CONFIRMED** | Sets `D_8009B73C/D740/D744` to `func_80084B78`/`func_80084F8C`/`func_80084C4C` in that store order (not sorted order); C exact. |
| `func_80085098` | 0x80085098 | 10 | **CONFIRMED** | `func_80085F44(0)`; `D_8009D24C=0`; C exact. |
| `func_800850C0` | 0x800850C0 | 13 | **CONFIRMED** | `D_8009D24C=1`; `func_80085F44(func_80085098)`; C exact incl. function-address argument. |
| `func_800858E8` | 0x800858E8 | 12 | **CONFIRMED** | `i=a0&0xFFFF`; `D_8009B7CC[1] |= D_8009B7D4[i]`; return `i<3` (signed `slti`, `$v0` reused as the index); C exact. |
| `func_80085918` | 0x80085918 | 13 | **CONFIRMED** | `i=a0&0xFFFF`; `D_8009B7CC[1] &= ~D_8009B7D4[i]` (`nor`); return 1; C exact. |
| `func_80085DC4` | 0x80085DC4 | 9 | **CONFIRMED** | `li a0,9; a1=value; jal func_80073CC4`; C exact. |
| `func_80085F44` | 0x80085F44 | 9 | **CONFIRMED** | `if (a0 != D_8009B434) D_8009B434=a0`; C exact. |
| `func_80086464` | 0x80086464 | 13 | **CONFIRMED** | `D_800BCD80=0x10`; `D_800BCD84=a0`; `jal func_8008CBA8`; C exact. |
| `func_800864CC` | 0x800864CC | 11 | **CONFIRMED** | `D_800BCD80=0x40`; `jal func_8008CBA8`; C exact. |
| `func_80086608` | 0x80086608 | 39 | **CONFIRMED** | `func_80085084(a0)!=0` → skip; else `D_800BCD80=0x24`, `D_800BCD84=a0+4`, `D_800BCD88=a1&0xFFFFFF`, `D_800BCD8C=a2&0xFF`, `D_800BCD90=a3&0x7F`, call. C store order/fields/masks exact. |
| `func_800866A4` | 0x800866A4 | 19 | **CONFIRMED** | `a0&=0xFFFF`, `a1&=0xFFFFFF`, `D_800BCD80=0x21`, `D_800BCD84=a0`, `D_800BCD88=a1`, call `func_8008CBA8(a0,a1)`. C exact. |
| `func_80086728` | 0x80086728 | 18 | **CONFIRMED** | `a0==1`→0x81, `a0==2`→0x82, else 0x80 (default materialized in the fall-through path); store `D_800BCD80`; call. C switch exact. |
| `func_800867E4` | 0x800867E4 | 18 | **CONFIRMED** | case1→0x9B, case2→0x9D, default 0x99; C exact. |
| `func_8008682C` | 0x8008682C | 18 | **CONFIRMED** | case1→0x9A, case2→0x9C, default 0x98; C exact. |
| `func_80086C1C` | 0x80086C1C | 16 | **CONFIRMED** | cmd 0xC0; `D_800BCD84=a1&0x7F`; `D_800BCD90=a0`; C exact (note `a0`→field 0x90). |
| `func_80086C5C` | 0x80086C5C | 18 | **CONFIRMED** | cmd 0xC1; `D_800BCD84=a1`; `D_800BCD88=a2&0x7F`; `D_800BCD90=a0`; C exact, incl. no mask on `a1`. |
| `func_80086F34` | 0x80086F34 | 14 | **CONFIRMED** | cmd 0xD8; `D_800BCD84=a0&0xFF`; C exact. |
| `func_80087090` | 0x80087090 | 20 | **CONFIRMED** | `one=1` in `$s2`; `while (func_800851A8(a0,a1)==1);` loop reuses `a0` in delay slot; C `== one` exact. |
| `func_800870E0` | 0x800870E0 | 4 | **CONFIRMED** | `lw D_8009D24C`; C `int` getter exact. |
| `func_80087198` | 0x80087198 | 5 | **CONFIRMED** | `li v0,1; sw D_8009D270; return 0` (`addu v0,zero,zero`); C exact (READY-FROM-BITWISE flags). |
| `func_80087414` | 0x80087414 | 5 | **CONFIRMED** | `li v0,2; sw D_8009D270; return 0`; C exact. |
| `func_80087728` | 0x80087728 | 7 | **CONFIRMED** | `sh a0,0x1F801D8C`; `srl a0,16`; `sh a0,0x1F801D8E`; C volatile base+offsets exact. |
| `func_8008A02C` | 0x8008A02C | 15 | **CONFIRMED** | `d=a1-*a0`; do{`*a0+=d`; `*p+=d`; `a0+=0x10`; `p+=0x10`; `a2--`}while(a2); C exact incl. 0x40-byte stride and do-while shape. |
| `func_8008D7C0` | 0x8008D7C0 | 4 | **CONFIRMED** | `lw v0,D_8009B3A0`; `sw v0,0(a0)` (delay slot); C `*a0=D_8009B3A0` exact. |
| `func_80090574` | 0x80090574 | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_8009059C` | 0x8009059C | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_800905C4` | 0x800905C4 | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_800905EC` | 0x800905EC | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_80090614` | 0x80090614 | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_8009063C` | 0x8009063C | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_80090664` | 0x80090664 | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_8009068C` | 0x8009068C | 10 | **CONFIRMED** | Stream byte consumer: `lw v1,0(a0)`; advance ptr; `lbu` byte; read `+0xF4`, `ori` the recorded bit; store; `sh byte,<field>` in the `jr` delay slot. C bit constant and target field match the disassembly exactly. |
| `func_80085134` | 0x80085134 | 16 | **CONFIRMED** | `jal func_800850C0` then `jal func_80085DF4(a0,a1)`; C exact order/args. |
| `func_80087050` | 0x80087050 | 16 | **CONFIRMED** | `bnez a0` arm returns `D_8009D2E0&1`; `a0==0` arm loops while `D_8009D2E0&1` then returns 0. Edge `D=0` → both return 0; C (with `volatile`) exact. |
| `func_80086568` | 0x80086568 | 15 | **CONFIRMED** | `D_800BCD80=0x12`; `D_800BCD84=a0`; `D_800BCD88=a1`; call; C exact (cmd 0x12). |
| `func_80086D2C` | 0x80086D2C | 15 | **CONFIRMED** | cmd 0xC9; same two-arg shape; C exact. |
| `func_8008C6D0` | 0x8008C6D0 | 15 | **CONFIRMED** | `D_8009D2B8=*(u32*)(a0+4)`; loop `i<0x18` with `|3` on `D_800B8BB4 + i*0x11C`; C exact stride 0x11C and bound 24. |
| `func_800858B0` | 0x800858B0 | 14 | **CONFIRMED** | `u=a0&0xFFFF`; `u>=3`→0; else `*(u16*)(D_8009B7D0 + u*16)`; C exact (`lhu` = unsigned short). |
| `func_800866F0` | 0x800866F0 | 14 | **CONFIRMED** | cmd 0x30; `D_800BCD84=a0&0x3FF`; call; C exact. |
| `func_80086874` | 0x80086874 | 14 | **CONFIRMED** | cmd 0xA8; `D_800BCD84=a0&0x7F`; C exact. |
| `func_800869AC` | 0x800869AC | 14 | **CONFIRMED** | cmd 0xAA; `D_800BCD84=a0&0xFF`; C exact. |
| `func_80086AE4` | 0x80086AE4 | 14 | **CONFIRMED** | cmd 0xAC; `a0&0xFF`; C exact. |
| `func_80086DAC` | 0x80086DAC | 14 | **CONFIRMED** | cmd 0xD0; `a0&0xFF`; C exact. |
| `func_80086E70` | 0x80086E70 | 14 | **CONFIRMED** | cmd 0xD4; `a0&0xFF`; C exact. |
| `func_8007A400` | 0x8007A400 | 13 | **CONFIRMED** | `i=a0&0xFF`; `i>=0x1C`→`D_800119CC`; else `D_8009AFDC[i]`; C exact bounds/returned default. |
| `func_8007A434` | 0x8007A434 | 13 | **CONFIRMED** | `i=a0&0xFF`; `i>=7`→`D_800119CC`; else `D_8009B05C[i]`; C exact. |
| `func_8008594C` | 0x8008594C | 13 | **CONFIRMED** | `u=a0&0xFFFF`; `u>=3`→0; else `*(u16*)(D_8009B7D0+u*16)=0`; return 1; C exact. |
| `func_80086498` | 0x80086498 | 13 | **CONFIRMED** | cmd 0x11; `D_800BCD84=a0`; C exact. |
| `func_800867B0` | 0x800867B0 | 13 | **CONFIRMED** | cmd 0x92; `D_800BCD84=a0`; C exact. |
| `func_80086CF8` | 0x80086CF8 | 13 | **CONFIRMED** | cmd 0xC8; `D_800BCD84=a0`; C exact. |
| `func_8008788C` | 0x8008788C | 13 | **CONFIRMED** | `(mode>>1)<<14 | (field<<6)`, preserve `&0x3F`; C exact. `$v0` return unused by callers (sibling 8783C proven unused). |
| `func_80073D88` | 0x80073D88 | 12 | **CONFIRMED** | `D_8009566C` then `lw 0x10(v0)` argument-less `jalr`; C exact. |
| `func_80073DB8` | 0x80073DB8 | 12 | **CONFIRMED** | `D_8009566C` then `lw 0x18(v0)` argument-less `jalr`; C exact. |
| `func_80075B1C` | 0x80075B1C | 12 | **CONFIRMED** | See src comment; shape reviewed — matches. |
| `func_800878C0` | 0x800878C0 | 12 | **CONFIRMED** | `(mode>>2)<<5 | low`, preserve `&0xFFC0`; C exact. |
| `func_800906B4` | 0x800906B4 | 12 | **CONFIRMED** | `+0x38 |= 0x200`; then stream byte + `+0xF4 |= 0x4400` + `sh 0x116`; C order and constants exact. |
| `func_80074478` | 0x80074478 | 11 | **CONFIRMED** | `if (a1 != D_8009568C[i]) D_8009568C[i]=a1`; C exact (guarded store, `sll i,2`). |
| `func_8007CE80` | 0x8007CE80 | 11 | **CONFIRMED** | if `a2!=0` do{`v=*a1; a0=*;*; i++`}while(`i<a2`) (`sltu`); C exact unsigned bound. |
| `func_8007F960` | 0x8007F960 | 11 | **CONFIRMED** | `if (D_800B8AB8) D_800B8AB8(a0&0xFF)`; C null-check + mask exact. |
| `func_8008B1D0` | 0x8008B1D0 | 11 | **CONFIRMED** | `func_8008A400(*(void**)(a0+4), *(void**)(a0+8))`; C exact. |
| `func_8008F4E8` | 0x8008F4E8 | 11 | **CONFIRMED** | stream byte; `+0xF4 |= 3`; `sh (byte<<8),0x6C`; C exact narrow type (avoids folded `lb`). |
| `func_8008FBFC` | 0x8008FBFC | 11 | **CONFIRMED** | `lbu`; `lhu 0xDE`; sign-extend byte (`sll 24`/`sra 24`) and add to `+0xDE`; C exact (`signed char` add). |
| `func_8008FCE4` | 0x8008FCE4 | 11 | **CONFIRMED** | Twin of 8FBFC at `+0xE0`; C exact. |
| `func_80090B30` | 0x80090B30 | 11 | **CONFIRMED** | stream byte; if nonzero `byte+1` else `0x101`; `sh +0xBA`; C exact magic 0x101. |
| `func_80090BA0` | 0x80090BA0 | 11 | **CONFIRMED** | Twin at `+0xBC`; C exact. |
| `func_80075C6C` | 0x80075C6C | 10 | **CONFIRMED** | `a0[3]=2`; `*(int*)(a0+4)= a1?0xE6000001:0xE6000000`; `*(int*)(a0+8)=0`; C exact. |
| `func_80077A00` | 0x80077A00 | 10 | **CONFIRMED** | `func_80073CF4(2, func_80076EE4)`; C exact. |
| `func_8007DE78` | 0x8007DE78 | 10 | **CONFIRMED** | `func_8007E334(); func_8007E514();`; C exact. |
| `func_80080D34` | 0x80080D34 | 10 | **CONFIRMED** | `func_8007EE84(a0&0xFF, a1, 0, 0)`; C exact — note the `unsigned char` param also discards `a1`? No: `a1` is the 2nd arg, `arg1` kept. Exact. |
| `func_8008783C` | 0x8008783C | 10 | **CONFIRMED** | `(v & 0xFF0F) | (a1<<4)` on `0x1F801C08 + a0<<4`; C exact — return value unused by caller (`lhu a1,0x22` follows), so `void` is safe. |
| `func_80087864` | 0x80087864 | 10 | **CONFIRMED** | `(*p & 0xFFF0) | a1` on `0x1F801C08 + a0<<4`; C exact. |
| `func_8008FBD4` | 0x8008FBD4 | 10 | **CONFIRMED** | stream byte cursor read; C exact (narrow intermediate). |
| `func_8008FCBC` | 0x8008FCBC | 10 | **CONFIRMED** | stream cursor pair twin; C exact. |
| `func_80090948` | 0x80090948 | 10 | **CONFIRMED** | stream byte; `sh 0,+0xD2`; `sh v,+0x58`; `sh v,+0x56`; `sh v,+0xD0`; C exact order and fields. |
| `func_800C7D2C` | 0x800C7D2C | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0824)` (lui+addiu reloc), return 0; C exact. |
| `func_800C8E70` | 0x800C8E70 | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E09A0)`; return 0; C exact. |
| `func_800C9B68` | 0x800C9B68 | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0A94)`; return 0; C exact. |
| `func_800CA700` | 0x800CA700 | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0B84)`; return 0; C exact. |
| `func_800CBF0C` | 0x800CBF0C | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0D08)`; return 0; C exact. |
| `func_800CCEE8` | 0x800CCEE8 | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0E60)`; return 0; C exact. |
| `func_800CD8C8` | 0x800CD8C8 | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0F28)`; return 0; C exact. |
| `func_800CE144` | 0x800CE144 | 10 | **CONFIRMED** | `func_800C2414(a0, &D_800E0FC0)`; return 0; C exact. |
| `func_80074330` | 0x80074330 | 9 | **CONFIRMED** | Word memset countdown `if(n==0)return; c=n-1; do{*p=0;c--;p++}while(c!=-1)`; C exact. |
| `func_800744A4` | 0x800744A4 | 9 | **CONFIRMED** | Twin of 74330; C exact. |
| `func_8007474C` | 0x8007474C | 9 | **CONFIRMED** | Twin of 74330; C exact. |
| `func_8007A8EC` | 0x8007A8EC | 9 | **CONFIRMED** | `func_80073CF4(3, value)`; C exact. |
| `func_8008D820` | 0x8008D820 | 9 | **CONFIRMED** | Word memcpy `n>>=2`; `do{...}while(--n)`; C exact (note `>>2` then words). |
| `func_8008FED8` | 0x8008FED8 | 9 | **CONFIRMED** | `+0x38 &= ~1`; `sh 0,+0xE8`; `+0xF4 |= 0x10`; C exact. |
| `func_8008FFC0` | 0x8008FFC0 | 9 | **CONFIRMED** | stream byte `*p<<8` → `sh +0xA6`; C exact. |
| `func_80090054` | 0x80090054 | 9 | **CONFIRMED** | `+0x38 &= ~2`; `+0xF4 |= 3`; `sh 0,+0xEA`; C exact (store order matches delay-slot layout). |
| `func_800900E4` | 0x800900E4 | 9 | **CONFIRMED** | stream byte `<<7` → `sh +0xB4`; C exact. |
| `func_80090178` | 0x80090178 | 9 | **CONFIRMED** | `+0x38 &= ~4`; `+0xF4 |= 3`; `sh 0,+0xEC`; C exact. |
| `func_80071944` | 0x80071944 | 8 | **CONFIRMED** | `if (flags & 8) return &arg0->value` (`a0+0xC`) else 0; C `BufferInfo.value` at 0xC exact. |
| `func_800719C4` | 0x800719C4 | 8 | **CONFIRMED** | Same with value at `a0+0x14` (`&arg0->value`); C struct `unkC/unk10` padding places `value` at 0x14; exact. |
| `func_8007A3CC` | 0x8007A3CC | 8 | **CONFIRMED** | Tail-call `func_8007B9EC`; C exact. |
| `func_8007A468` | 0x8007A468 | 8 | **CONFIRMED** | Tail-call `func_8007B010`; C exact. |
| `func_8007A488` | 0x8007A488 | 8 | **CONFIRMED** | `return func_8007B290(mode,result)`; C exact return forward. |
| `func_8007A88C` | 0x8007A88C | 8 | **CONFIRMED** | `func_8007B964(buffer); return 1`; C exact. |
| `func_8007A8AC` | 0x8007A8AC | 8 | **CONFIRMED** | `return !func_8007BF44()` (`sltiu v0,1`); C exact. |
| `func_8007A8CC` | 0x8007A8CC | 8 | **CONFIRMED** | `return !func_8007C044()`; C exact. |
| `func_8007A910` | 0x8007A910 | 8 | **CONFIRMED** | Tail-call `func_8007BDDC`; C exact. |
| `func_8007EE64` | 0x8007EE64 | 8 | **CONFIRMED** | Tail-call `func_8007FB04`; C exact. |
| `func_8007F7C8` | 0x8007F7C8 | 8 | **CONFIRMED** | Return `func_8007FC08()` truncated `&0xFF`; C `unsigned char` exact. |
| `func_80080AC4` | 0x80080AC4 | 8 | **CONFIRMED** | `func_8007B964(buffer); return 1`; C exact. |
| `func_80080AE4` | 0x80080AE4 | 8 | **CONFIRMED** | `return func_8007BF44(buffer,value)==0` (`sltiu v0,1`); C exact — args passed through. |
| `func_80080B04` | 0x80080B04 | 8 | **CONFIRMED** | `return func_8007C044(buffer,value)==0`; C exact. |
| `func_80080B24` | 0x80080B24 | 8 | **CONFIRMED** | Tail-call `func_8007BDDC`; C exact. |
| `func_800813E8` | 0x800813E8 | 8 | **CONFIRMED** | Tail-call `func_8007C564`; C exact. |
| `func_80082514` | 0x80082514 | 8 | **CONFIRMED** | Tail-call `func_80082CDC`; C exact. |
| `func_80082554` | 0x80082554 | 8 | **CONFIRMED** | Tail-call `func_80082DBC`; C exact. |
| `func_80083EA4` | 0x80083EA4 | 8 | **CONFIRMED** | `a0[0x36]=0x46`; `*(char**)(a0+0x2C)=a0+0x24`; `a0[0x24]=a1`; `a0[0x35]=1`; C exact. |
| `func_80084B20` | 0x80084B20 | 8 | **CONFIRMED** | `result = D_800A5B70; if (a0 & 0xF0) result += 0xF0`; C exact (`+0xF0` not `+0x100`). |
| `func_80084FC4` | 0x80084FC4 | 8 | **CONFIRMED** | `count = *(volatile u16*)0x1F801120`; `D_800BD02C=limit`; `D_800A76D0=count`; C exact order/volatile. |
| `func_80089960` | 0x80089960 | 8 | **CONFIRMED** | `D_8009D2C4 |= 0x100`; C exact. |
| `func_80089B28` | 0x80089B28 | 8 | **CONFIRMED** | `D_8009D2C4 |= 0x100`; C exact. |
| `func_80089CF0` | 0x80089CF0 | 8 | **CONFIRMED** | `D_8009D2C4 |= 0x100`; C exact. |
| `func_8008C16C` | 0x8008C16C | 8 | **CONFIRMED** | `lb` signed `a0[4]`; `sh 0,D_8009D220`; `D_8009D2D0 = value<<16`; C `signed char` + `<<16` exact (uses `lb` not `lbu`). |
| `func_8008C270` | 0x8008C270 | 8 | **CONFIRMED** | Twin writing `D_8009D21E`/`D_8009D2CC`; C exact. |
| `func_80091080` | 0x80091080 | 8 | **CONFIRMED** | Tail-call `func_80090F68`; C exact. |
| `func_800C2AF0` | 0x800C2AF0 | 8 | **CONFIRMED** | `base+=3`; `D_800E2248=base`; `base[index+18]=value`; return 0; C exact (`sw a3,0x48(a2)` = `(index+18)*4`). |
| `func_8007C544` | 0x8007C544 | 7 | **CONFIRMED** | Three independent `lui $at` stores `D_800C0DC0=a0`, `D_800B6918=a1`, `D_800C0DBC=a2`; C exact. |
| `func_80083C20` | 0x80083C20 | 7 | **CONFIRMED** | `value=*(u32*)(a0+0x20)`; `a0[0x36]=0x4D`; `a0[0x35]=6`; `*(u32*)(a0+0x2C)=value`; C exact. |
| `func_80085728` | 0x80085728 | 7 | **CONFIRMED** | `D_8009D240=base`; `D_8009D260=(char*)base+0x800`; C exact (`addiu a0,0x800`). |
| `func_8008770C` | 0x8008770C | 7 | **CONFIRMED** | `sh a0,0x1F801D88`; `srl a0,16`; `sh a0,0x1F801D8A`; C exact (base[0xEC4]/[0xEC5]). |
| `func_80087744` | 0x80087744 | 7 | **CONFIRMED** | base[0xECC]/[0xECD] = 0x1F801D98/0x1F801D9A; C exact. |
| `func_80087760` | 0x80087760 | 7 | **CONFIRMED** | base[0xECA]/[0xECB]; C exact. |
| `func_8008777C` | 0x8008777C | 7 | **CONFIRMED** | base[0xEC8]/[0xEC9]; C exact. |
| `func_800877D4` | 0x800877D4 | 7 | **CONFIRMED** | `sh (a1>>3),0x1F801C06 + a0<<4`; C exact (`srl` unsigned). |
| `func_800877F0` | 0x800877F0 | 7 | **CONFIRMED** | `sh (a1>>3),0x1F801C0E + a0<<4`; C exact. |
| `func_80089F08` | 0x80089F08 | 7 | **CONFIRMED** | `a0 = (a0<<4) + D_8009B3FC`; `*a1 = *(u16*)(a0+0xC)`; C exact displacement on base. |
| `func_8008F84C` | 0x8008F84C | 7 | **CONFIRMED** | stream byte → `sh +0x7C`; C exact. |
| `func_80073DF8` | 0x80073DF8 | 6 | **CONFIRMED** | `lhu *D_80095674`; C `unsigned short` deref exact. |
| `func_800877BC` | 0x800877BC | 6 | **CONFIRMED** | `sh a1,0x1F801C04 + a0<<4`; C exact. |
| `func_8008F868` | 0x8008F868 | 6 | **CONFIRMED** | `(u16 + 1) & 0xF` at `+0x7C`; C exact. |
| `func_8008F880` | 0x8008F880 | 6 | **CONFIRMED** | `(u16 - 1) & 0xF`; C exact. |
| `func_800C2B10` | 0x800C2B10 | 6 | **CONFIRMED** | `(a0<<2)+8 + D_800E2248`; C exact (index scaling). |
| `func_800C2B28` | 0x800C2B28 | 6 | **CONFIRMED** | `(a0<<2)+0x48 + D_800E2248`; C exact. |
| `func_800C2B50` | 0x800C2B50 | 6 | **CONFIRMED** | `D_800E2248[0x1C]` = `+0x70`; C exact. |
| `func_800C6EC0` | 0x800C6EC0 | 6 | **CONFIRMED** | `D_800F346C=(u16)first`; `D_800F3414=(u16)second`; C exact. |
| `func_80074A14` | 0x80074A14 | 5 | **CONFIRMED** | Exchange `D_800956EC`, return old; C exact. |
| `func_80077B84` | 0x80077B84 | 5 | **CONFIRMED** | `a0[3]=6`, `a0[7]=48`; C exact. |
| `func_80077BE4` | 0x80077BE4 | 5 | **CONFIRMED** | `a0[3]=12`, `a0[7]=60`; C exact. |
| `func_80077C24` | 0x80077C24 | 5 | **CONFIRMED** | `a0[3]=2`, `a0[7]=104`; C exact. |
| `func_8007A3EC` | 0x8007A3EC | 5 | **CONFIRMED** | Exchange signed `D_8009AFC0`, return old; C `int` exact. |
| `func_8007A4A8` | 0x8007A4A8 | 5 | **CONFIRMED** | Exchange callback `D_8009AFB4`; C exact. |
| `func_8007A4BC` | 0x8007A4BC | 5 | **CONFIRMED** | Exchange callback `D_8009AFB8`; C exact. |
| `func_80081254` | 0x80081254 | 5 | **CONFIRMED** | Exchange callback `D_8009B6D0`; C exact. |
| `func_800824B4` | 0x800824B4 | 5 | **CONFIRMED** | Exchange `D_800B8AB0` via `$3`; C exact. |
| `func_80082CDC` | 0x80082CDC | 5 | **CONFIRMED** | Return `D_8009B78C`, then 0; C exact. |
| `func_80083EE4` | 0x80083EE4 | 5 | **CONFIRMED** | `a0[0x36]=0x4B`; `*(u32*)(a0+0x2C)=0`; `a0[0x35]=0`; C exact. |
| `func_8008C70C` | 0x8008C70C | 5 | **CONFIRMED** | `D_8009D2C8->value (+0x56) = arg0[1]`; C exact offset 0x56. |
| `func_8008F694` | 0x8008F694 | 5 | **CONFIRMED** | `*(u32*)a0 += 2`; C exact. |
| `func_80090A0C` | 0x80090A0C | 5 | **CONFIRMED** | `+0x38 &= ~8`; C exact. |
| `func_80090C38` | 0x80090C38 | 5 | **CONFIRMED** | `+0x38 |= 0x10`; C exact. |
| `func_80090C4C` | 0x80090C4C | 5 | **CONFIRMED** | `+0x38 &= ~0x10`; C exact (`li -17`). |
| `func_80090C60` | 0x80090C60 | 5 | **CONFIRMED** | `+0x38 |= 0x20`; C exact. |
| `func_80090C74` | 0x80090C74 | 5 | **CONFIRMED** | `+0x38 &= ~0x20`; C exact (`li -33`). |
| `func_80090F54` | 0x80090F54 | 5 | **CONFIRMED** | `+0x38 |= 0x100000` (`lui 0x10`); C exact. |
| `func_80074CB8` | 0x80074CB8 | 4 | **CONFIRMED** | `lbu D_8009574E`; C `unsigned char` getter exact. |
| `func_8007A324` | 0x8007A324 | 4 | **CONFIRMED** | `lbu D_8009AFC4`; C exact. |
| `func_8007A334` | 0x8007A334 | 4 | **CONFIRMED** | `lbu D_8009AFD4`; C exact. |
| `func_8007A344` | 0x8007A344 | 4 | **CONFIRMED** | `lbu D_8009AFD5`; C exact. |
| `func_8007FC08` | 0x8007FC08 | 4 | **CONFIRMED** | `lbu D_8009B580`; C exact. |
| `func_8007FC18` | 0x8007FC18 | 4 | **CONFIRMED** | `lbu D_8009B581`; C exact. |
| `func_8007FC34` | 0x8007FC34 | 4 | **CONFIRMED** | `lbu D_8009B586`; C exact. |
| `func_8007FC44` | 0x8007FC44 | 4 | **CONFIRMED** | `lbu D_8009B587`; C exact. |
| `func_800835B0` | 0x800835B0 | 4 | **CONFIRMED** | `sb a1,0x36`; `sw a2,0x2C`; `sb a3,0x35`; C exact. |
| `func_800847A0` | 0x800847A0 | 4 | **CONFIRMED** | `v=a0[0x36]; a0[0x36]=0; a0[0x37]=v`; C exact. |
| `func_800C2B40` | 0x800C2B40 | 4 | **CONFIRMED** | `D_800E2248->[0x1C] = arg0` (`sw a0,0x70`); C exact. |
| `func_800C6ED8` | 0x800C6ED8 | 4 | **CONFIRMED** | `D_800F33E4=(u16)value`; C exact. |
| `func_800C6EE8` | 0x800C6EE8 | 4 | **CONFIRMED** | `D_800F3420=(u16)value`; C exact. |
| `func_800C7DC4` | 0x800C7DC4 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800C8F08` | 0x800C8F08 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800C9C00` | 0x800C9C00 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800CA798` | 0x800CA798 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800CBFA4` | 0x800CBFA4 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800CCF80` | 0x800CCF80 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800CD960` | 0x800CD960 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800CE1DC` | 0x800CE1DC | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_800D4850` | 0x800D4850 | 4 | **CONFIRMED** | `*a0=4; return 0`; C exact. |
| `func_8007A354` | 0x8007A354 | 3 | **CONFIRMED** | `lui v0,%hi(D_8009AFD0); jr; addiu %lo`; C `&D_8009AFD0` exact. |
| `func_8007C130` | 0x8007C130 | 3 | **CONFIRMED** | `sw a0,D_8009B260`; C opaque `unsigned int` setter exact. |
| `func_8007DEA4` | 0x8007DEA4 | 3 | **CONFIRMED** | `sw a0,D_8009B4AC`; C exact. |
| `func_800904A0` | 0x800904A0 | 3 | **CONFIRMED** | `li v0,1; sh +0x84`; C exact. |
| `func_800C8BB4` | 0x800C8BB4 | 3 | **CONFIRMED** | `*a1=2` (a0 unused); return `$v0=2`. `$v0` leftover; caller is a table slot invoked for side effect (see note). |
| `func_800C9968` | 0x800C9968 | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_800CA4A8` | 0x800CA4A8 | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_800CBB24` | 0x800CBB24 | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_800CD5A4` | 0x800CD5A4 | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_800CD71C` | 0x800CD71C | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_800CDF40` | 0x800CDF40 | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_800CE464` | 0x800CE464 | 3 | **CONFIRMED** | `*a1=2` twin of func_800C8BB4 (same table family); C exact side effect. |
| `func_8008CA7C` | 0x8008CA7C | 2 | **CONFIRMED** | `jr;nop` empty stub; C `{}` exact. |
| `func_8008F6A8` | 0x8008F6A8 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_8008FCB4` | 0x8008FCB4 | 2 | **CONFIRMED** | `jr; sh 0,+0x82`; C exact. |
| `func_800904AC` | 0x800904AC | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800904B4` | 0x800904B4 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800904BC` | 0x800904BC | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800C7DD4` | 0x800C7DD4 | 2 | **CONFIRMED** | `jr; addu v0,zero,zero` (`return 0`); C exact. |
| `func_800C7DDC` | 0x800C7DDC | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800C8268` | 0x800C8268 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800C8F18` | 0x800C8F18 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800C8F20` | 0x800C8F20 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800C9260` | 0x800C9260 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800C9C10` | 0x800C9C10 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800C9C18` | 0x800C9C18 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800C9EA0` | 0x800C9EA0 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800CA7A8` | 0x800CA7A8 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CA7B0` | 0x800CA7B0 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CACD4` | 0x800CACD4 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800CBFB4` | 0x800CBFB4 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CBFBC` | 0x800CBFBC | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CCF90` | 0x800CCF90 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CCF98` | 0x800CCF98 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CD2DC` | 0x800CD2DC | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800CD2E4` | 0x800CD2E4 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800CD59C` | 0x800CD59C | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800CD970` | 0x800CD970 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CD978` | 0x800CD978 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CDD04` | 0x800CDD04 | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800CE1EC` | 0x800CE1EC | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CE1F4` | 0x800CE1F4 | 2 | **CONFIRMED** | `return 0`; C exact. |
| `func_800CE3AC` | 0x800CE3AC | 2 | **CONFIRMED** | `jr;nop` empty stub; C exact. |
| `func_800751E4` | 0x800751E4 | 50 | **CONFIRMED** | Log gate `D_8009574E>=2` → `D_80095748(&D_800118F8,s0,s1)`; `n=s1-1`; while `n!=0`: link each 4-byte node (element-form OR `*(int*)s0=(*(int*)s0&0xFF000000)|((unsigned)(s0+4)&0xFFFFFF)`, `s0[3]=0`); then `*(int*)&D_8009580C = ((unsigned)&D_800957F8 & 0xFFFFFF)|0x4000000` and `*(int*)s0 = (unsigned)&D_8009580C & 0xFFFFFF`; return s0. C loop-local `lo`/`hi` constants and `next` pointer exact. |
| `func_800755BC` | 0x800755BC | 13 | **CONFIRMED** | `func_80071A34(a0,&D_8009575C,0x5C)`; return a0; C exact template/size/return. |
| `func_80075AE8` | 0x80075AE8 | 13 | **CONFIRMED** | `func_80071A34(a0,&D_800957B8,0x14)`; return a0; C exact. |
| `func_80075B84` | 0x80075B84 | 32 | **CONFIRMED** | `a0[3]=2`; `*(int*)(a0+4)=func_80076170(a1->x,a1->y)` (two `lh`); `*(int*)(a0+8)=func_80076208(x+w-1,y+h-1)` (u16 adds then sign-extend `sra 16`). C `struct XY` shorts and the `-1` bounds exact. |
| `func_80075C04` | 0x80075C04 | 16 | **CONFIRMED** | `a0[3]=2`; `*(int*)(a0+4)=func_800762A0(a1[0],a1[1])` (`lh` pair); `*(int*)(a0+8)=0`; C exact. |
| `func_80075C94` | 0x80075C94 | 21 | **CONFIRMED** | `a0[3]=2`; `*(int*)(a0+4)=func_80076150(a1,a2,a3&0xFFFF)`; `*(int*)(a0+8)=func_800762BC(a4)` where `a4` is read from caller stack `sp+0x30`. C 5th-arg and `0xFFFF` mask exact. |
| `func_8007DBC8` | 0x8007DBC8 | 15 | **CONFIRMED** | `v = D_8009B3FC[a0]` (`lhu`, narrow local homed in `$a0`); `a1==-1` → return v; else `v << D_8009B424` (`sllv`, unsigned count); C exact. |
| `func_80084AE8` | 0x80084AE8 | 14 | **CONFIRMED** | Linear search over 2 entries at `D_800A5B70 + i*0xF0`; returns `0x10 + i*0x10` on match else `0xFF`; C loop bounds/stride/slot exact. |
| `func_8008AB1C` | 0x8008AB1C | 32 | **CONFIRMED** | `a2=(a2&0x3FF)<<1`; two independent lookups: `v=D_8009D240[a2]`, if `v!=0xFFFF` result=`D_8009D260+v` else 0, `*out0=result`; `a2++`; repeat into `*out1`. C exact (D_8009D240 `unsigned short *`, D_8009D260 `unsigned char *`, unscaled byte add). |
| `func_8008F430` | 0x8008F430 | 16 | **CONFIRMED** | Two bytes: `lo=p[0]`, `q=p+2`, `hi=p[1]`, `q += (signed short)(lo|(hi<<8))`, store cursor. C exact — the `signed short` cast reproduces `sll 16`/`sra 16`. |
| `func_8008F6B0` | 0x8008F6B0 | 17 | **CONFIRMED** | Stream byte `<<8` → `sh +0xD8`; `sh 0,+0x74`; if `*(+0x38) & 0x100` then `+0xF4 |= 3`; C store order/mask exact. |
| `func_8008F784` | 0x8008F784 | 14 | **CONFIRMED** | Stream byte → `sh 0,+0x78`; `+0xF4 |= 3`; `sh (((c+0x40)&0xFF)<<8),+0x76`; C exact (`addiu 0x40` then `andi 0xFF` before the shift). |
| `func_8008FC28` | 0x8008FC28 | 20 | **CONFIRMED** | Stream byte → `sh +0x7E`, if zero overwrite `0x100`; then second byte sign-extended (`sll 24`/`sra 24`) → `sh +0xE4`; C exact. |
| `func_800864F8` | 0x800864F8 | 28 | **CONFIRMED** | `p=&D_800BCD80`; `*p=0x19`; `D_800BCD84=a0`; `r=func_8008CBA8()`; `*p=0xC0`; `D_800BCD84=a1&0x7F`; `D_800BCD90=0`; `func_8008CBA8()`; return r. C exact incl. the **two-call** sequence and returning the first call’s value. |
| `func_800865A4` | 0x800865A4 | 25 | **CONFIRMED** | cmd 0x20; `a0&0x3FF`, `a1&0xFFFFFF`, `a2&0xFF`, `a3&0x7F`; C exact. |
| `func_80086770` | 0x80086770 | 16 | **CONFIRMED** | cmd 0x90; `D_800BCD84 = a0 & 0xFFFFFF` (`lui 0xff/ori 0xffff` + `and`, **not** `andi 0xFFFF`); C exact. |
| `func_800868AC` | 0x800868AC | 17 | **CONFIRMED** | cmd 0xA9; `a0&0xFF`, `a1&0x7F`; C exact. |
| `func_800868F0` | 0x800868F0 | 22 | **CONFIRMED** | cmd 0xA0; `a0&0xFFFF`, `a1&0xFFFFFF`, `a2&0x7F`; C exact. |
| `func_80086948` | 0x80086948 | 25 | **CONFIRMED** | cmd 0xA1; `a0&0xFFFF`, `a1&0xFFFFFF`, `a2&0xFF`, `a3&0x7F`; C exact. |
| `func_800869E4` | 0x800869E4 | 17 | **CONFIRMED** | cmd 0xAB; `a0&0xFF`, `a1&0xFF`; C exact. |
| `func_80086A28` | 0x80086A28 | 22 | **CONFIRMED** | cmd 0xA2; `a0&0x3FF`, `a1&0xFFFFFF`, `a2&0xFF`; C exact. |
| `func_80086A80` | 0x80086A80 | 25 | **CONFIRMED** | cmd 0xA3; `a0&0x3FF`, `a1&0xFFFFFF`, `a2&0xFF`, `a3&0xFF`; C exact. |
| `func_80086B1C` | 0x80086B1C | 17 | **CONFIRMED** | cmd 0xAD; `a0&0xFF`, `a1&0xFF`; C exact. |
| `func_80086B60` | 0x80086B60 | 22 | **CONFIRMED** | cmd 0xA4; `a0&0xFFFF`, `a1&0xFFFFFF`, `a2&0xFF`; C exact. |
| `func_80086BB8` | 0x80086BB8 | 25 | **CONFIRMED** | cmd 0xA5; `a0&0xFFFF`, `a1&0xFFFFFF`, `a2&0xFF`, `a3&0xFF`; C exact. |
| `func_80086CA4` | 0x80086CA4 | 21 | **CONFIRMED** | cmd 0xC2; `*p=0xC2`; `D_800BCD84=a1` (unmasked); `D_800BCD88=a2&0x7F`; `D_800BCD8C=a3&0x7F`; `D_800BCD90=a0`; C exact field/arg mapping. |
| `func_80086D68` | 0x80086D68 | 17 | **CONFIRMED** | cmd 0xCA; `D_800BCD84=a0`, `D_800BCD88=a1`, `D_800BCD8C=a2` (all unmasked); C exact. |
| `func_80086DE4` | 0x80086DE4 | 16 | **CONFIRMED** | cmd 0xD1; `D_800BCD84=a0`, `D_800BCD88=a1&0xFF`; C exact. |
| `func_80086E24` | 0x80086E24 | 19 | **CONFIRMED** | cmd 0xD2; `a0`, `a1&0xFF`, `a2&0xFF`; C exact. |
| `func_80086EA8` | 0x80086EA8 | 16 | **CONFIRMED** | cmd 0xD5; `a0`, `a1&0xFF`; C exact. |
| `func_80086EE8` | 0x80086EE8 | 19 | **CONFIRMED** | cmd 0xD6; `a0`, `a1&0xFF`, `a2&0xFF`; C exact. |
| `func_80086F6C` | 0x80086F6C | 16 | **CONFIRMED** | cmd 0xD9; `a0`, `a1&0xFF`; C exact. |
| `func_80086FAC` | 0x80086FAC | 19 | **CONFIRMED** | cmd 0xDA; `a0`, `a1&0xFF`, `a2&0xFF`; C exact. |
| `func_800906E4` | 0x800906E4 | 14 | **CONFIRMED** | `+0x38 &= ~0x200` (`li -513`); `byte = D_800B290C[*(u16*)(a0+0x5A) << 6]`; `+0xF4 |= 0x4400`; `sh byte,+0x116`; C exact index scale 6 and mask. |
| `func_8009071C` | 0x8009071C | 14 | **CONFIRMED** | `h=(*(u16*)(a0+0xCE)+1)&3`; store back; `*(u32*)(a0+4+(h<<2)) = *(u32*)a0`; re-load `+0xCE`, `sh 0,(h<<1)+a0+0x62`; C exact (two independent halfword accesses, byte offsets). |
| `func_800C2B68` | 0x800C2B68 | 10 | **CONFIRMED** | `(D_800E2248->value & 0xFFFF0000) == 0x01010000` via `lui 0xffff`/`and`/`lui 0x101`/`xor`/`sltiu 1`; C exact. |
| `func_800C7D00` | 0x800C7D00 | 11 | **CONFIRMED** | Wrapper: copies 5th/6th args from `sp+0x30/0x34` to the call, `func_800C2AF0`, return 0; C exact. |
| `func_800C8E44` | 0x800C8E44 | 11 | **CONFIRMED** | Twin of C7D00 (same 6-arg forward + return 0); C exact. |
| `func_800C9B3C` | 0x800C9B3C | 11 | **CONFIRMED** | Twin of C7D00; C exact. |
| `func_800CA6D4` | 0x800CA6D4 | 11 | **CONFIRMED** | Twin of C7D00; C exact. |
| `func_800CBEE0` | 0x800CBEE0 | 11 | **CONFIRMED** | Twin of C7D00; C exact. |
| `func_800CCEBC` | 0x800CCEBC | 11 | **CONFIRMED** | Twin of C7D00; C exact. |
| `func_800CD89C` | 0x800CD89C | 11 | **CONFIRMED** | Twin of C7D00; C exact. |
| `func_800CE118` | 0x800CE118 | 11 | **CONFIRMED** | Twin of C7D00; C exact. |
| `func_800C8C4C` | 0x800C8C4C | 13 | **CONFIRMED** | `values[2] -= 20` (u16); sign-extend and `slti 0x14`; if underflow `values[2]=0; state[1]=2`; C exact. |
| `func_800C9A00` | 0x800C9A00 | 13 | **CONFIRMED** | Twin of C8C4C (step 20); C exact. |
| `func_800CA540` | 0x800CA540 | 13 | **CONFIRMED** | Twin of C8C4C (step 20); C exact. |
| `func_800CBBBC` | 0x800CBBBC | 13 | **CONFIRMED** | Twin of C8C4C (step 20); C exact. |
| `func_800C8C80` | 0x800C8C80 | 15 | **CONFIRMED** | `+4 -= 8`; `+6 += 0x3C`; independent signed `lh +4 < 0x14` → `+4=0`, `a1[1]=2`; C exact (second independent load). |
| `func_800C8CBC` | 0x800C8CBC | 15 | **CONFIRMED** | Twin of C8C80 with `+6 += 0x78`; C exact. |
| `func_800C8CF8` | 0x800C8CF8 | 15 | **CONFIRMED** | Twin of C8C80 with `+6 += 0x0A`; C exact. |
| `func_800C9A34` | 0x800C9A34 | 15 | **CONFIRMED** | Twin of C8C80 with `+6 += 0x28`; C exact. |
| `func_800CBBF0` | 0x800CBBF0 | 15 | **CONFIRMED** | Twin of C8C80 with `+6 += 0x3C`; C exact. |
| `func_800CBC2C` | 0x800CBC2C | 15 | **CONFIRMED** | Twin of C8C80 with `+6 += 0x78`; C exact. |
| `func_800CBC68` | 0x800CBC68 | 15 | **CONFIRMED** | Twin of C8C80 with `+6 += 0x0A`; C exact. |
| `func_800CD5B0` | 0x800CD5B0 | 15 | **CONFIRMED** | `+4 -= 0x0A` (not 8); `+6 += 0x3C`; signed `lh +4 < 0x14` → `+4=0`, `a1[1]=2`; C exact. |
| `func_800CCA40` | 0x800CCA40 | 14 | **CONFIRMED** | `*(u8*)(a2+3) -= 2`; `*(u16*)(a2+4) += 0x28`; `*(signed char*)(a2+3) < 0x1E` → `a1[1]=2`; C exact. |
| `func_800CCA78` | 0x800CCA78 | 14 | **CONFIRMED** | `*(u8*)(a2+3) -= 6`; `+4 += 0xB4`; `< 0x1E` → `a1[1]=2`; C exact. |
| `func_800CCB6C` | 0x800CCB6C | 15 | **CONFIRMED** | `*(u8*)(a2+3) -= 8`; `+4 += 0x1A4`; `< 0x14` → `+3=0`, `a1[1]=2`; C exact. |
| `func_800CE470` | 0x800CE470 | 11 | **CONFIRMED** | `++counter[3]` (signed char); `== 6` → `state[1]=2`; C exact (the `sll 24`/`sra 24` compare). |
| `func_800CE870` | 0x800CE870 | 32 | **CONFIRMED** | mode 0: `out[0..2] = obj->source->field_14/18/1C` (int low half via `sh`); mode 1: `out[0..2] = obj->field_28/2C/30 >> 16` (= signed `lh` at 0x2A/0x2E/0x32). Source pointer at `+0x238`. C struct offsets and both modes exact. |

## Result

- **360 / 360 audited → CONFIRMED**, **0 DEFECT**, **0 UNCERTAIN**.
- All 360 are independently byte/link-exact at their declared VMA (the
  full-image strong sweep covers them), so they move from "byte-confirmed" to
  "byte-confirmed + semantics-confirmed".

**Semantics-confirmed total:**
- conservative (only the 29 named baseline rows): **29 + 360 = 389 / 750**;
- as previously claimed (42 baseline): **42 + 360 = 402 / 750**.

The 360 in this file are a strict, disjoint addition under either baseline.

## Notable near-misses examined and cleared

These are the "multiple C spellings compile to identical bytes" cases where the
audit had to work harder than usual; all resolved CONFIRMED.

- **`func_80077AC4` — retail masks the pointer, not the pointee.** The second
  statement is `*a0 = (*a0 & 0xFF000000) | (((unsigned)a1) & 0xFFFFFF)`: retail
  emits `and a1,a1,a2` on the **register holding `a1`**, i.e. it masks the
  *address*, not `*a1`. The C reproduces exactly that (with the pinned
  `$6`/`$7` constants). A cleaner spelling that loaded `*a1` would compile
  differently and be semantically different; the matched C is faithful.
- **`func_80077A64` / `func_80077AA4` — `sra` vs `srl`.** Retail uses `sra`
  for the `>>4`, `>>6` shifts, but after the `&0x100`, `&0x3FF`, `&0x3F` masks
  the values are non-negative, so `sra` == `srl` and the `int` C is equivalent.
- **`func_80076150` vs `func_80077C84` — arg-to-flag mapping.** `80076150`
  maps `arg1`→`0x200`, `arg0`→`0x400`; `80077C84` maps `arg2`→`0x200`,
  `arg1`→`0x400`. The C follows each retail delay-slot/register assignment, not
  a shared template.
- **`func_800C8BB4` family (8 leaves) — latent off-program return.** These
  leaves store `2` into `a1[1]` but also leave `$v0 = 2`; the matched C is
  `void`. Their only references are **data-table slots** (`.word
  func_800C8BB4` at `D_800E0840`, consumed by the non-matching `func_800C2414`
  `jalr` loop in `asm/disc1/B2AF8.s`), which discards `$v0` for the sibling
  entries. The published game program never reads the slot return. Not a
  defect; noted for the record.
- **`func_8008783C` / `func_8008788C` / `func_800878C0` — unused returns.**
  Retail leaves `$v0` set but the sole caller (`func_800878F0`,
  `asm/disc1/780F0.s`) ignores it (`jal func_8008783C` immediately followed by
  another `lhu`/`jal`). The C `void` is safe.
- **`func_80076B20` — full-word store truncated to `sb`.** Retail is
  `sb a0,D_800A3348(v0)` where `v0 = a0>>24`; the C `D_800A3348[a0>>24] = a0`
  relies on byte-store truncation and sets `unsigned` so the index uses `srl`
  (not `sra`). Exact.
- **`func_80077D30` / `func_80077DC4` — four-table sine dispatch.** Each arm
  reads a *different* table (`D_8009489C`, `D_8009509C`, `D_8009589C`,
  `D_8009409C`) with a different index direction and negation. All arms
  re-derived address-by-address; C exact, including the asymmetry between the
  two functions.
- **`func_800749D8` — byte pairs stored out of order.** Retail stores
  `sb 0x11` then `0x10` then `0x13` then `0x12` (fields at 0x10..0x13); only a
  struct declared in exactly that order reproduces it. C matches.
- **`func_8008C16C` / `func_8008C270` — `lb` (not `lbu`).** Retail uses the
  signed byte load and `sll 16`; the C `signed char value` is required.
- **`func_8008FED8` / `func_80090054` / `func_80090178` — `~1`/`~2`/`~4` clear
  masks.** Retail materializes `li a1,-2` / `-3` / `-5`; the C uses `&= ~N`
  and the `+0xF4` OR constants `0x10` / `3` / `3`. Exact.
- **`func_80086608` / `func_80086C1C` / `func_80086C5C` / `func_80086CA4` —
  command-record field order.** Retail interleaves the `lui $at` address
  materialization with the masked-value computation and stores `+0x84`,
  `+0x88`, `+0x8C`, `+0x90` in varying orders (e.g. `0xC2` stores `+0x8C`
  before `+0x90`, with `a0` going to `+0x90` and `a1` to `+0x84`). Every
  command word, mask, and field/arg mapping was re-checked individually.
- **`func_80086770` — `& 0xFFFFFF`, not `& 0xFFFF`.** Retail uses
  `lui 0xff/ori 0xffff` + `and`, unlike the sibling masks that use `andi`.
- **`func_80077D30` — the `negu` in the `jr` delay slot.** Retail computes
  arm 4 as `D_8009589C[0x1000-a0]` and only negates in the return delay slot;
  the C `-D_8009589C[0x1000 - a0]` carries the same value. The third arm reads
  `D_8009489C` (not `D_8009589C`) — re-checked by immediate (`18588` vs
  `22684`).
- **`func_80086608` — `a0+4` as a stored value, not a pointer.** Retail stores
  `s0+4` into `D_800BCD84`; the C `D_800BCD84 = a0 + 4` is exact (integer
  arithmetic, not `&a0[1]`).
- **`func_8007E594` — byte 4 stored before the loop.** Retail does
  `sb zero,4(a0)` once, then the `q[5]` loop covers bytes 5..8; the C puts the
  `a0[4]=0` before the `do` loop. Off-by-one / iteration count exact.
- **`func_8008A02C` — `d` is a delta of word 0, applied to word 0 too.** Retail
  re-loads `*a0` after `d = a1 - *a0` and adds `d` to both `a0[0]` and `a0[1]`
  each iteration with a `0x40`-byte stride; C exact (`do…while(a2)`).
- **`func_80076B58` — down-count sentinel.** Retail `c = a1-1`, then
  `c--`/`bne c,-1`; the C `do {… c--; } while (c != -1)` is exact, including the
  `a1 != 0` pre-guard.
- **`func_800858E8` / `func_80085918` — bit-set vs bit-clear via `nor`.**
  `|=` for one, `&= ~` (`nor v0,zero,v0`) for the other, with the bound test
  `slti i,3` reusing the same `$v0` register. C exact.
- **`func_8007E594` — descending byte loop with a parallel pointer.** The
  loop walks `i = 3..0` clearing `q[5]` with `q = a0+3; q--`; C reproduces the
  direction (`do { q[5]=0; i--; q--; } while (i>=0)`).
- **`func_8008F430` — signed displacement.** Retail does
  `sll 16`/`sra 16` before `addu`; the C uses `(signed short)(lo | (hi << 8))`.
- **`func_800C8C4C` family (5 twins) vs `func_800C8C80` family (8 twins).**
  The `-=20` group has a single `lhu`/re-sign-extend path; the `-=8` group
  re-reads `+4` as an independent `lh` for the `slti` test and writes `+6`.
  `func_800CD5B0` is the outlier (`-=0x0A`). All step constants and bound
  constants (0x14 / 0x1E) individually re-checked.
- **`func_800CE870` — mode-1 high-half reads.** `obj->field_28 >> 16` in C
  compiles to `lh 0x2A(a0)`; the C’s `int` fields and `>>16` are faithful
  (equivalent to `(short)(field>>16)` here).
- **`func_8007DBC8` — narrow local.** The `unsigned short v` local is
  load-bearing (`lhu a0,...` homes the value in `$a0`); an `unsigned int`
  would land in `$v1` with an extra `move`.

## Exact command(s) used to read the disassembly

```
tools/mipsel-host/bin/mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL \
  --start-address=<file_off> --stop-address=<file_off+span> \
  build/extracted/disc1/SLUS_006.62
```

with `file_off = 0x800 + vram - 0x80010000`.

## Next batch

None left in Shard B (all 360 audited). The remaining surface is Shard A’s 361
leaves (owned by the sibling auditor) plus the byte-confirmed-only remainder;
recommend continuing the same per-leaf re-derivation on the next highest-fan-in
remaining leaves.
