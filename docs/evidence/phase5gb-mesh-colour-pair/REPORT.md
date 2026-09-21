# Phase 5GB — the mesh colour transfer pair, and the port checked again

Date: 2026-09-21 (GOAL 4H #3, `docs/ai_context/GOAL_4H_QUEUE_ROUND2.md`).

```text
IMPLEMENTED=func_800C6EF8 0x800C6EF8..0x800C6F4C  (0x54, 21 words)  mesh -> buffer
            func_800C6F4C 0x800C6F4C..0x800C6FA0  (0x54, 21 words)  buffer -> mesh
SOURCES=src/func_800C6EF8.c, src/func_800C6F4C.c
PROFILE=default era_o2_g0 (-O2 -G0) for both
YAML=[0xB76F8 c func_800C6EF8][0xB774C c func_800C6F4C][0xB77A0 asm]
LINK_CHECK=LINK_EXACT for both (word mismatches=0, pad 0)
PLANTED_STATE=NO
```

## What the pair is

Two mirror-image word-array transfers between a mesh record and the fixed buffer
`D_800E2370`. The record carries a byte offset in its `+8` halfword and a word count
in its `+0xA` halfword:

- **`func_800C6EF8`** — copy `count` words *from* `record + *(u16 *)(record + 8)`
  *to* `D_800E2370`;
- **`func_800C6F4C`** — copy them back.

Both re-read `*(u16 *)(record + 0xA)` on **every** iteration, which is why retail
carries `lhu 0xA(a0)` inside the loop instead of hoisting it: the stores could alias
the record. The transfer is 32-bit word granular, and the guard is a `blez` on the
count followed by a signed `slt` on a zero-extended 16-bit load — so both behave as
unsigned for a 16-bit count, with no signed/unsigned trap at `count >= 0x8000`.

## The one spelling detail

The counter increment has to be written **inside the loop body, before the copy**:

```c
    int i = 0;
    ...
    while (i < *(unsigned short *)(record + 0xA)) {
        i++;                 /* in-body: GCC schedules this first, as retail does */
        *dst++ = *src++;
    }
```

As a `for (i = 0; i < count; i++)` header the increment lands *after* the load and
exactly three words differ. That is the sixth source-spelling fix of the session —
the same class as `volatile int *p`, `int one = 1;`, the post-increment scan, the
non-small declaration and locals-before-stores.

## Port verification

The port implements both in `pc_port/game/boot/func_800C71E4_port.c`:

```c
void func_800C6EF8(pe_addr_t mesh)
{
    pe_addr_t colors=mesh+PE_LoadU16(mesh+8u);unsigned i;
    for (i=0;i<PE_LoadU16(mesh+10u);i++) PE_StoreU32(0x800E2370u+i*4u,PE_LoadU32(colors+i*4u));
}
```

Verdict: **agrees exactly** — same source derivation (`mesh + *(u16 *)(mesh+8)`), same
count field re-read every iteration (`PE_LoadU16(mesh+10u)` is a call in the loop
condition, so a mid-loop change is observed, as retail's reload does), same word
granularity, and the mirror leaf has the operands swapped the same way. The only
cosmetic difference is the port's `unsigned i` versus the decompiled `int i`, which
the `lhu`/`blez`/`slt` sequence makes equivalent for a 16-bit count.

`pc_port/tests/test_port_verify_decomp.h` now pins the pair: both directions
round-trip three words through `D_800E2370`, the transfer stops at the count, and a
zero count is a no-op in both directions.

## Gate

```text
EXACT_REBUILD_GATE=PASS  spans=[810 c, 347 asm, 2 rodata]
VERIFY_SWEEP=PASS leaves=810   plan=184dab537bab…
sha1_orig == sha1_cand == 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

As with the 42E34 cluster and the arena pair, `funcs` stays 377/980 and `c_words`
6834 while the plan goes 808 → **810** c spans: these two are reached through the
effect/draw path, not the direct-call closure, so the union metric cannot see them.
