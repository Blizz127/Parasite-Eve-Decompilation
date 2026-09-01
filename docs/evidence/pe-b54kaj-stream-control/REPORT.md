# PE-B54K-AJ — movie stream-control initializer

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the complete state initializer called after the movie
record pool. The state shape is proven, but no public SDK name is assigned
without string or symbol evidence.

## Retail identity and function-hood

```text
initializer         [0x8007C304,0x8007C388) / 33 words
initializer offset  0x6CB04
initializer SHA-256 b36aa720216a101d55e1f1fc01fdb4de5d469616649d35efb6e3637cc71992f2
setter              [0x8007C544,0x8007C560) / 7 words
setter offset       0x6CD44
setter SHA-256      0b85dee972b3ff9e4c01682f71ae2e5c63ed9c55d0934f20911104b6fb66f3db
production call     0x80192768 jal 0x8007C304
caller prefix       [0x801924F8,0x80192770) / 158 words
caller SHA-256      fbbb2ffe13e98fa176ce686b491f28fc0e44db98c594365a802f97ebbefd5196
```

Function-hood is proven by the exact production call, conventional prologue,
exact call to the setter, and canonical return. The setter has that exact
caller and returns with its third store in the `jr ra` delay slot. Both
neighboring boundaries contain real instructions.

## Production arguments

The eight-word production block is exact:

```text
80192750 24040001  li    a0,1
80192754 3C02801D  lui   v0,0x801D
80192758 8C4211AC  lw    v0,0x11AC(v0)
8019275C 2406FFFF  li    a2,-1
80192760 84450006  lh    a1,6(v0)
80192764 00003821  move  a3,zero
80192768 0C01F0C1  jal   0x8007C304
8019276C AFA00010  sw    zero,0x10(sp)
```

Thus the five arguments are `(1, signed record[+6], -1, 0, 0)`. The first
following word at `0x80192770` is a real `jal 0x8007F72C`, starting the CD
ready/busy loop.

## Proven state contract

The setter always stores mode `1`, then stores its signed `start` and `end`
arguments as full words. The initializer then stores:

```text
D_800C0DB8 = 0
D_800B0CC8 = callback
D_800A801C = caller_mode & 1
D_800B8620 = 0
D_800B6914 = 0
*(uint16_t *)D_800A8018 = 0
D_800A5D54 = 0
D_800B0CCC = fifth_argument
```

The third register argument is otherwise unused by the initializer after it
is forwarded to the setter. No callback is invoked and no stream work is
performed here.

## Controls

- Negative start/end values prove full-width signed forwarding.
- Caller modes 2 and 3 prove only bit zero is retained in the option word.
- Distinct callback and fifth-argument sentinels prove their separate homes.
- A poisoned upper halfword adjacent to `D_800A8018` proves the retail
  halfword store width.
- The real Disc 1 path proves the record-derived signed start and all four
  constant arguments before the strict cut.

## Verification

```text
B54K-AJ independent oracle: PASS
B54K-AJ focused test:       1/1 PASS
B54K-Y real-disc tests:     2/2 PASS
native suite:               992/992
normal CTest:               2/2 PASS
fresh ASan/UBSan CTest:     2/2 PASS
strict real-disc exit:      1
strict frontier:            func_801924F8_80192770_cut
retail EXE SHA-1:           452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No CD read is issued, no frame is decoded, no callback is invoked, and no
scene, scheduler, story, persistence, or destination state is planted.

```text
FUNC_8007C304=COMPLETE_STREAM_CONTROL_INITIALIZER
FUNC_8007C544=COMPLETE_THREE_GLOBAL_SETTER
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192770_cut
NEXT_ARTIFACT_FREE_RUNG=cd_ready_search_loop_from_0x80192770
```
