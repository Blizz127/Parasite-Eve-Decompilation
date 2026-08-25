# `func_80055FB4` — exact gp-backed bitset setter

Leaf 320, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x467B4,0x467E0)`, VRAM `0x80055FB4`, is eleven words and
ends in canonical `jr ra` plus the bitset store in its delay slot. The exact
direct caller is:

```text
file 0x354C4 / VA 0x80044CC4: jal func_80055FB4
```

The preceding real `func_80055E14` ends at VA `0x80055FAC/0x80055FB0` with
`jr ra; nop`. The following proven real function `func_80055FE0` starts at
VA `0x80055FE0`; it remains asm under its separately recorded optimizer park.
No padding belongs to this span.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; bitset mutation leaf |
| written-state Stage 0 | writes one word through the bitset pointer `D_8009D058`; sibling query `func_80055FE0` is a proven direct reader of the same indexed bits |
| coloring pressure | signed word index stays in `$a1`; bit number stays in `$a0`; pointer/value use `$v0/$v1` |
| `$v0` liveness | gp-loaded bitset pointer, then constant/mask; no return value |
| address retention | computed element address remains in `$a1` through load and return-slot store |
| optimization signal | gp-relative pointer load and era register/scheduling fingerprint select `-O2 -G8` |
| loop/back-edge owner | none |

## C and flags

```c
extern unsigned int *D_8009D058;

void func_80055FB4(int index) {
    D_8009D058[index >> 5] |= 1u << (index & 0x1F);
}
```

Era GCC 2.7.2-psx plus maspsx 2.21 behavior, `-O2 -G8`. With
`_gp=0x8009CD70`, the `R_MIPS_GPREL16 D_8009D058` relocation normalizes to
`0x8009D058 - 0x8009CD70 = 0x2E8`. No pins, assembly, or special maspsx
switch is used.

Unlike parked query sibling `func_80055FE0`, the OR side effect prevents the
optimizer from replacing the explicit variable mask with shift-and-bit
extraction.

## Full eleven-word comparison

| word | retail | candidate after relocation | instruction |
|---:|---:|---:|---|
| 0 | `00042943` | `00042943` | `sra a1,a0,5` |
| 1 | `00052880` | `00052880` | `sll a1,a1,2` |
| 2 | `8F8202E8` | `8F8202E8` | `lw v0,0x2E8(gp)` |
| 3 | `3084001F` | `3084001F` | `andi a0,a0,0x1F` |
| 4 | `00A22821` | `00A22821` | `addu a1,a1,v0` |
| 5 | `24020001` | `24020001` | `addiu v0,zero,1` |
| 6 | `8CA30000` | `8CA30000` | `lw v1,0(a1)` |
| 7 | `00821004` | `00821004` | `sllv v0,v0,a0` |
| 8 | `00621825` | `00621825` | `or v1,v1,v0` |
| 9 | `03E00008` | `03E00008` | `jr ra` |
| 10 | `ACA30000` | `ACA30000` | `sw v1,0(a1)` |

```text
00000000 <func_80055FB4>:
   0: 00042943  sra    a1,a0,0x5
   4: 00052880  sll    a1,a1,0x2
   8: 8f820000  lw     v0,0(gp)       R_MIPS_GPREL16 D_8009D058
   c: 3084001f  andi   a0,a0,0x1f
  10: 00a22821  addu   a1,a1,v0
  14: 24020001  li     v0,1
  18: 8ca30000  lw     v1,0(a1)
  1c: 00821004  sllv   v0,v0,a0
  20: 00621825  or     v1,v1,v0
  24: 03e00008  jr     ra
  28: aca30000  sw     v1,0(a1)
```

## Carve and packed-span proof

The active asm span was `[0x44AA0,0x486CC)`, size `0x3C2C`:

```text
prefix asm: 0x467B4 - 0x44AA0 = 0x1D14
C leaf:     0x467E0 - 0x467B4 = 0x002C
resume asm: 0x486CC - 0x467E0 = 0x1EEC
closure:    0x1D14 + 0x002C + 0x1EEC = 0x3C2C
```

The object has one zero alignment word at offset `0x2C`; trim validation
removes it and keeps the exact boundary-derived body.

Packed candidate and retail disassemble identically across both boundaries:

```text
80055fac: 03e00008  jr     ra
80055fb0: 00000000  nop
80055fb4: 00042943  sra    a1,a0,5
80055fb8: 00052880  sll    a1,a1,2
80055fbc: 8f8202e8  lw     v0,744(gp)
80055fc0: 3084001f  andi   a0,a0,0x1f
80055fc4: 00a22821  addu   a1,a1,v0
80055fc8: 24020001  li     v0,1
80055fcc: 8ca30000  lw     v1,0(a1)
80055fd0: 00821004  sllv   v0,v0,a0
80055fd4: 00621825  or     v1,v1,v0
80055fd8: 03e00008  jr     ra
80055fdc: aca30000  sw     v1,0(a1)
80055fe0: 00041143  sra    v0,a0,5
80055fe4: 00021080  sll    v0,v0,2
```

```text
retail:    4329040080280500e802828f1f0084302128a200010002240000a38c04108200251862000800e0030000a3ac
candidate: 4329040080280500e802828f1f0084302128a200010002240000a38c04108200251862000800e0030000a3ac
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
320
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 320 leaves
```

Result: `MATCHED=11/11`, first phrasing; consecutive-park count remains zero.
