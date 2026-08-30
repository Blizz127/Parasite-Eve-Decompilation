# PE-B54K-F — `func_80030894` fixed sprite and L9

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the coherent group after L8: one fixed wrapped sprite
and the complete four-sprite L9 loop. The cut is at the next fixed-group
materialization, leaving no partially initialized record.

## Retail identity and cut

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x80031110,0x800311EC)
file range       [0x00021910,0x000219EC)
size             0xDC bytes / 55 words
window SHA-256   7d64f3e10e16895bafd1b2fb71ed2757a5513072a35bd9772e04bd84ce56a739
prior word       0x8003110C  sb s7,0x06(s0) (L8 delay slot)
first word       0x80031110  lui s0,%hi(D_8009E768)
last word        0x800311E8  sb s7,0x06(s0) (L9 delay slot)
next word        0x800311EC  lui s0,%hi(D_8009E730)
```

The native prefix now covers:

```text
[0x80030894,0x800311EC) = 0x958 bytes / 598 words
```

The named strict boundary moves from `func_80030894_L8_cut` to
`func_80030894_L9_cut`. The following `D_8009E730` group does not execute.

## Calls and address geometry

The exact retail window contains two static calls to `func_800370DC`:

| PC | Runtime count | Role |
| --- | ---: | --- |
| `0x8003112C` | 1 | fixed sprite at `D_8009E768 + bank*28` |
| `0x800311A4` | 4 | L9 sprites at `D_8009E7A0 + bank*112 + slot*28` |

The machine arithmetic closes as follows:

```text
fixed bank stride = (bank*8 - bank) * 4  = bank*28
L9 bank stride    = (bank*8 - bank) * 16 = bank*112
L9 slot stride    = (slot*8 - slot) * 4   = slot*28
L9 bound          = (slot & 0xFF) < 4
L9 packet span    = 4 * 28 = 0x70 bytes
```

For bank zero, the admitted packet ranges remain separate across the retail
gap:

| Object | Range |
| --- | --- |
| fixed sprite | `[0x8009E768,0x8009E784)` |
| L9 sprites | `[0x8009E7A0,0x8009E810)` |

The fixed sprite uses RGB `0x80`, U/V `0x58/0xEF`, CLUT `0x7E13`, and
dimensions `36 x 5`. Each L9 sprite uses RGB `0x80`, the same CLUT, and
dimensions `6 x 6`. All packets retain the wrapped-sprite header
`length=6`, draw-mode word `0xE1000234`, zero appended tag, and sprite code
`0x64`. Retail leaves each packet's XY word at `+0x10` untouched.

## Independent oracle

`pc_port/tools/b54kf_30894_l9_oracle.py` imports no production code. It
requires the exact executable and window hashes, compares all 55 words,
decodes both calls, proves the L9 edge/bound/strides/extent, and verifies
both cut-side words.

```text
  OK retail window: 55 words, SHA-256 exact
  OK call census: two jal func_800370DC sites
  OK complete word comparison: 55/55
  OK L9: head 0x8003118C, bound 4, strides 112/28, extent 0x70
  OK cut: prior L8 delay slot and first excluded group word exact

B54K-F oracle: 6 check groups passed.
```

## Focused tests and gates

1. `B54KF_30894_fixed_sprite_and_l9` checks all fixed/L9 fields, both exact
   ranges, the intervening gap, the next group base, the named cut, and stop
   reason.
2. `B54KF_30894_l9_dirty_repeat_deterministic` poisons and snapshots the
   complete `0x8C`-byte union, compares repeated output, and proves the
   retail-untouched XY word remains poisoned in all five packets.

Normal suite:

```text
TEST B54KF_30894_fixed_sprite_and_l9... PASS
TEST B54KF_30894_l9_dirty_repeat_deterministic... PASS
Results: 940 run, 940 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 940 run, 940 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L9
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L9_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
POST_L9_FIXED_GROUP=absent
```
