# PE-B54K-B2 — `func_80030894` L5 continuation

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung advances only the next closed retail loop in `func_80030894`.
It adds no scheduler, destination, scene, or story-state behavior.

## Retail identity and boundary

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x80030C9C, 0x80030D20)
file range       [0x0002149C, 0x00021520)
size             0x84 bytes / 33 words
window SHA-256   9fbe234f952c643335daaff99026f070ed84133c5654f9ed673c4e14893cec10
prior word       0x80030C98  andi v0,s6,0x00FF (L4 delay slot)
first word       0x80030C9C  addu s6,zero,zero  (L5 counter init)
last word        0x80030D1C  andi v0,s6,0x00FF (L5 delay slot)
next word        0x80030D20  lbu s5,0x18(sp)    (post-L5 setup)
```

The implemented function prefix is now:

```text
[0x80030894,0x80030D20) = 0x48C bytes / 291 words
```

The explicit strict boundary moves from `func_80030894_L4_cut` to
`func_80030894_L5_cut`. This remains an internal continuation of the
B54J-proven real function, whose sole direct caller is the `jal` in
`func_8006AD40` at `0x8006B0AC`.

## Retail decode

The window has exactly one call:

```text
0x80030CDC  jal func_800370DC
```

The counter/address arithmetic is instruction-proven:

```text
bank stride = (((bank*8 + bank)*4 - bank)*4) = bank*140
slot stride = ((slot*8 - slot)*4)             = slot*28
packet      = 0x8009E1D0 + bank*140 + slot*28
bound       = (slot & 0xFF) < 5
extent      = 5 * 28 = 140 = 0x8C bytes
```

For each packet, retail calls the already-native sprite wrapper, stores the
shared CLUT at `+0x16`, and stores dimensions `6 x 10` at `+0x18/+0x1A`.
For bank zero the only new guest-memory extent is therefore exactly:

```text
[0x8009E1D0,0x8009E25C)
```

The byte at `0x8009E25C` and the first following fixed record at
`0x800B0130` are negative-control sentinels and remain untouched.

## Independent oracle

`pc_port/tools/b54kb2_30894_l5_oracle.py` imports no production code. It
requires the exact executable SHA-1 and window SHA-256, checks a complete
33-word table plus the first excluded word, independently decodes the sole
call, and proves the branch target, five-iteration bound, and 0x8C extent.

```text
  OK retail window: 33 words, SHA-256 exact
  OK complete word table: 33 words plus first excluded word
  OK call census: func_800370DC at 0x80030CDC
  OK L5 loop: head 0x80030CC8, bound 5, extent 0x8C
  OK cut: first excluded word is lbu s5,0x18(sp)

B54K-B2 oracle: 5 check groups passed.
```

## Native contract and gates

Two focused tests were added:

1. `B54KB2_30894_l5_five_packets` verifies all five packet headers, draw-mode
   words, CLUTs, dimensions, exact end sentinels, named boundary, and stop
   reason.
2. `B54KB2_30894_l5_dirty_repeat_deterministic` poisons all 0x8C bytes,
   executes twice, compares the whole result, and proves the first byte after
   the extent is preserved.

The existing production write-footprint guard admits only the new proven
extent. All B54K-A/B1 tests remain green.

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_fixed_setup_and_l4... PASS
TEST B54KB1_30894_dirty_repeat_deterministic... PASS
TEST B54KB2_30894_l5_five_packets... PASS
TEST B54KB2_30894_l5_dirty_repeat_deterministic... PASS
Results: 932 run, 932 passed, 0 failed, 0 skipped
```

Freshly rebuilt ASan/UBSan suite:

```text
Results: 932 run, 932 passed, 0 failed, 0 skipped
sanitizer_diagnostics=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L5
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L5_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
POST_L5_EXECUTION=absent
```
