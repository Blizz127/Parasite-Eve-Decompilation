# `func_8006F9F0` — PARK (delayed-branch fill residual)

Status: **PARKED** (not counted). VRAM `0x8006F9F0`..`0x8006FC18`
(file `[0x601F0,0x60418)`, `0x228` bytes = 138 words).

## What is known

The record-state dispatcher is fully understood:

- id `>= 0x16` -> `-0x13`.
- state byte (record offset 0) must be in `{1,2,4,5}`; otherwise return 0.
  - `4` -> becomes `5`, return 0.
  - `5` -> reset the record (bytes `00 FF FF FF`, words `+4`/`+8` = 0) and,
    when the handler byte is `0x72`, also clear the `D_800E10A0` voice table
    (`[0x6C,0x73)`) and bit 16 of `D_800B0CD8`; return 0.
  - `1` -> becomes `2`, then falls into the handler call.
  - `2` -> falls into the handler call.
- Handler byte (offset 1): `>= 0xC0` -> `-0x14`, clamp to `0x55`, null
  `D_800942E0[h]` -> `-0x15`. Calls handler field `+0x10` with the record,
  increments the record word at `+4`, returns the callee's result; a null
  handler pointer returns `-1`.

Arena selection is the same two pointer globals as the matched siblings
(`D_800942E4` stride `0xA0C` ids `0..0xA`, `D_800942E8` stride `0x10C` ids
`0xB..0x15`), and the condition must be written as the positive
`if (...) { ... } return 0;` form so cc1 emits the two `sltiu` compares with
the body as the `bnez` target (the negated early-return form gives the same
instruction count but a different `li` placement).

## Exact residual (era `-O2 -G0`, best rung)

`era_leaf_match.sh src/func_8006F9F0.c 0x8006F9F0 0x228 -O2 -G0`
-> `SIZE_MISMATCH C=0x230 ROM=0x228`, `MISMATCHES=102`.

The arena select, state checks, `0x72` reset block and handler call all land
byte-for-byte after relocation. The residual is at the state range test:

```text
retail                                generated (C)
8006fa70  bnez  v0,0x8006FA8C         8006fa70  bne  v0,0x8006FA90
8006fa74  addiu v0,v1,-4   (delay)    8006fa74  li   v0,4        (delay)  +1 word
8006fa78  sltiu v0,v0,2               8006fa78  addu v0,v3,-4
8006fa7c  bnez  v0,0x8006FA8C         8006fa7c  sltiu v0,v0,2
8006fa80  nop                         8006fa80  bne  v0,0x8006FA90
8006fa84  j     0x8006FA84            8006fa84  li   v0,4        (delay)
8006fa88  addu  v0,zero,zero (delay)  8006fa88  j    0x8006FA84
                                      8006fa8c  addu v0,zero,zero
```

cc1/`reorg` materializes the `p[0] == 4` constant (`li v0,4`) **twice** —
once into the first branch delay slot and again before the `bne` — whereas
retail fills the first slot with the second condition's `addiu v0,v1,-4` and
materializes `4` only once (in the body). The two instructions/structure
otherwise agree; the extra word is the observed `C=0x230` vs `ROM=0x228`.

Rungs tried (all `SIZE_MISMATCH`, none better):
`-O2 -G0` (102), `-O2 -G0 -fno-schedule-insns2` (104, and mismatches from
offset 4), `-O1 -G0` (102, size `0x250`, offset 4), `-O1 -G0
-fschedule-insns2` (111). Source shapes tried: `st` local vs re-reading
`p[0]`, negated early-return vs positive-if, and `4`/`5` case ordering — all
byte-identical apart from relocation fields.

## Next lever to try

The residual is a `reorg`/`sched2` delay-slot fill choice (which constant
gets rematerialized into the first slot). A maspsx-style reorder knob (drop a
duplicate `li` before a branch and hoist the fall-through `addiu` into the
slot) is the plausible patch-4 class, but that is not warranted by a single
leaf yet.

## Registration

- `src/func_8006F9F0.c` is **not** present (no unmatched C may live in
  `src/`).
- Span remains in the `0x601F0` asm region; the matching-C count is
  unaffected.
