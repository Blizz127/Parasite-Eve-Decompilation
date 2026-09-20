# agent/decompile-med — medium/large queue attempt (2026-09-20)

Base `0f984996` (708 leaves). Goal: add byte-exact leaves from the size-ranked
queue because the small-function quick-win pool is exhausted. **No new leaf was
matched in the time**; the two attempts below are recorded with first-mismatch
data so the next agent does not repeat them blindly.

## func_80036E7C — 0x2767C, 0x100 (256 B), straight-line, 0 `jal`

Packs three `(a0[1] % 60)` fields into `a1[0]`:
`a1[0] &= 0xFFF00000`, `|= (x%60)*216000 & 0xFFFFF`, `&= 0xFC0FFFFF`,
`|= ((x%60)*3600 & 0x3F) << 20`, `&= 0x3FFFFFF`, `|= (x%60)*15 << 28`.
The arithmetic is confirmed from the asm (`multu 0x88888889` / `mfhi` / `srl 5`
is cc1's unsigned `/60`; `x*7<<5 + x` = `x*225`, `*15<<6` = `*960`; so
`x*216000` decomposes exactly as retail does).

Best candidate (`unsigned int a2 = *a1 & 0xFFF00000; … v = a0[1] % 60; …`):
**31/64 words differ**, all instruction-shape-preserving register permutations.
First mismatch `0x0038`: retail `00E23823` (`subu a3,a3,v0`) vs candidate
`00624823` (`subu t1,v1,v0`). Variants tried: separate `a3` local → 41 differ;
in-place `a1[0] &= …` accumulator → 74 differ. The best keeps the running `a2`
in a register (retail never reloads `a1[0]`).

Next lever to try: force the modulo temp into `$a3` (retail reuses the loaded
`a0[1]` register for the remainder) and the accumulator into `$a2`, e.g. by
making those the only two live locals and storing once per field.

## func_800D401C — 0xC481C, 0x120 (288 B), no direct calls

8-slot `D_800E2368` allocator (stride 0xC from +0x20, `0xFFFF` = free), then an
indirect `jalr` through the `[s0+0x80][a0]` function table. Best candidate:
**63/72 words differ** — the slot-search loop shape is wrong. Retail tests
`h[s1] == 0xFFFF` first, then `a1++`, then `a1 < 8` (with `s1 += 0xC` in the
`bnez` delay slot), and materialises `8` into `v0` in the `beq` delay slot, so
`beq a1,v0` after the loop is exactly `a1 == 8`. The C loop must be written to
reproduce that test order before this function is worth another attempt.

## Method / commands

```
cd /tmp/pe-agent-decomp-med
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp-med && python3 tools/analysis/try_leaf.py build/cand36E7C.c 0x2767C 0x100'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp-med && python3 tools/analysis/try_leaf.py build/candD401C.c 0xC481C 0x120'
```

No `src/` or `configs/USA/disc1.yaml` change; the matching build is untouched
(708 leaves, exact SHA-1). No matching claim is made.
