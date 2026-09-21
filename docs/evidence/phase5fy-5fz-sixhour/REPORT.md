# Phase 5FY/5FZ — three more matching leaves, and port verification against the decomp

Date: 2026-09-18/19 (GOAL 6H, `docs/ai_context/GOAL_6H_DECOMP_PORT.md`).

```text
IMPLEMENTED=func_8005270C 0x8005270C..0x80052764  (0x58, 22 words)  Phase 5FY
            func_80052C08 0x80052C08..0x4346C     (0x64, 25 words)  Phase 5FZ
            func_800773D0 0x800773D0..0x80077C04  (0x34, 13 words)  Phase 5FZ
SOURCES=src/func_8005270C.c, src/func_80052C08.c, src/func_800773D0.c
PROFILES=era_o2_g8 for func_8005270C; default era_o2_g0 for the other two
LINK_CHECK=LINK_EXACT for all three, first correct spelling each
PLANTED_STATE=NO
```

Gate evidence is filled in at the bottom once `scripts/exact_rebuild.sh` reports.

## Phase 5FY — `func_8005270C` (the parked 0x58 leaf, now closed)

This was left as `asm` in Phase 5FW after a bounded codegen hunt (best 11/22
words). The blocker was **addressing**, not structure:

- retail needs `&D_800B0E08` computed once into `$a0` (`lui` + `addiu`) and then
  loaded through twice — the test read and the call argument;
- at `-G8` a 4-byte declaration makes cc1 address it **gp-relative**, and the
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS` knob only rewrites that into a *fused*
  `lui`/`lw` — a shape cc1 never emits for a live address register;
- declaring the pointer as a **3-word object** (above the `-G8` threshold) makes
  cc1 emit the genuine absolute address, and the pair matches with no knob.

The array declaration is a compile-time device only: element 0 is the same 4-byte
package pointer, so every emitted load is unchanged and no maspsx environment
variable is needed. That difference — "forced absolute" vs "naturally absolute" —
is the reusable part for the next gp-relative leaf.

## Phase 5FZ — two leaves first try, one after a spelling change

`func_800773D0` (13 words) matched immediately at the default `-O2 -G0`: arm the
GPU timeout deadline from the VSync clock plus `0xF0`, clear the poll word.

`func_80052C08` (25 words) needed one spelling change. It appends a
0xFF-terminated byte string, and retail's terminator scan carries the
`addiu dst,dst,1` in the **branch delay slot of the exit test**, leaving the
pointer one past the terminator before a step back. The pre-increment spelling
(`while (*dst != 0xFF) dst++;`) reads identically but rotates both loops
differently — 13 word mismatches. The post-increment form with an explicit step
back, `while (*dst++ != 0xFF) { } dst--;`, is word-exact.

So the session's tally of "the lever was the source, not a flag" now covers three
leaves: `volatile int *p` (address), `int one = 1;` (loop constant), and the
post-increment test with `dst--` (loop rotation).

## Port verification against the new decomp

Every leaf landed in this session has a hand-written port counterpart. The
now-authoritative C turns these from "looks plausible" into checked statements,
and `pc_port/tests/test_port_verify_decomp.h` (`PORTVERIFY_matched_leaves`) locks
them in the native suite.

| decompiled leaf | port implementation | verdict |
| --- | --- | --- |
| `func_80062F3C` (5FX) | `func_80062D2C_port.c` — `func_80062A34(1u,id)` + `func_8006269C` | **agrees**: helper walks head `0x8009D154`, matches `+0x20 == kind`, `+0x24 == id`, follows `+0x0`, returns the node or NULL — exactly the decompiled walk |
| `func_800773D0` (5FZ) | `func_80076C34_port.c` | **agrees**: `GA_GPU_TIMEOUT_DEADLINE` (`0x80095888`) = clock+0xF0, `GA_GPU_TIMEOUT_POLLS` (`0x8009588C`) = 0, deadline returned (retail leaves it in `$v0`) |
| `func_8005270C` (5FY) | `field_message_port.c` | **agrees**: package from `0x800B0E08`, sound `0x450`, args `0x100/0x80/0x7F`, result stored at `0x8009D01C` and returned |
| `func_80052764` (5FW) | `battle_reward_port.c` | **agrees**: `if (target) { func_800866A4(target,0); target = 0; }` against the same `0x8009D01C` |
| `func_80052C08` (5FZ) | `func_8004F910_port.c` (copy factored into `func_80052BCC`) | **agrees**: same observable buffer. Note the factoring differs — retail inlines the copy loop inside `func_80052C08` while the port calls `func_80052BCC` (itself a matched leaf at `0x433CC`), so this is a legitimate adaptation, not drift |

The only divergences found are the documented host substitutions (VSync query,
sound-package stub) and that one factoring difference; no semantic drift.

```
TEST PORTVERIFY_matched_leaves... PASS
Results: 1405 run, 1 passed, 0 failed, 1404 skipped   (filtered; full suite 1405)
```

## Gate

```text
5FY (func_8005270C):  EXACT_REBUILD_GATE=PASS  spans=[804 c, 347 asm, 2 rodata]
                      VERIFY_SWEEP=PASS leaves=804  plan=1e487548c844…
                      sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
                      funcs 374 -> 376/980, c_words 6776 -> 6809
5FZ (both leaves):    EXACT_REBUILD_GATE=PASS  spans=[806 c, 348 asm, 2 rodata]
                      VERIFY_SWEEP=PASS leaves=806  plan=87f1d99d909d…
                      sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
                      funcs 376 -> 377/980, c_words 6809 -> 6834, asm_funcs 528 -> 527
```

Note the union grew from 979 to **980** functions during 5FY (`tierA` 734 → 735):
these leaves are inside the direct-call closure, so unlike the 42E34 cluster they
do move the coverage metric.

Full suite: **1405 run, 1405 passed**.

