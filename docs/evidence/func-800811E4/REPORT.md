# `func_800811E4` — CD read poll (issue-vsync timeout / pending count)

Outcome: **MATCHED** on era `-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(a new opt-in maspsx delay-slot fill; see below). Integrated as matching-C
leaf 565. Linked at its retail VMA with its four referenced symbols defined,
the object `.text` is **byte-identical** to retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x719E4,0x71A54)` = `0x70` bytes = 28 words.
- VRAM span `[0x800811E4,0x80081254)`.
- Canonical `jr ra` at `0x8008124C` with the frame teardown
  `addiu sp,sp,32` in the delay slot at `0x80081250`.
- Preceded by an asm span (`0x7155C..0x719E4`, carved); followed by
  matched-C `func_80081254` at `0x80081254`.
- 9 retail `jal 0x800811E4` (`0C020479`) call sites:
  `0x8006E7F0`, `0x8006E8E0`, `0x8006ED88`, `0x8006EE5C`, `0x8006EEE8`,
  `0x8006EFB8`, `0x8006F0F0`, `0x8006F1A0`, `0x80081D50` — the boot CD
  read/poll family incl. `func_8006E7E8`/`func_8006E834`.

## Semantics (from retail bytes)

```text
800811e4  27bdffe0  addiu sp,sp,-32
800811e8  afb10014  sw    s1,20(sp)        ; s1 = p
800811ec  00808821  move  s1,a0
800811f0  2404ffff  li    a0,-1
800811f4  afbf0018  sw    ra,24(sp)
800811f8  0c01ce91  jal   func_80073A44    ; VSync(-1) -> v0
800811fc  afb00010  sw    s0,16(sp)        ; delay
80081200  3c04800a  lui   a0,0x800a        ; a0 = &D_8009B6C4 (issue vsync)
80081204  2484b6c4  addiu a0,a0,-18748
80081208  8c830000  lw    v1,0(a0)
8008120c  00000000  nop
80081210  246304b0  addiu v1,v1,1200
80081214  0062182a  slt   v1,v1,v0         ; (issue + 1200) < vsync ?
80081218  10600005  beqz  v1,0x80081230
8008121c  00000000  nop
80081220  0c02049a  jal   func_80081268    ; timeout abort
80081224  2410ffff  li    s0,-1            ; delay: result -1
80081228  0802048d  j     0x80081234
8008122c  00000000  nop
80081230  8c90fff0  lw    s0,-16(a0)       ; else s0 = D_8009B6B4 (pending)
80081234  0c01fd82  jal   func_8007F608    ; PROBABLE DsDataSync (result discarded)
80081238  02202021  move  a0,s1          ; delay
8008123c  02001021  move  v0,s0
80081240  8fbf0018  lw    ra,24(sp)
80081244  8fb10014  lw    s1,20(sp)
80081248  8fb00010  lw    s0,16(sp)
8008124c  03e00008  jr    ra
80081250  27bd0020  addiu sp,sp,32        ; delay slot: frame teardown (ASPSX fill)
```

C shape (`src/func_800811E4.c`):

```c
int func_800811E4(unsigned char *p) {
    int vsync = func_80073A44(-1);
    int *base = &D_8009B6B4.issue;   /* 0x8009B6C4 */
    int status;
    asm volatile("" : "=r"(base) : "0"(base)); /* zero-code: keep base in $a0 */
    if (base[0] + 1200 < vsync) {
        func_80081268();
        status = -1;
    } else {
        status = base[-4];            /* D_8009B6B4, 0x10 below issue */
    }
    func_8007F608(p);
    return status;
}
```

## Two levers

### 1. Address retention (one `$a0` base for the two adjacent words)

Retail materializes the base once (`lui $4,0x800a` / `addiu $4,$4,0xB6C4`)
and serves both `lw $3,0($4)` and the else-branch `lw $16,-16($4)` from it.
GCC folds a bare two-symbol or `sym[i]` access pair into two independent
`lui`/`lw` address pairs, and the `-O0..-O3`, `-Os`, sched1/sched2 and
delayed-branch rungs all reproduce the fold. Reaching the words through one
`&D_8009B6B4.issue` pointer and adding a zero-code `asm volatile("" : "=r"
: "0")` barrier stops copy-propagation from folding the pointer back into
the loads, so cc1 emits the single `la $4,D_8009B6C4` and both offsets
relative to it. The barrier emits no instructions (same technique as the
`func_8006E834` register split).

### 2. Epilogue delay-slot fill (new maspsx knob, class of the store fill)

With callee-saved restores, cc1 always leaves the frame teardown **before**
the return jump and GNU as emits a `nop` slot:

```text
move  v0,s0
lw    ra,24(sp)
lw    s1,20(sp)
lw    s0,16(sp)
addu  sp,sp,32      <- pre-jump
j     $31
nop
```

Retail has the 28-word form with `addiu sp,sp,32` **in** the slot. This is
the ASPSX reorder-mode scheduler moving the trailing stack adjust into the
delay slot, the same class as the already-vendored
`MASPSX_FILL_STORE_DELAY_SLOT` (patch 1) — cc1 cannot do it here, and no
compiler flag rung moves it (`-O1/-O2/-O3/-Os`, `-fschedule-insns`,
`-fschedule-insns2`, `-fno-schedule-insns2`, `-fno-delayed-branch`,
`-freorder-blocks` all byte-identical pre-fill).

