# PE-B54K-A — func_80030894 prologue + bank-0 L2/L3 sprite array

```text
PE-B54K-A SUCCESS — NAMED L2L3 CUT; 6AD40 UNPARKED THROUGH jal 30894
```

Base PE-B54J `3bddd76`. Production now translates:

1. `func_8006AD40` `0x8006B060..0x8006B0BC`: `D_800930F0` /
   dest+0x14C, the second `func_800718D0` walk of dest+0x180, and
   `jal func_80030894`. Exclusive end is `jal func_8006E7E8` at
   `0x8006B0BC` (the third live poll, not taken).
2. `func_80030894` `0x80030894..0x80030AC4` (140 words): prologue
   vectors, bank-0 record at `0x800BE9F0`, and the L2(j<10) ×
   L3(k<4) sprite array at `0x800B01C0`. Named cut
   `func_80030894_L2L3_cut`. First excluded word is
   `move a0, zero` (`0x00002021`) at `0x80030AC4` (group L4 setup).

Zero new callees. All 42 jal targets were native since B54I/GPU1;
this window uses six of them.

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
branch=phase6e-b-provider-frontier
base_b54j_commit=3bddd76
```

Uncommitted until an explicit commit request. Not pushed.

## Window (machine-derived from SHA-1 EXE)

```text
symbol     func_80030894 prefix
window     0x80030894..0x80030AC4 exclusive (140 words / 0x230)
file off   0x21094
sha1 exe   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
caller     jal func_8006AD40 @ 0x8006B0AC
cut        func_80030894_L2L3_cut; 6AD40 then parks at
           func_8006AD40_post30894_cut @ 0x8006B0BC
```

The B54J audit's "~55 words" was a working estimate; the exclusive
end at the first L4 instruction is **140 words**. Confirmed by
`(0x80030AC4-0x80030894)/4` against `build/disc1.candidate.exe`.

## Retail vectors (this window)

```text
GetTPage(0, 1, 0x100, 0x1E0) & 0xFFFF = 0x34   # s8 / wrap_sprt a1
GetClut(0x130, 0x1F8)                 = 0x7E13 # sp+32, per-packet clut
func_8005DADC(139)                             # palette record
SetPolyFT4(0x800BE9F0)                         # bank-0 record header
GetTPage(0, 0, 0x1C0, 0)              = 7      # record tpage halfword +0x16
SetSemiTrans(record, 1)                        # a1=1 is loaded AFTER the
                                               # second GetTPage clobbers a1
wrap_sprt x40  at 0x800B01C0 + j*140 + k*28
sh clut 0x7E13 at packet+0x16
           == 0x800B0000 + off + 0x1D6
```

Delay-slot facts consumed by the C:

- GetClut jal delay `andi $fp, $v0, 0xFFFF` captures the GetTPage
  return; `sh $v0, 32(sp)` after the call stores GetClut.
- Second GetTPage jal delay `sb $v0, 37($s0)` stores the **pre-call**
  u4/v4 adder (`lbu(s3+1)+lbu(s3+5)`).
- L2 back-edge delay `move $s3, $zero` resets k; host `for` does this.
- sp+40 save/restore of `i*12` around wrap_sprt is host-stack only.

Font triple `lb 0x8009CD90+0/+1/+2` and `s7=128` are written in the
prologue and unread before `0x80030AC4`. Host C omits them.

## 6AD40 unpark

B54G parked at `0x8006B060`. Reaching 30894 requires the next 23
words. Literal:

```text
0x8006B060  s0 = 0
0x8006B064  s1 = D_800930F0
0x8006B06C  s2 = -1
0x8006B080  jal func_8006E6A8     # dest = *(D_800B0CD8+0x14C)
0x8006B088  beq v0, s2, B070      # retry on -1 only
0x8006B090  s2 = 1
0x8006B094  s1 = -1
0x8006B098  bnez s0, B0B4         # canonical s0==0, taken fallthrough
0x8006B0A4  jal func_800718D0     # dest+0x180 (the EE issue's TIM)
0x8006B0AC  jal func_80030894
0x8006B0B4  beq s2, s1, B064      # not taken (1 != -1)
0x8006B0BC  jal func_8006E7E8     # EXCLUSIVE END (third poll)
```

No poll between the F0 issue and 30894. Busy bits stay armed
(`B54K-A D_800930F0 issue must re-arm the busy bits`). Host
first-sample collapse does not apply (no `func_8006E7E8` in this
window).

## Disc / emulator

USA Disc 1 BIN is at

```text
/var/home/blizz/Projects/Parasite-Eve-Decompilation/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin
```

(473 MiB, recorded in gitignored `local/pe_disc1.path`). This
checkout's `rom/image/` is still empty. No DuckStation/PCSX
binary is installed; the retail oracle for this rung is the
SHA-1-exact EXE interpreter, not an emulator screenshot.

Real-disc `--strict-stubs` now stops at `func_80030894_L2L3_cut`
from `func_80030894` (exit 1). `--bootstrap-disc --strict-stubs`
is unchanged: `func_8007F72C` from `func_800698D4`.

## Files

```text
pc_port/game/boot/func_80030894_port.c     # new prefix
pc_port/game/boot/func_8006AD40_port.c     # unpark B060..B0BC
pc_port/include/psx_compat.h              # prototype + named-cut comment
pc_port/CMakeLists.txt                    # GAME_SRCS
pc_port/tests/test_native.c               # 2 focused + retained frontier
pc_port/tools/b54ka_30894_l2l3_oracle.py  # independent 8-group oracle
pc_port/src/port_main.c                   # hold window after frontier stop
```

## Next rung

**PE-B54K-B** — remaining groups L4..L11 + bank-1 pass + epilogue
(`0x80030AC4..0x800314E4`). Zero new callees. First instruction is
`move a0, zero` at `0x80030AC4`. After a complete 30894 the 6AD40
third poll at `0x8006B0BC` becomes the live cut.

---

```text
base_b54j_commit=3bddd76
cut_30894=func_80030894_L2L3_cut @ 0x80030AC4
cut_6ad40=func_8006AD40_post30894_cut @ 0x8006B0BC
words_30894=140
words_6ad40_unpark=23
new_callees=0

SUCCESS
PE-B54K-A SUCCESS — NAMED L2L3 CUT; 6AD40 UNPARKED THROUGH jal 30894
```
