# Non-matches — agent/decompile-continue (2026-09-20)

Four candidates were triaged with `tools/analysis/try_leaf.py` and did not
reproduce retail under era `-O2 -G0` (default, three-word, or passthrough
gate). Their C drafts are preserved under `nonmatch/`; the precise first
mismatch (relocation-normalized) is recorded here. None is registered in
`configs/USA/disc1.yaml`.

## func_80078C94 (file 0x69494, size 0x24) — three-word struct copy

Intent: copy three source words (+0/+4/+8) to dest +0x14/+0x18/+0x1C and
return dest. Retail proves the load-before-store ordering
(`lw a1+0 / lw a1+4 / lw a1+8 / sw a0+0x14 / sw a0+0x18 / sw a0+0x1C`).
The era compiler interleaves load/store and uses a 48-byte frame:

```text
0x0004: retail 8CA90004 (lw t1,4(a1))  cand 00801021 (move v0,a0)
0x0008: retail 8CAA0008 (lw t2,8(a1))  cand AC430014 (sw v1,0x14(a0))
0x000C: retail AC880014 (sw t0,0x14(a0)) cand 8CA30004 (lw v1,4(a1))
```

First mismatch `0x0004`. Needs register-allocation/scheduling control not yet
available; parked.

## func_8007C444 (file 0x6CC44, size 0x34) — 32-byte record zero loop

Intent: zero `count` 32-byte records from record index `base`, reloading
pointer global `D_800C0DC8` each iteration. Era hoists the pointer out of the
loop and reorders the index add:

```text
0x0008: retail 00C41021 (addu v0,a2,a0)  cand 00003021 (move a2,zero)
0x000C: retail 24C60001 (addiu a2,a2,1)  cand 00861021 (addu v0,a0,a2)
0x0010: retail 3C03800C (lui v1,%hi)     cand 24C60001 (addiu a2,a2,1)
```

First mismatch `0x0008`. The retail loop keeps the pointer load inside the
body; expressing that without an anti-hoist barrier is not yet found.

## func_80083578 (file 0x73D78, size 0x28) — field bit poll

Intent: `while ((*(volatile u16 *)(D_8009B788 + 4) & 2) == 0);`. The body is
semantically exact; only the back-branch displacement differs because
cc1/maspsx places the `lw` load-delay `nop` after the loop label:

```text
0x0018: retail 1040FFFC (beqz v0,-4)  cand 1040FFFB (beqz v0,-5)
```

Retail's loop label is on the `lhu`; the candidate's is one word earlier (on
the delay-slot nop). One-instruction label-placement skew; parked.

## func_800858B0 (file 0x760B0, size 0x38) — bounds-checked table read

Intent: `i = index & 0xFFFF; if (i < 3) return *(u16 *)(D_8009B7D0 + i*16);
return 0;`. Retail compares the *masked* value with signed `slti` and scales
that same register; era compares the unmasked `$a0` with `slti`:

```text
0x0004: retail 28620003 (slti v0,v1,3)  cand 28820003 (slti v0,a0,3)
0x0008: retail 10400008 (beqz v0,+8)    cand 14400003 (bnez v0,+3)
0x000C: retail 00031900 (sll v1,v1,4)   cand 00041100 (sll v0,a0,4)
```

First mismatch `0x0004`. The `if (i < 3) … return 0` and `if (i >= 3) return 0`
phrasings both diverge; parked.
