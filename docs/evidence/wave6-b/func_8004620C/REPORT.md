# func_8004620C

- **VRAM**: 0x8004620C
- **File offset**: 0x36A0C (size 0x128)
- **Build profile**: era_o2_g0 (default `-O2 -G0`)
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
Battle state-machine step driven by two event bits. `list = func_80062A20(page,0)`,
`s1 = func_80062A34(2,0xB)`; when `s1` is live it forwards to
`func_80047FE0(list,s1,event)`. If the forward failed and `event & 0x40` is set
it runs the 7/5/`func_800439D8`/`func_80052634` teardown (only when `s1 == 0`).
Otherwise, when `event & 0x1000` is set it marks `list+0x44 = -1`, reacquires
`func_80062A34(2,5 | 0x1B)`, clears +0x44, sets `+0x48 = *(int*)(+0x58)-1` and
calls `func_80062CB8` + `func_8005267C`. Always returns 1.

## Method
Three source-shape levers:
- a single `v0` temporary used for **both** `event & 0x1000` and `event & 0x40`
  (retail emits `andi $v0,…` twice) — separate locals get CSE'd into one;
- explicit `goto check` so the `s3 != 0` early path and the fall-through share
  the final test;
- the `s1` selection is written as **two calls**
  (`if (s1 == 0) func_80062A34(2,5); else func_80062A34(2,0x1B);`) so gcc's
  cross-jump merges them into retail's single `jal` with a0=2 set on both arms
  and the `bnez $s1` polarity retail uses.

## Evidence
try_leaf `WORDS MATCH (+8 pad bytes)`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (891
registered C leaves)`, `VERIFY_US=PASS`.
