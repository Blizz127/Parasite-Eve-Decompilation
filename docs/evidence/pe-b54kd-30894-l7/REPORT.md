# PE-B54K-D — `func_80030894` fixed primitives and L7

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the next coherent retail group after L6: one wrapped
sprite, two direct SetSprt records, one PolyF3 header, and the complete
three-sprite L7 loop. The cut is immediately after L7; the next instruction
initializes the separate L8 loop.

## Retail identity and cut

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x80030F6C,0x800310A4)
file range       [0x0002176C,0x000218A4)
size             0x138 bytes / 78 words
window SHA-256   0b3149236b8f26a7cc5614dd4179039f1f33824a4c3618fef4cf64eca551659d
prior word       0x80030F68  andi s1,s6,0x00FF (L6 delay slot)
first word       0x80030F6C  lui s0,%hi(D_8009E460)
last word        0x800310A0  sb s7,0x06(s0) (L7 delay slot)
next word        0x800310A4  move s6,zero (L8 initialization)
```

The native prefix now covers:

```text
[0x80030894,0x800310A4) = 0x810 bytes / 516 words
```

The named strict boundary moves from `func_80030894_L6_cut` to
`func_80030894_L7_cut`. No L8 instruction executes in the translated prefix.

## Call census

The exact retail window contains five static call sites:

| PC | Callee | Role |
| --- | --- | --- |
| `0x80030F88` | `func_800370DC` | wrapped sprite before L7 |
| `0x80030FD4` | `func_80077C64` | direct SetSprt A |
| `0x80030FE4` | `func_80077C64` | direct SetSprt B |
| `0x80031020` | `func_80077B64` | direct PolyF3 header |
| `0x80031058` | `func_800370DC` | L7 sprite wrapper, three runtime calls |

All callees were already real native implementations. No new provider,
callback, SDK collapse, or host-only state was introduced.

## Address and state geometry

Retail's register arithmetic decodes to:

```text
wrapped sprite bank stride = bank * 28
direct SetSprt bank stride = bank * 32, records at +0 and +16
PolyF3 bank stride         = bank * 20
L7 bank stride             = bank * 84
L7 slot stride             = slot * 28
L7 bound                   = (slot & 0xFF) < 3
L7 extent                  = 3 * 28 = 0x54 bytes
```

For bank zero, the exact write set is admitted as five separate ranges:

| Object | Range |
| --- | --- |
| wrapped sprite | `[0x8009E460,0x8009E47C)` |
| direct SetSprt A header/RGB | `[0x8009E498,0x8009E4A0)` |
| direct SetSprt B header/RGB | `[0x8009E4A8,0x8009E4B0)` |
| PolyF3 header | `[0x8009E4D8,0x8009E4E0)` |
| L7 sprites | `[0x8009E3B8,0x8009E40C)` |

The wrapped sprite uses U/V `E8/E0`, CLUT `0x7E13`, and dimensions
`24 x 24`. The direct records receive SetSprt's retail header bytes
`length=3`, `code=0x40`, with RGB `E0/E0/E0` and `60/60/60`. The PolyF3
header is `length=4`, `code=0x20`. Each L7 wrapper has RGB `0x80`, CLUT
`0x7E13`, and dimensions `24 x 8`.

The production full-RAM canary admits only the ranges above. Focused
sentinels cover each exact end and the next L8 base. The dirty-state test also
proves retail-untouched bytes inside admitted records remain poisoned, so the
range whitelist cannot conceal broad record clearing.

## Independent oracle

`pc_port/tools/b54kd_30894_l7_oracle.py` imports no production code. It
requires the exact executable and whole-window hashes, decodes the complete
five-call census, checks 52 selected literal/address/store/control words,
proves the L7 branch target, bound, and `0x54` extent, and verifies both words
adjacent to the cut.

```text
  OK retail window: 78 words, SHA-256 exact
  OK call census: five jal sites in retail order
  OK selected literal/control words: 52
  OK L7 loop: head 0x80031040, bound 3, extent 0x54
  OK cut: prior L6 delay slot and first excluded L8 word exact

B54K-D oracle: 5 check groups passed.
```

## Focused tests and self-audit

1. `B54KD_30894_fixed_primitives_and_l7` checks every proven header, color,
   UV, CLUT, and dimension field, all range-end sentinels, the named cut, and
   unresolved-boundary stop reason.
2. `B54KD_30894_dirty_repeat_deterministic` poisons the complete `0x88`-byte
   union, snapshots it after each of two executions, compares both results,
   and verifies representative untouched bytes remain poisoned.

The first full run exposed an oracle-side expectation error: the test had
incorrectly expected SetSprt header bytes `4/0x64`. Reading the already-proven
`func_80077C64` implementation showed retail SetSprt writes `3/0x40`; the
production source already called that exact native leaf. Only the four
mistaken expected bytes were corrected. Both B54K-D tests then passed without
a production change.

Normal suite:

```text
TEST B54KD_30894_fixed_primitives_and_l7... PASS
TEST B54KD_30894_dirty_repeat_deterministic... PASS
Results: 936 run, 936 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 936 run, 936 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L7
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L7_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
L8_EXECUTION=absent
```
