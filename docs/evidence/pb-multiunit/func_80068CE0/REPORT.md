# func_80068CE0 — landed (779 → 780)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x80068CE0 · **file:** 0x594E0 · **size:** 0x48 (18 words)
**Unit:** 5747C.s (mid-carve), prefix 0x5747C..0x594E0, resume 0x59528..0x5AA5C
**Profile:** `era_o2_g0` (default; `-O1 -G0` is byte-identical)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_80068CE0.c 0x594E0 0x48 --flags "-O2 -G0"` | `WORDS MATCH (+8 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (780 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

Six ordered calls — `func_80066CE8()`, `func_80065674()`,
`func_80067E1C()`, `func_80067A78()`, `func_80067B74()`, `func_80067D18()`
— then `return 0`. Frame -0x18 with `$ra` at 0x10; the frame restore is
in the `jr` delay slot.

## Levers

* Straight call sequence then `return 0`; no typing tricks needed.
* Matched first try from the m2c draft (`--try` shape already exact).

## Divergences / negatives

* The +8 pad bytes are trailing alignment nops, trimmed by the build.
