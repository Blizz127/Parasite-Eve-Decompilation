# PE-B54K-I — complete `func_80030894`

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the function's final 43 words: the per-bank final
sprite, outer-bank increment/back edge, and normal return. `func_80030894`
is now complete; the strict frontier moves into its caller at the existing
`func_8006AD40_post30894_cut` boundary.

## Retail identity and function boundary

```text
executable SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
full VA range      [0x80030894,0x800314E4)
full file range    [0x00021094,0x00021CE4)
full size          0xC50 bytes / 788 words
full SHA-256       a4dbd2cf130979a0f5db8ed532d0c559c5b3b90b2c5786e10fe91125311ed6e2
new VA range       [0x80031438,0x800314E4)
new file range     [0x00021C38,0x00021CE4)
new size           0xAC bytes / 43 words
new SHA-256        69704305125d3ecde96259e29d5a100c3ca327f8e130e04541e4e62e13c68bc0
prior word         0x80031434  andi s1,s6,0x00FF (L11 delay slot)
first word         0x80031438  move a0,zero
return             0x800314DC  jr ra
return delay       0x800314E0  nop
next function      0x800314E4  addiu sp,sp,-0x20
```

The sole direct caller remains `jal func_80030894` at `0x8006B0AC`, with
`nop` at `0x8006B0B0`. The caller's first untranslated instruction is now
the real branch at `0x8006B0B4`, so the named production frontier is:

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_post30894_cut
```

## Final packet and outer-loop contract

The exact final window has two calls:

| PC | Callee | Role |
| --- | --- | --- |
| `0x80031444` | `func_80077A64` | `GetTPage(0,0,0x1C0,0) & 0xFFFF = 7` |
| `0x80031468` | `func_800370DC` | final wrapped sprite |

For each bank, the packet is:

```text
D_8009EC38 + bank*28
wrapper mode       7       (draw-mode word E1000207)
RGB                80/80/80
width/height       16/16
XY, UV, CLUT       untouched
```

The bank byte was initialized once at `0x8003090C`. Retail increments it at
`0x80031480/0x80031484`, compares the unsigned byte against 2, and branches
from `0x800314A8` back to `0x80030910`. Therefore the prologue executes once,
the complete bank-local body executes exactly for banks 0 and 1, and the two
final packets close at:

```text
[0x8009EC38,0x8009EC70) = 2 * 28 = 0x38 bytes
```

The two 40-byte FT4 bank records occupy `[0x800BE9F0,0x800BEA40)`. The two
L2/L3 arrays occupy `[0x800B01C0,0x800B0728)` and
`[0x800B0738,0x800B0CA0)`, preserving the retail `0x10`-byte inter-bank gap.
All remaining bank-strided groups are admitted as their exact individual
ranges by the production full-RAM write-set test; no broad envelope replaces
those checks.

## Independent oracle

`pc_port/tools/b54ki_30894_complete_oracle.py` imports no production code.
It verifies the exact executable, full-body and final-window hashes, compares
all 43 new words, decodes both calls, checks the final packet constants,
proves the back edge/bound/extent, and checks the canonical return plus the
next function boundary.

```text
  OK retail identity: full 788 words and final 43-word hash exact
  OK complete final-window comparison: 43/43
  OK final calls: TPage(0,0,0x1C0,0) then sprite wrapper
  OK final packet: D_8009EC38 + bank*28, RGB 0x80, dimensions 16x16
  OK outer loop: back edge 0x80030910, bound 2, final extent 0x38
  OK return/boundary: jr ra+nop; next function begins 0x800314E4

B54K-I oracle: 6 check groups passed.
```

## Focused tests and gates

1. `B54KI_30894_second_bank_and_normal_return` samples the first and last
   second-bank packet of every loop family, all bank-strided fixed wrappers,
   the second FT4 record, L2/L3 CLUT endpoints, L6 entry-color snapshots,
   direct Sprt/F3 headers, both L11 descriptor endpoints, both final sprites,
   the exact end sentinel, and normal boundary-free return.
2. `B54KI_30894_full_region_dirty_repeat_deterministic` poisons and snapshots
   the complete `0x8009E068..0x8009EC70` two-bank output region, compares a
   repeated run, and proves both final sprites leave XY/UV/CLUT untouched.
3. The retained production integration test compares all guest RAM and admits
   only the exact two-bank packet/state ranges. It now observes one boundary:
   `func_8006AD40_post30894_cut`.

Normal suite:

```text
TEST B54KI_30894_second_bank_and_normal_return... PASS
TEST B54KI_30894_full_region_dirty_repeat_deterministic... PASS
Results: 946 run, 946 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 946 run, 946 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_complete
FUNC_80030894_INTERNAL_BOUNDARY=absent
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_post30894_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
OUTER_BANK_COUNT=retail_two
```
