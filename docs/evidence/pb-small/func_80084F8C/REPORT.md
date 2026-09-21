# func_80084F8C — pb-small (slice C)

Landed as a matching retail C leaf. Leaf count 768 -> 774 (six-leaf batch).

## Span math

- VRAM `0x80084F8C`; file offset `0x7578C`; size `0x2C`; unit `75378`.
- Carve inside the `[0x75378, asm]` run:
  `[0x75378, asm]` / `[0x7578C, c, func_80084F8C]` / `[0x757B8, asm]`.
- `0x7578C + 0x2C = 0x757B8`; resume runs to the next span `0x757C4`.

## Semantics

Controller-record readiness: `1` when the halfword at `+0xE6` is zero, else `1`
when the byte at `+0x46` is not `0xFF`, else `0`. Matches
`pe_controller_sdk_port.c:10`.

## Build profile

Default `era_o2_g0` (no profile entry needed).

## Matching lever

This leaf is the hard one. Retail branches twice:

```
lhu  $v0,0xE6($a0) ; nop
beqz $v0,L1
addiu $v0,$zero,0xFF
lbu  $v1,0x46($a0) ; nop
beq  $v1,$v0,L2
addu $v0,$zero,$zero
L1: addiu $v0,$zero,1
L2: jr $ra ; nop
```

Any plain `return`/`if-else-return` spelling lets era cc1 fold the tail into a
branchless `xori $v0,$v0,0xFF / sltu $v0,$zero,$v0` (6-word difference), and
`volatile`, `-O1`, `-fno-thread-jumps`, and register pins do not stop it. The
m2c-recovered **short-circuit comma form** keeps the branch:

```c
if (*(unsigned short *)(record + 0xE6) == 0 || (result = 0, record[0x46] != 0xFF)) {
    return 1;
}
return result;
```

The `result = 0` side effect is exactly retail's `addu $v0,$zero,$zero` in the
`beq` delay slot. Earlier comma placements where `result` is live across the
compare (e.g. `(result = 0, ...)` inside the condition plus a later
`result = 1`) force the `0xFF` constant out of `$v0` into `$v1` (3-word
difference). The exact form above was the only one of ~15 tried that matched.

## Evidence

- Triage: `try_leaf.py src/func_80084F8C.c 0x7578C 0x2C --flags "-O2 -G0"` -> `WORDS MATCH`.
- Authority: see batch report — `EXACT SHA-1 452fb033...`, 774 leaves, `VERIFY_US=PASS`.
