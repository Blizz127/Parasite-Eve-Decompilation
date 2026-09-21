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

## Batch 3 additions

### func_8006A2E8 (file 0x5AAE8, size 0x30)

Bounded `< 0x10` triple store (sh/sh/sb) returning 0. Era fills the `beqz`
delay slot with `addu v0,a1,zero`, shifting the whole tail one word:

```text
0x0004: retail 10400008 (beqz +8)  cand 10400007 (beqz +7)
0x0024: retail A0220DB1 (sb)       cand 03E00008 (jr ra)
```

`-O1 -G0` and `-fno-delayed-branch` did not close it. Parked.

### func_80018E84 (file 0x9684, size 0x30)

Two double-deref stores to D_800BD020/D_800BD022, return 1. Retail orders
load-store-load-store; era hoists the second pointer load:

```text
0x0004: retail 00000000 (nop)      cand 8C830004 (lw v1,4(a0))
0x0018: retail 00000000 (nop)      cand 8C620000 (lw v0,0(v1))
```

`volatile` on the destination globals did not prevent the hoist. Parked.

### func_8007E594 (file 0x6ED94, size 0x30)

Zero word 0, byte 4, a four-byte countdown-clear at 5..8, then words
0xC/0x10/0x14. Era does not produce retail's descending pointer loop
(`addiu v0,a0,3` / `sb 5(v0)` / `addiu v0,v0,-1`). 11 words differ. Parked.

### func_80082ADC (file 0x732DC, size 0x2C) — callback-table install

Retail materializes the table base once (`lui/addiu $v0,D_800A5AB4`) and
stores by offset (`sw $v1,0($v0)` / `+4` / `-4` / `+8` in the `jr` delay
slot). Both direct-array and local-pointer phrasings make cc1 re-materialize
`D_800A5AB4+n` for every store, and the relocation-normalized diff hides the
extra `lui/addu` pairs behind zeroed words. One non-relocated word differs;
the address-computation shape differs. Draft at `nonmatch/func_80082ADC.c`.

### Batch 5 additions

- `func_800762A0` (0x66AA0, 0x1C): pack `0xE5000000 | (a0&0x7FF) | ((a1&0x7FF)<<11)`.
  Era orders the constant/operand materialization differently (3 words differ).
- `func_80087798` (0x77F98, 0x24): two 0x7FFF-masked stores at 0x1F801C00+index*16.
  6 words differ; era does not keep the base in `$v0` with `sh` in the delay slot.
- `func_80038CE4` (0x294E4, 0x28): two-stage byte lookup through D_80091A28.
  Era hoists/orders the two `lbu`/`addu` pairs differently (3 words differ).

Drafts preserved under `nonmatch/`.
