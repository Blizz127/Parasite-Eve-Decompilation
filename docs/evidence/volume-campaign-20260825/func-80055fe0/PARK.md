# `func_80055FE0` — parked bit-test canonicalization

No integration and no matching-count change. Two bounded natural-C phrasings
were attempted; both optimize retail's explicit mask-and-test into a shorter
shift-and-mask form.

`DISPOSITION=PARKED-BIT-TEST-CANONICALIZATION`.

## Function hood and boundaries

Retail span `[0x467E0,0x4680C)`, VRAM `0x80055FE0`, is eleven words and ends
in canonical `jr ra` plus `sltu` in the return delay slot. Twelve exact direct
callers target its start:

```text
0x800430E8  0x80044328  0x800443F4  0x800444D0
0x800445E8  0x80044A8C  0x80044BD0  0x80049C90
0x80049D4C  0x800506A4  0x80050824  0x80050908
```

The preceding real `func_80055FB4` ends at VA `0x80055FD8/0x80055FDC`
with `jr ra` and its store delay slot. The following real `func_8005600C`
starts at VA `0x8005600C` with `addiu sp,sp,-0x30`. There is no padding at
either boundary.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; bitset query leaf |
| written-global Stage 0 | no global writes; reads the bitset pointer `D_8009D058` at `$gp+0x2E8` |
| coloring pressure | retail scales the signed word index in `$v0`, retains the pointer in `$v1`, then reuses `$v1` for the mask |
| `$v0` liveness | table address, table word, masked value, then boolean return |
| address retention | gp-relative pointer load; no symbolic address survives the table load |
| optimization signal | era `-O2 -G8`: gp-relative load and Psy-Q-era scheduling; `_gp=0x8009CD70`, so `D_8009D058-_gp=0x2E8` |
| loop/back-edge owner | none |

## Retail, all eleven words

```text
00041143  sra    v0,a0,5
00021080  sll    v0,v0,2
8F8302E8  lw     v1,0x2E8(gp)
3084001F  andi   a0,a0,0x1F
00431021  addu   v0,v0,v1
24030001  addiu  v1,zero,1
8C420000  lw     v0,0(v0)
00831804  sllv   v1,v1,a0
00431024  and    v0,v0,v1
03E00008  jr     ra
0002102B  sltu   v0,zero,v0
```

Retail explicitly constructs `1 << (index & 31)`, intersects it with the
selected table word, and converts the nonzero result to boolean.

## Attempt 1 — unsigned table/value form

```c
extern unsigned int *D_8009D058;

int func_80055FE0(int index) {
    return (D_8009D058[index >> 5] & (1u << (index & 0x1F))) != 0;
}
```

Era GCC 2.7.2-psx `-O2 -G8`, relocation normalized:

```text
00000000 <func_80055FE0>:
   0: 00041143  sra    v0,a0,0x5
   4: 8f830000  lw     v1,0(gp)       R_MIPS_GPREL16 D_8009D058
   8: 00021080  sll    v0,v0,0x2
   c: 00431021  addu   v0,v0,v1
  10: 8c420000  lw     v0,0(v0)
  14: 3084001f  andi   a0,a0,0x1f
  18: 00821006  srlv   v0,v0,a0
  1c: 03e00008  jr     ra
  20: 30420001  andi   v0,v0,0x1
```

After `R_MIPS_GPREL16` normalization, word 1 is `8F8302E8`. The compiler
reduces the expression to a logical variable shift followed by `& 1`, making
the body nine words. The first mismatch is word 1: candidate loads the pointer
where retail first scales the index.

## Attempt 2 — signed table/value form

```c
extern int *D_8009D058;

int func_80055FE0(int index) {
    return (D_8009D058[index >> 5] & (1 << (index & 0x1F))) != 0;
}
```

The object is identical to attempt 1 except for:

```text
  18: 00821007  srav   v0,v0,a0
```

Signedness changes logical `srlv` to arithmetic `srav`, but does not restore
the explicit `addiu 1; sllv; and; sltu` DAG or retail's ordering. It remains
nine words, so the second phrasing does not close the mechanism.

## Disposition

This is not compiler skew, padding, or a function-hood failure. It is a
specific era optimizer canonicalization: both ordinary expression types fold
an explicit variable mask test into variable shift plus low-bit extraction.
No pin or assembly was attempted. Revisit only with a new source-level
anti-canonicalization idiom or a compiler/tooling hypothesis.

Both candidate source states are represented in the evidence above; the final
source is preserved in stash
`park func_80055FE0 bit-test optimization attempts`. Matching C remains 318,
and this is consecutive park 1.
