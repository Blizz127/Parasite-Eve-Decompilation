# Phase 5FX — matching C leaf `func_80062F3C` (node-list scan)

Date: 2026-09-18

```text
IMPLEMENTED=0x80062F3C..0x80062F9C exclusive
WORDS=24
BYTES=0x60
SOURCE=src/func_80062F3C.c
PROFILE=era_o2_g8 (-O2 -G8)
YAML=[0x5373C, c, func_80062F3C]   (was the whole [0x5373C, asm] span)
LINK_CHECK=LINK_EXACT (tools/analysis/era_link_check.py, word mismatches=0, pad 0)
PREFLIGHT=disc1_preflight: PASS (deep, 803 c / 348 asm / 2 rodata)
GATE=EXACT_REBUILD_GATE=PASS
GATE_SHA1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b (orig == cand)
GATE_PLAN=2b52a0ac44eacc95089fdb328b7f02a6e188d32b0460c1f7aba098cd18d586e7
GATE_SWEEP=VERIFY_SWEEP=PASS leaves=803
METRIC=funcs 373 -> 374/979, c_words 6752 -> 6776 (+24 = 0x60), asm_funcs 530 -> 529
       — unlike the 42E34 cluster, this leaf IS in the 979-function union.
PLANTED_STATE=NO
```

## What the function is

Walks the node list rooted at `D_8009D154` (`gp+0x3E4` = `0x8009D154`) looking for
the entry with `+0x20 == 1` and `+0x24 == the argument`, then calls
`func_8006269C` with that node. The cursor stays in `$a0` across the loop, so a
miss — or an empty list — passes NULL. The port already spells the same thing as
`func_8006269C(func_80062A34(1u, id))` in
`pc_port/game/boot/func_80062D2C_port.c`.

```c
extern int D_8009D154;
void func_8006269C(int node);
void func_80062F3C(int id) {
    int node = D_8009D154;
    int one = 1;

    while (node != 0) {
        if (*(int *)(node + 0x20) == one && *(int *)(node + 0x24) == id) break;
        node = *(int *)node;
    }
    func_8006269C(node);
}
```

`D_8009D154` is in the small-data window, so the head load is gp-relative
(`0x3E4($gp)`) and the leaf uses the existing `era_o2_g8` profile.

## The interesting part: `int one = 1;` is load-bearing

Every structural spelling of the loop produced the same **6-word** difference,
entirely in the prologue. Retail hoists the loop constant into the entry block;
the naive spelling materialises it inside the loop body:

```text
retail                                naive spelling
addiu sp,sp,-0x18                     addu  v1,a0,zero
addu  v1,a0,zero                      lw    a0,0x3E4(gp)
lw    a0,0x3E4(gp)                    addiu sp,sp,-0x18     <- frame adjust sank
addiu a1,zero,1                       beqz  a0,.L
beqz  a0,.L                             sw  ra,0x10(sp)
  sw  ra,0x10(sp)                     addiu a1,zero,1       <- constant landed here
```

Holding the constant in a local (`int one = 1;`) makes GCC treat it as a
loop-invariant value with a register home, so it is hoisted before the guard —
and the frame adjust stays first. With that one line the whole function matches.

| source form (`-O2 -G8` unless noted) | word mismatches |
| --- | --- |
| pointer casts, inline `== 1` | 6 (prologue only) |
| `struct Node` fields, inline `== 1` | 6 |
| `for` loop, inline `== 1` | 6 |
| nested `if`s, inline `== 1` | 6 |
| `extern volatile int D_8009D154;` | 6 |
| explicit early-return guard | 18 |
| **`int one = 1;` local** | **0 — LINK_EXACT** |
| `one == *(int *)(node + 0x20)` (swapped operands) | 2 (`bne` operand order flips) |
| `-O1 -G8` inline | 23 |
| `-O1 -G8 -fschedule-insns2` inline | 6 |
| `-O2 -G8 -fno-schedule-insns2` | 23 |
| `-O2 -G8 -fno-delayed-branch` | 24 |

So the lever here was the **source**, not a compiler flag or a maspsx knob:
expressing the constant as a value gives the register allocator the freedom
retail's build exercised. Worth trying first on the next `<8`-mismatch leaf
(`func_8005270C`, where the address rather than a constant is the value that must
be hoisted into the entry block).
