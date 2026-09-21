# `func_80077AC4` — colour/palette key swap (24-bit payload, both directions)

Outcome: **MATCHED** on era `-O2 -G0` (`BYTE_EXACT` object; `LINK_EXACT`,
0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x682C4,0x68300)` = 15 words. VRAM `[0x80077AC4,0x80077B00)`.
- Fan-in **16** on the boot→Day-2 path (callers in the `0x80031xxx`/`0x80033xxx`
  field/clip cluster, e.g. `func_80033C70`-family via `D_800B0E38`).
- Preceded by the 1-word nop `func_80077ABC` and followed by asm at `0x68300`.

## Semantics (retail bytes)

```text
80077ac4  3c0600ff  lui  a2,0xff
80077ac8  34c6ffff  ori  a2,a2,0xffff    ; a2 = 0x00FFFFFF
80077acc  3c07ff00  lui  a3,0xff00       ; a3 = 0xFF000000
80077ad0  8ca30000  lw   v1,0(a1)
80077ad4  8c820000  lw   v0,0(a0)
80077ad8  00671824  and  v1,v1,a3
80077adc  00461024  and  v0,v0,a2
80077ae0  00621825  or   v1,v1,v0
80077ae4  aca30000  sw   v1,0(a1)        ; *a1 = hi(*a1) | lo(*a0)
80077ae8  8c820000  lw   v0,0(a0)
80077aec  00a62824  and  a1,a1,a2        ; note: masks the POINTER
80077af0  00471024  and  v0,v0,a3
80077af4  00451025  or   v0,v0,a1
80077af8  03e00008  jr   ra
80077afc  ac820000  sw   v0,0(a0)        ; *a0 = hi(*a0) | lo(a1)
```

C (`src/func_80077AC4.c`):

```c
void func_80077AC4(unsigned int *a0, unsigned int *a1) {
    register unsigned int lo asm("$6") = 0x00FFFFFFu;
    register unsigned int hi asm("$7") = 0xFF000000u;

    *a1 = (*a1 & hi) | (*a0 & lo);
    *a0 = (*a0 & hi) | ((unsigned int)a1 & lo);
}
```

The second statement's `lo` operand is the *pointer* `a1` (retail masks `$a1`
itself), which is why the RHS reads as-such and the function returns the `*a0`
store in the `jr` delay slot.

## Lever: pin the two mask constants to `$6`/`$7`

Retail materializes `0x00FFFFFF` into `$a2` and `0xFF000000` into `$a3` **once**,
first, and serves both statements from them. A plain C body makes cc1 mirror the
two `lui/ori` pairs in the opposite register order (`$7` then `$6`) and swap the
two `and` operand positions (7 mismatched words, `SIZE_MISMATCH C=0x40`).
Declaring the masks as `register … asm("$6")` / `asm("$7")` locals reproduces
retail exactly (`BYTE_EXACT`).

Rungs/shapes tried: `-O2 -G0` plain (7), source-order and OR-operand swaps
(7–15), `unsigned int t = *a0` temp (14), named constants (7). Only the asm
register pins reach `BYTE_EXACT`.

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80077AC4.c 0x80077AC4 0x3C -O2 -G0
BYTE_EXACT (ignoring gas align pad)
```

## Link-level proof

```text
python3 tools/analysis/era_link_check.py src/func_80077AC4.c 0x80077AC4 0x3C -O2 -G0
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The leaf has no undefined symbols, so link-level proof is object-identity
(gas pads the 15-word function to 0x40; the checker ignores trailing zero pad).

## Registration

- Source `src/func_80077AC4.c`; YAML `- [0x682C4, c, func_80077AC4]` (carved
  from the former `0x682BC` asm region; resume asm at `0x68300`).
- Profile: default `era_o2_g0` (`-O2 -G0`).
