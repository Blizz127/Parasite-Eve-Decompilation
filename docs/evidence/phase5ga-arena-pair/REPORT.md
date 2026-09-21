# Phase 5GA — the arena push/pop pair, and the port verified against it

Date: 2026-09-19 (GOAL 4H, `docs/ai_context/GOAL_4H_QUEUE_CONTINUE.md`).

```text
IMPLEMENTED=func_8005E8C4 0x8005E8C4..0x8005E914  (0x50, 20 words)  push
            func_8005E914 0x8005E914..0x8005E968  (0x54, 21 words)  pop
SOURCES=src/func_8005E8C4.c, src/func_8005E914.c
PROFILE=era_o2_g8 (-O2 -G8) for both
YAML=[0x4F0C4 c func_8005E8C4][0x4F114 c func_8005E914]
LINK_CHECK=LINK_EXACT for both (word mismatches=0, pad 0)
PLANTED_STATE=NO
```

## What the pair is

Both functions move the arena write cursor `D_8009D12C` (gp+0x3BC) by eight bytes
and exchange two words with `D_8009D124` (gp+0x3B4) / `D_8009D128` (gp+0x3B8):

- **`func_8005E8C4` (push)** — while `cursor < D_800A22B0` (the arena end), the
  cursor stores the two words and advances by 8; otherwise it calls
  `func_800527C0(2)`.
- **`func_8005E914` (pop)** — while `D_800A2270` (the arena start) `< cursor`, it
  steps back 8 and publishes the two words it passed; otherwise it calls
  `func_800527C0(3)`.

So the arena window is `0x800A2270..0x800A22B0`, and the two report codes are
**2 = overflow, 3 = underflow** — values the decomp now establishes and which were
not previously recorded anywhere.

## Two spelling details, both load-bearing

1. **Both source words are read into locals before the cursor moves.** Written with
   the loads inline in the stores, GCC sinks the second load below the first store
   and the function is 7 words off; with the locals the two `lw`s stay hoisted above
   the stores exactly as retail has them.
2. **The arena bound is declared as a 3-word object** (`extern int D_800A22B0[3];`).
   Only its address is taken, but the declaration puts it above the `-G8`
   small-data threshold, so cc1 emits the absolute `lui`+`addiu` bound reference
   retail uses instead of a gp-relative one. Same device as Phase 5FY, and again it
   removes the need for a `MASPSX_FORCE_ABSOLUTE_SYMBOLS` entry.

## Port verification

Both functions have hand-written port counterparts in
`pc_port/game/boot/func_8005EED4_port.c`:

| decompiled | port | verdict |
| --- | --- | --- |
| `func_8005E8C4` push | `func_8005EED4_port.c:29` | **agrees** on cursor arithmetic, bounds and word order (port stores through `menu_ram()`, the documented KUSEG mirror) |
| `func_8005E914` pop | `func_8005EED4_port.c:39` | **agrees** likewise |

One structural difference, and it is behaviourally nil: the port **elides the
`func_800527C0(2)` / `(3)` call** on the out-of-bounds path. The retail callee at
`0x800527C0` is an empty `jr $ra; nop` stub (`src/func_800527C0.c`, a Phase 5AG
matching leaf), and the port already elides calls to it elsewhere with that reason
spelled out (`func_8005ED18_port.c`: "pool exhausted (an empty stub)"). So the
port is faithful here; what the decomp adds is the *argument values*, which are now
recorded. If `func_800527C0` ever gains a body, this is the place that would need
the calls added.

`pc_port/tests/test_port_verify_decomp.h` now pins the pair: the cursor advances
and retreats by 8, the two words round-trip through the arena in order, and a push
at the arena end / pop at the arena start are no-ops.

The test suite also caught a mistake while writing this — the first draft seeded a
cursor *past* the arena end, and the assertion failed until the window was fixed.
That is the point of writing the boundary cases down.

## Gate

```text
EXACT_REBUILD_GATE=PASS  spans=[808 c, 347 asm, 2 rodata]
VERIFY_SWEEP=PASS leaves=808   plan=49257b756e66…
sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

`funcs` stayed 377/980 and `c_words` 6834 even though the plan went 806 → **808** c
spans: like the 42E34 cluster these two are reached through the menu draw path
rather than the direct-call closure, so the union metric cannot see them. The plan
count and the gate are the evidence that counts here.
