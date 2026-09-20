# func_80046ABC — landed (776 → 777)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x80046ABC · **file:** 0x372BC · **size:** 0x9C (39 words)
**Unit:** 35698.s (mid-carve), prefix 0x35698..0x372BC, resume 0x37358..0x379BC
**Profile:** `era_o2_g0` (default; `era_o2_g8` is byte-identical)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_80046ABC.c 0x372BC 0x9C --flags "-O2 -G0"` | `WORDS MATCH (+4 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (777 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

```
a = func_80062D2C(8, arg0, 0, 0);
b = func_8006322C(8, a, a);
*(void **)(a + 0x2C) = func_80046B58;
*(void **)(b + 0x30) = func_8004FC80;
*(void **)(b + 0x8C) = func_8004FC3C;
func_80062CB8((int)b);
func_80055610();
return func_800647D0(b, func_80054288());
```

## Levers

1. **`char *` locals for the two objects.** They land in `$s0`/`$s1` as
   retail (`addu $s0,$v0,zero`, `addu $s1,$v0,zero`) and every field store
   goes through those base registers.
2. **Materialize each callback address in the following call's delay
   slot** — the natural ordering of the stores relative to the
   `jal func_80062CB8` / `jal func_80055610` pairs does this.
3. Matched on the first try from the m2c draft: only pointer typing and
   the `func_80062CB8((int)b)` argument needed care.

## Divergences / negatives

* `-O1 -G0` = 13 diffs (different inlining/scheduling of the allocation
  calls).
* The +4 pad bytes are a compiler alignment nop at the tail, trimmed by
  the build.
