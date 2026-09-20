# func_800D401C closer (second pass) — register allocation will not budge

Target: `func_800D401C` (file `0xC481C`, size `0x120`, no direct calls) — 8-slot
`D_800E2368` allocator (stride `0xC` from `+0x20`, `0xFFFF` = free) plus an
indirect `jalr` via `[s0+0x80][a0]`.  Authority: retail Disc1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Prior state: `docs/evidence/agent-d401c/REPORT.md` (inverted-`for` loop lever,
63 -> 43 differing words; best candidate at `nonmatch/func_800D401C.c`).

## Result

**No match.** Best remains **43/72 differing words**, all
register-allocation/operand-order only.

## Variants tried this pass (baseline 43 = `nonmatch/func_800D401C.c`)

The disputed block is the table access.  Retail:
```
0x006C: lw   v1, 0x80(s0)     ; table base -> v1
0x0070: sll  v0, a0, 1        ; index*2    -> v0
0x0074: addu v0, v0, v1
0x0078: lhu  s2, 0x20(v0)
```
Signed-off block in the baseline candidate:
```c
v1 = (unsigned int)(a0 << 1);
v0 = *(unsigned int *)(s0 + 0x80);
v0 += v1;
s2 = *(unsigned short *)(v0 + 0x20);
```

| variant | source shape | differing words |
|---|---|---|
| baseline | index->`v1`, base->`v0`, `v0 += v1` | 43 |
| vA | index first then base: `v0=(a0<<1); v1=base; v0+=v1` | 44 |
| vB | local `unsigned short *p = base+0x20; s2 = p[a0];` | 45 |
| vC | single expression `*(u16*)(base + 0x20 + (a0<<1))` | 49 |
| vD | base first then index (baseline roles) | 43 |
| vE | base -> `v1`, index -> `v0`, `v0 += v1` | 44 |
| vF | base -> `v1`, index -> `v0`, folded address expr | 43 |
| vG | struct-typed `s0` view (`struct S0 { u8 pad[0x80]; u32 tbl; }`) | 44 |

Also tried earlier (recorded in the prior report): `-O1`, `-fno-strength-reduce`,
and every expression/declaration order; none changed the permutation.

## Conclusion / next lever

cc1 insists on reusing the (dead) incoming `$a0` for the table base, so the
index lands in `v1`; retail keeps the index in `v0` and the base in `v1`.
The `beq a1,v0` tail displacement also stays 2 words short (retail `+0x2D`,
candidate `+0x2B`).  No pure C source shape tried so far moves either.  A
different class of lever is required, e.g. a per-leaf scheduling/reorder gate,
or a matching rewrite that keeps `$a0` live past the table load (so cc1 cannot
reuse it) — but that must not change semantics, and no such shape was found in
this pass.

Best candidate preserved: `docs/evidence/agent-d401c2/cand_best_43.c`
(identical to `nonmatch/func_800D401C.c`).

## Commands

```
distrobox enter pe-mipsel -- bash -lc 'cd /home/blizz/dev/Parasite-Eve-Decompilation && \
  python3 tools/analysis/try_leaf.py /tmp/candD401C_v?.c 0xC481C 0x120'
```

No `src/`/`configs` change; matching build untouched (709 leaves, EXACT SHA-1).
No matching claim.