ROM cross-check (reproducible scan of the retail EXE): for every word pair
`0x03E00008` (`jr $31`) immediately followed by an `addiu sp,sp,imm`
(`(w>>16)==0x27BD`, `imm!=0`) there are **272** filled epilogues; counting
contiguous `lw …,offset($sp)` restores immediately before the `jr`, the
restore histogram is
`{0:112, 2:49, 3:48, 4:15, 5:17, 6:9, 7:7, 8:4, 9:4, 10:6, 12:1}` — so 48
have exactly the three-restore shape used here, and 112 have none (the
no-callee-saved shape cc1 can already fill). The knob is therefore opt-in
per leaf, and flag-off output is byte-identical.

New vendored patch (patch 3, `tools/era/maspsx/maspsx/__init__.py`):
`fill_epilogue_delay_slot` (ctor arg or env
`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`) rewrites an `addu/addiu $sp,$sp,N`
immediately before a bare `j $31` into a `.set noreorder` block
`j $31` / `addiu $sp,$sp,N`, consuming the original jump line. Guards:
only `$sp,$sp,imm`; only when the next non-blank, non-comment input line is
exactly `j $31` (labels and `.set` lines block it). Durable test:
`tools/era/maspsx/tests/test_fill_epilogue_delay_slot.py` (5 cases); the
full maspsx suite from `tools/era/maspsx` is 167 OK via
`python3 -m unittest discover -t . -s tests`.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 \
  tools/analysis/era_leaf_match.sh src/func_800811E4.c 0x800811E4 0x70 -O2 -G0
```

Result: `MISMATCHES=6`, size `0x70` = ROM — every mismatch is a link-time
relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x800811F8` | `0c01ce91` | `0c000000` | R_MIPS_26 `func_80073A44` |
| `0x80081200` | `3c04800a` | `3c040000` | R_MIPS_HI16 `D_8009B6B4` |
| `0x80081204` | `2484b6c4` | `24840010` | R_MIPS_LO16 `D_8009B6B4` (+16 in imm) |
| `0x80081220` | `0c02049a` | `0c000000` | R_MIPS_26 `func_80081268` |
| `0x80081228` | `0802048d` | `08000014` | R_MIPS_26 `.text` (local L3) |
| `0x80081234` | `0c01fd82` | `0c000000` | R_MIPS_26 `func_8007F608` |

Link-level proof: assemble the leaf, link it at `0x800811E4` with
`--defsym func_80073A44=0x80073A44 --defsym func_80081268=0x80081268
--defsym func_8007F608=0x8007F608 --defsym D_8009B6B4=0x8009B6B4`, then
compare the linked `.text` word-for-word with the ROM:

```bash
BIN="$PWD/tools/mipsel-host/usr/bin"
export MASPSX_FILL_EPILOGUE_DELAY_SLOT=1
tools/era/gcc-2.7.2-psx/cpp src/func_800811E4.c > x.i
tools/era/gcc-2.7.2-psx/cc1 -quiet -O2 -G0 x.i -o x.s
python3 tools/era/maspsx/maspsx.py --aspsx-version=2.21 --dont-expand-li x.s > xm.s </dev/null
"$BIN/mipsel-linux-gnu-as" -EL -mips1 -mabi=32 -I include -o x.o xm.s
printf 'SECTIONS { .text 0x800811E4 : SUBALIGN(4) { *(.text) } }\n' > x.ld
"$BIN/mipsel-linux-gnu-ld" -EL -T x.ld -e 0 \
  --defsym func_80073A44=0x80073A44 --defsym func_80081268=0x80081268 \
  --defsym func_8007F608=0x8007F608 --defsym D_8009B6B4=0x8009B6B4 \
  -o x.elf x.o
"$BIN/mipsel-linux-gnu-objcopy" -O binary --only-section=.text x.elf x.bin
# compare x.bin (trimming any link-time pad up to the 3-word prologue) with
# build/extracted/disc1/SLUS_006.62[0x800811E4-0x80010000+0x800 : +0x70]
```

Result:

```text
linked .text 112 bytes (function at +0x0 pad), target 0x70, word mismatches=0
LINK_EXACT
```

All six relocations resolve to the retail values (`D_8009B6B4+16 =
0x8009B6C4`; local L3 = `0x80081234`).

## Registration

- Source: `src/func_800811E4.c`.
- YAML carve: `- [0x719E4, c, func_800811E4]` inside the former
  `0x7155C` asm span (which now ends at `0x719E4`).
- Build profile: `era_o2_g0_fill_epilogue_delay_slot` in
  `configs/USA/disc1_build_profiles.json` (`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`),
  assigned to `func_800811E4`.
- `python3 tools/build/disc1_plan.py --check` → 853 spans
  (565 c, 286 asm, 2 rodata).
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
- maspsx suite → 167 tests OK (`python3 -m unittest discover -t . -s tests`
  from `tools/era/maspsx`).
- Exact packed SHA-1 rebuild not run here (no `mipsel-linux-gnu-gcc`, no
  distrobox/docker); see ACTIVE_HANDOFF.
