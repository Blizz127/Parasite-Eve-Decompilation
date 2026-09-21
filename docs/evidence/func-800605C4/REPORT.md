# func_80060528 / func_8006055C / func_80060590 / func_800605C4 / func_800605F8

**Status: LINK_EXACT** (all five). 13 words each (0x34), no frame beyond the
0x18 ra save, file `0x50D28.s`, era `-O2 -G8`, profile `era_o2_g8`.

## Semantics

Five identical gp-counter wrappers, differing only in the `N` argument word
(6, 5, 4, 3, 2 respectively):

```c
extern int D_8009D124;
extern int D_8009D128;
extern void func_800602D0(int a, int b);
void func_80060528(int a) {
    volatile int *p = (volatile int *)&D_8009D128;
    func_800602D0(a, 6);
    D_8009D124 += 6;
    *p = *p;
}
```

`func_800602D0(a,b)` is a nonmatching 0x258-byte state machine (its own body
uses `a1` as a loop bound: `slt $v0,$s0,$s2`), so the second argument is real;
`a` is passed through from the caller unchanged.

`D_8009D124 = gp+0x3B4`, `D_8009D128 = gp+0x3B8` (`_gp = 0x8009CD70`).

## Retail shape

```
addiu sp,sp,-0x18
sw    ra,0x10(sp)
jal   func_800602D0
li    a1,N
lw    v0,0x3B4(gp)
lw    v1,0x3B8(gp)
addiu v0,v0,N
sw    v0,0x3B4(gp)
sw    v1,0x3B8(gp)
lw    ra,0x10(sp)
addiu sp,sp,0x18
jr    ra
```

Note the `D_8009D128` read is materialized as its own `lw v1,0x3B8(gp)` and
re-stored unchanged (a read-modify-write with a zero delta), *before* the
accumulator's `sw`. That is the signature of a **volatile read-modify-write**.

## Levers

1. **`volatile int *p = (volatile int *)&D_8009D128;` + `*p = *p;`** forces
   cc1 to emit the independent `lw $v1`/`sw $v1` pair, and the `volatile`
   qualifier makes sched1 emit the load early (right after the accumulator's
   `lw v0`, before the `+N`) and the store last — exactly retail's interleave.
   Plain `D_8009D128 = D_8009D128;` is eliminated outright by cc1 (`-O2`);
   a bare `extern volatile int` global fold put the `lw`/`sw` adjacent and
   after the accumulator store (`word mismatches=4`), and `D_8009D128++` /
   `--` produced an extra 2-word sequence (`9` mismatches).
2. **Two-argument callee declaration.** Declaring `func_800602D0(int a)` (one
   param) made cc1 assign the literal to `$a0` (`24040006`) instead of `$a1`.
   The frozen-callee prototype must carry both parameters even though this leaf
   only writes the second.
3. The five leaves are **carved contiguously** rather than one big C body,
   because each has its own `jal`: they are five distinct objects in retail.
