# func_8003708C — PARKED (mflo/mfhi order + unsigned multiply)

7 words, VRAM `0x8003708C`, file `0x2788C.s`. `(a0 * a1) >> 16` — the high half
of a multiply, i.e. a fixed-point product:

```
mult  a0,a1
mflo  v0
mfhi  v1
srl   v0,v0,0x10
sll   v1,v1,0x10
jr    ra
or    v0,v1,v0
```

Best C attempt: **5 word mismatches**. cc1 emits `mfhi` **before** `mflo`
(retail has `mflo` first), regardless of whether the source is
`(unsigned long long)a*b >> 16`, `(long long)a*b >> 16` with a
`(unsigned long long)` cast, or an `"=l"/"=h"` asm constraint pair — and an
`unsigned` operand type additionally selects `multu` (`funct 0x19`) where retail
uses `mult` (`funct 0x18`), so the operands are **signed**. Invariant across
`-O1/-O2`, `-G0/-G8`, `-fno-schedule-insns2`, and both return-expression orders
(`(lo>>16)|(hi<<16)` and `(hi<<16)|(lo>>16)`). With the asm form cc1 even
reuses one register for both halves (`mfhi $6` / `sll $6,$6,16` /
`mflo $6`), which is a different allocation failure on top of the order issue.
