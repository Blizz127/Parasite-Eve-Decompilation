# `func_8006EC84` — offset-table walker (int counter / short index)

Outcome: **MATCHED** on era `-O2 -G0` (YAML default profile), pinless (no
`register asm`, no barrier, no maspsx knob). Integrated as matching-C leaf
566. Linked at its retail VMA the object `.text` is **byte-identical** to
retail (`LINK_EXACT`).

## Function hood and retail span

- File span `[0x5F484,0x5F4EC)` = `0x68` bytes = 26 words.
- VRAM span `[0x8006EC84,0x8006ECEC)`.
- Canonical `jr ra; nop` at `0x8006ECE4/0x8006ECE8`.
- Preceded by matched-C `func_8006EC6C` at `0x8006EC6C` (size 0x18);
  followed by the 214-word `func_8006ECEC` loader (still asm).
- Carved out of the former `0x5F484` asm span (now begins at `0x5F4EC`).

## Semantics (from retail bytes, SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`)

```text
8006ec84  27bdffd8  addiu sp,sp,-40
8006ec88  afb1001c  sw    s1,28(sp)
8006ec8c  00808821  addu  s1,a0,zero        ; s1 = base
8006ec90  afb20020  sw    s2,32(sp)
8006ec94  00a09021  addu  s2,a1,zero        ; s2 = count
8006ec98  afb00018  sw    s0,24(sp)
8006ec9c  00008021  addu  s0,zero,zero      ; i = 0
8006eca0  1a40000b  blez  s2,0x8006ecd0
8006eca4  afbf0024  sw    ra,36(sp)         ; delay
8006eca8  00101400  sll   v0,s0,0x10
8006ecac  00021383  sra   v0,v0,0xe         ; v0 = (short)i * 4
8006ecb0  00511021  addu  v0,v0,s1
8006ecb4  8c440000  lw    a0,0(v0)          ; a0 = *(int *)(base + i*4)
8006ecb8  26100001  addiu s0,s0,1
8006ecbc  0c01c634  jal   func_800718D0
8006ecc0  02242021  addu  a0,s1,a0          ; delay: base + a0
8006ecc4  0212102a  slt   v0,s0,s2
8006ecc8  1440fff8  bnez  v0,0x8006ecac
8006eccc  00101400  sll   v0,s0,0x10        ; delay
8006ecd0  ... epilogue (ra/s2/s1/s0 restores, addiu sp,sp,40, jr ra; nop)
```

C shape (`src/func_8006EC84.c`):

```c
extern void func_800718D0(int);

void func_8006EC84(int base, int count) {
    int i;
    for (i = 0; i < count; i++)
        func_800718D0(base + *(int *)(base + (short)i * 4));
}
```

## The one lever: `int` counter, `short` table index

The counter is a signed `int` (retail `blez`/`slt` compare `s0` directly),
but the table index is sign-extended as a `short`: retail folds the
sign-extension and the `*4` scale into `sll $v0,$s0,16 ; sra $v0,$v0,14`.
Writing the access as `((int *)base)[i]` (plain int index) makes cc1 emit a
byte-pointer strength-reduction (`addu s1,s1,4` inside the loop) and a
different frame — a 23-word mismatch that is invariant under the flag
rungs. Casting only the index (`(short)i`) restores retail's carried
`sll 16` induction value and the `addu v0,v0,s1` addressing, matching all
25 words modulo the `func_800718D0` call relocation. A `short i` loop
counter is **not** equivalent: it re-sign-extends the counter for the
`slt`, adding an `sll 16 ; sra 16` pair and shrinking the frame.
This is the same access rule already established for the neighbouring
`func_8006EC6C` reader (`short index` parameter).

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006EC84.c 0x8006EC84 0x68 -O2 -G0
```

Result: `MISMATCHES=1` — the only differing word is the call relocation field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006ECBC` | `0c01c634` | `0c000000` | R_MIPS_26 `func_800718D0` |

Extra `.text` bytes (`C=0x70` vs `ROM=0x68`) are GNU as alignment pad,
trimmed to the YAML span size by `tools/trim_elf_section_pad.py` during
`scripts/build_us.sh`.

Link-level proof: assemble the leaf, link it at `0x8006EC84` with
`--defsym func_800718D0=0x800718D0`, then compare the linked `.text`
word-for-word with the ROM:

```text
linked .text 112 bytes, target 0x68, word mismatches=0
LINK_EXACT
```

## Registration

- Source: `src/func_8006EC84.c`.
- YAML carve: `- [0x5F484, c, func_8006EC84]` + `- [0x5F4EC, asm]`
  (replacing the former `0x5F484` asm span).
- Build profile: default `era_o2_g0` (no assignment entry needed).
- `python3 tools/build/disc1_plan.py --check` → 854 spans
  (566 c, 286 asm, 2 rodata).
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
- Exact packed SHA-1 rebuild not run here (no `mipsel-linux-gnu-gcc`, no
  distrobox/docker); see ACTIVE_HANDOFF.
