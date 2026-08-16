# PE-B54J — func_80030894 read-only structural audit (evidence only)

Goal: 100% retail accuracy to the sewers. Rung B54I completed the callee
set; this rung audits the 788-word body before any production C. **No
production code was written.** The audit corrected two early working
hypotheses (see "corrections") — the reason the audit-first rule exists.

## Function identity

```text
symbol     func_80030894
window     0x80030894..0x800314E4 exclusive (788 words / 0xC50 bytes)
file off   0x21094  (= 0x80030894 - 0x8000F800)
sha256     a4dbd2cf130979a0f5db8ed532d0c559c5b3b90b2c5786e10fe91125311ed6e2
caller     sole jal func_8006AD40 @ 0x8006B0AC
next fn    func_800314E4 (own prologue addiu sp,-32 verified at 0x800314E4)
ABI        void(void): no incoming register is read before write
           (backward liveness fixpoint: a0-a3 dead at entry; $v0 at exit
            is a stale scratch value, discarded by the caller)
```

## Corrections to working assumptions

1. **"No branches" was false.** A tab-separated grep missed the loop
   family. Opcode-census decoding finds 12 control-flow words: **11
   `bnez`** (10 group loops + the outer bank test) + 1 `jr $ra`.
   **The body is a loop nest, not straight-line.**
2. **"sp+24 written once" was false.** sp+24 is the OUTER loop counter:
   zeroed at 0x8003090C, incremented at 0x80031484, tested at 0x800314A8.
3. My earlier eyeball `0x800CE9F0` was wrong; `lui 0x800C, addiu -5648`
   resolves to **0x800BE9F0** (machine-derived, resolver-verified).

## Control flow (all 11 loops, machine-extracted)

| # | head..tail | counter | bound (sltiu) | iterations | body |
| - | --- | --- | --- | --- | --- |
| L1 | 0x80030910..0x800314A8 | sp+24 (i) | `< 2` @0x800314A4 | **2** (banks) | whole program |
| L2 | 0x80030A28..0x80030ABC | s6 (j) | `< 10` @0x80030AB8 | 10 | contains L3 |
| L3 | 0x80030A3C..0x80030AA8 | s3 (k) | `< 4` @0x80030A94 | 4 | wrap_sprt + clut sh |
| L4 | 0x80030C44..0x80030C94 | s6 | `< 4` @0x80030C90 | 4 | poly/packet group |
| L5 | 0x80030CC8..0x80030D18 | s6 | `< 5` @0x80030D14 | 5 | poly/packet group |
| L6 | 0x80030F2C..0x80030F64 | s6 | `< 3` @0x80030F60 | 3 | poly/packet group |
| L7 | 0x80031040..0x8003109C | s6 | `< 3` @0x8003108C | 3 | poly/packet group |
| L8 | 0x800310CC..0x80031108 | s6 | `< 10` @0x800310FC | 10 | poly/packet group |
| L9 | 0x8003118C..0x800311E4 | s6 | `< 4` @0x800311D8 | 4 | poly/packet group |
| L10 | 0x800312CC..0x80031318 | s6 | `< 2` @0x80031314 | 2 | poly/packet group |
| L11 | 0x80031394..0x80031430 | s6 | `< 13` @0x8003142C | **13** | lookup+GetTPage+wrap_sprt |

L11's 13 matches the B54B VRAM-upload count (13 channel-1 entries) —
almost certainly the same material enumeration.

No forward branches exist: every loop is closed, all loop-exit paths
fall through; there is exactly one exit sequence (epilogue 0x800314B0).

## Frame (88 bytes)

```text
sw ra,84(sp); sw s8..s0,80..48(sp)   (10 saved + ra)
locals: 16(sp) byte  <- lb 0x8009CD90+0  (font/descriptor triple, sign-extended)
        17(sp) byte  <- lb 0x8009CD90+1
        18(sp) byte  <- lb 0x8009CD90+2
        24(sp) byte  =  outer loop counter i (0..1)
        32(sp) half  =  GetClut(304,504) = 0x7E13 (clut id, reused by all groups)
        40(sp) word  =  scratch v1 around the L3 body
```

## Registers (entry→use)

```text
s8 = GetTPage(0,1,256,480) & 0xFFFF = 0x34  (sprite tpage mode, most wrap_sprt a1)
s7 = 128 (0x80)                     vertex byte constant, sb to +4/+5/+6 of packets
s6 = group slot counter (rebased per group, & 0xFF)
s5/s2 = i*1400 base offsets          s4 = j (masked)
s3 = L3 k counter; later material ptr (lookup(139) result) and scratch
s0/s1 = per-group record/packet pointers
```

## Guest memory touched (machine-resolved lui/addiu census)

```text
reads  0x8009CD90 (3 lb, font triple)      writes none outside packets below
r/w    0x8009E0xx..0x8009EC40 (~35 lbu config/palette scalars)
reads  0x800B00E8 / 0x800B0130 / 0x800B0154 (state words)
packets 0x800B01C0 + i*1400 + j*140 + k*28   (L2/L3 sprite array, strides proven:
       1400 = ((i*3*4 - i)*16 - i)*8, 140 = ((j*9)*4 - j)*4, 28 = (k*8 - k)*4
       instruction-exact at 0x80030A18..A6C)
       + 0x1D6 clut halfword per packet (sh t1,470(at))
also   0x800B6920/28, 0x800BE9F0 (record tables: sh tpage/u/v fields)
palette ptr = func_8005DADC(139) = *(u32*)0x800A8030 + 0x800A8028 + 139*8
```

## Call census (42 jal, all native since B54I/GPU1)

```text
GetTPage x5  GetClut x2  SetSemiTrans x2  SetShadeTex x3
SetPolyF3/FT4 x2  SetPolyG4 x3  SetTile x2(direct)  SetSprt x2(direct)
wrap_sprt(func_800370DC) x18  wrap_tile(func_80037140) x1
lookup(func_8005DADC) x2  [B38h trampoline only via wrappers' fail paths]
```

No jalr, no other unresolved target. **func_80030894 is implementable
end-to-end with zero new dependencies.**

## Recommended B54K cut

The whole body at once is large but low-risk (no new callees, closed
loops, all constants known). Recommended instead, two cuts:

1. **B54K-A**: outer prologue + bank-0 L2/L3 sprite array build
   (0x80030894..0x80030AC8, ~55 words incl. nest) — the dominant packet
   mass; stop at 0x80030AC4 fall-through with the named strict boundary
   `func_80030894_L2L3_cut`.
2. **B54K-B**: remaining groups L4..L11 + epilogue (the rest).

## Reproduce

```sh
python3 pc_port/tools/b54j_30894_audit_oracle.py   # all structural checks
```
