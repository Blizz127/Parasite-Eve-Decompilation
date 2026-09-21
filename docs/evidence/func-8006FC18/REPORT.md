# `func_8006FC18` — record handler invoke + teardown

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x60418,0x60614)` = `0x1FC` bytes = 127 words.
- VRAM span `[0x8006FC18,0x8006FE14)`.
- Follows the PARKed `func_8006F9F0`; precedes `func_8006FE14`.

## Semantics (from retail bytes)

`int func_8006FC18(int idx, int a1, int a2)`:

1. `if ((unsigned)idx >= 0x16) return -0x16;`
2. Arena select (pointer globals): `>= 0xB → D_800942E8 + (idx-0xB)*0x10C`,
   else `D_800942E4 + idx*0xA0C`.
3. `p[0] == 0 || p[0] == 6` → return 0.
4. Match filter: if `a2 == 0` and `*(int *)(p + 8) != a1` → return 0.
5. Handler byte `h = p[1]`: `>= 0xC0` → `-0x17`, clamp to `0x55`.
6. `D_800942E0[h] == 0` → `-0x18`;
   `*(int *)((char *)D_800942E0[h] + 0x14) == 0` → `-1`.
7. Call the offset-`0x14` handler `fn(p, h, a2)` and keep the result.
8. Re-run the arena lookup; if `p[1] == 0x72` zero the seven
   `D_800E10A0[0..6]` slots and clear `D_800B0CD8` bit `0x10000`.
9. Reset the record: `p[0]=0`, `p[1..3]=0xFF`, `*(int*)(p+4)=0`,
   `*(int*)(p+8)=0`. Return the handler result.

## Levers

1. **Second arena lookup uses a distinct local `q`, not `p`.** Retail
   computes the second pointer into `$a1` (the first one lived in `$a0`);
   reusing `p` makes cc1 allocate both to `$a0` and produces 11 register
   mismatches (`0x8006FD90` onward).
2. **`D_800942E0` typed `extern void **`** (see the sibling reports).
3. `p[0] == 0 || p[0] == 6` written in that order gives `beqz` + `li 6` /
   `beq`.
4. `if (a2 == 0 && *(int *)(p + 8) != a1)` — the `&&` short-circuit keeps
   the `+8` load *after* the `bnez $a2` as retail does.
5. The redundant `if ((unsigned)idx < 0x16)` around the teardown is
   retained by cc1 2.7.2 (not folded) and must be written.

## Single-leaf object

```text
AS=…/mipsel-linux-gnu-as OBJDUMP=… OBJCOPY=… \
LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/era_leaf_match.sh src/func_8006FC18.c 0x8006FC18 0x1FC -O2 -G0
```

Result: `SIZE_MISMATCH C=0x200 ROM=0x1FC` (GNU as pad), `MISMATCHES=23`,
first at `0x8006FC30`. All 23 differing words carry relocations; none
remain after resolution.

Relocated symbols: `D_800942E4`, `D_800942E8`, `D_800942E0`,
`D_800E10A0`, `D_800B0CD8`.

Link-level proof:

```text
linked .text 512 bytes, target 0x1fc, word mismatches=0
LINK_EXACT
```

## Registration

- Source `src/func_8006FC18.c`; YAML `- [0x60418, c, func_8006FC18]`
  between the `0x601F0` asm (PARKed `func_8006F9F0`) and `0x60614`.
- Default `era_o2_g0`; no profile entry needed.
