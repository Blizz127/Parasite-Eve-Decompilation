# func_800702DC

- **VRAM**: 0x800702DC
- **File offset**: 0x60ADC (size 0x118)
- **Build profile**: era_o2_g0 (default `-O2 -G0`)
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
First half of the battle-slot scrub (indices 0..0xA). For each of eleven
records it calls `func_8006FC18(i,0,1)`; a non-zero result aborts the walk and
is returned. Otherwise, when `i < 0x16` (unsigned), it selects the record base
(`D_800942E8 + off_b` for `i >= 0xB`, `D_800942E4 + off_a` for `i < 0xB`); if
byte +1 is 0x72 it wipes the seven words at `D_800E0EF0+0x1B0` and clears bit
0x10000 of `D_800B0CD8`, then writes the 0/FF/FF/FF header and zeroes +4/+8.

## Method
The loop-back index compare is **signed** while the two range guards are
**unsigned** (`int i` + `(unsigned int)i < 0x16`/`>= 0xB`). The `&D_800E0EF0`
base is hoisted into a callee-saved register before the loop. To reproduce
retail's unfilled `bnez $a2` delay slot the early exit must copy the result to
a distinct local and jump to a label that the fall-through also reaches
(`if (temp_v0 != 0) { result = temp_v0; goto out; } … out: return result;`);
with a direct `return temp_v0` cc1 duplicates the copy into the delay slot
(1 word off).

## Evidence
try_leaf `WORDS MATCH (+8 pad bytes)`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (889
registered C leaves)`, `VERIFY_US=PASS`.
