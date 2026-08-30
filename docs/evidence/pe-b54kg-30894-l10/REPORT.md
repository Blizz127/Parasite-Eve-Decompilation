# PE-B54K-G — `func_80030894` post-L9 fixed sprites and L10

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the complete call-coupled group after L9: two fixed
wrapped sprites, the first sprite's separately computed CLUT, and the
complete two-sprite L10 loop. The cut is at the next fixed-group base.

## Retail identity and cut

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x800311EC,0x80031320)
file range       [0x000219EC,0x00021B20)
size             0x134 bytes / 77 words
window SHA-256   b8eafebde2564d8c3315c39c6b7e37ae8fe04036c525f8bdd2d12a761d0e86e2
prior word       0x800311E8  sb s7,0x06(s0) (L9 delay slot)
first word       0x800311EC  lui s0,%hi(D_8009E730)
last word        0x8003131C  andi v0,s6,0x00FF (L10 delay slot)
next word        0x80031320  lui s2,%hi(D_8009E928)
```

The native prefix now covers:

```text
[0x80030894,0x80031320) = 0xA8C bytes / 675 words
```

The named strict boundary moves from `func_80030894_L9_cut` to
`func_80030894_L10_cut`. No `D_8009E928` group instruction executes.

## Call order and CLUT derivation

The exact window's four static calls are:

| PC | Callee | Role |
| --- | --- | --- |
| `0x80031208` | `func_800370DC` | first fixed sprite |
| `0x8003122C` | `func_80077AA4` | first fixed sprite CLUT |
| `0x80031268` | `func_800370DC` | second fixed sprite |
| `0x800312E0` | `func_800370DC` | L10 wrapper, two runtime calls |

The source preserves that order. Retail calls the first wrapper, writes its
UV bytes, computes the CLUT, completes the packet, and only then calls the
second wrapper.

The CLUT helper receives `(x=0x130,y=0x1F9)`. Its proven packed-coordinate
contract gives:

```text
((0x1F9 << 6) | ((0x130 >> 4) & 0x3F)) & 0xFFFF = 0x7E53
```

The second fixed sprite and L10 retain the prologue CLUT `0x7E13`.

## Address and packet geometry

For bank zero, production admits three separate ranges:

| Object | Range | Fields added by this rung |
| --- | --- | --- |
| first fixed sprite | `[0x8009E730,0x8009E74C)` | RGB `80`, UV `68/F4`, CLUT `7E53`, `24 x 4` |
| second fixed sprite | `[0x8009E880,0x8009E89C)` | RGB `80`, UV `7C/EF`, CLUT `7E13`, `36 x 5` |
| L10 sprites | `[0x8009E8B8,0x8009E8F0)` | CLUT `7E13`, `6 x 6`; RGB untouched |

The fixed sprites use bank stride `28`. L10 uses bank stride `56`, slot
stride `28`, unsigned bound `2`, and exact packet span `2*28 = 0x38`.
All four packets receive the proven wrapper header (`length=6`, draw-mode
word `0xE1000234`, zero appended tag, code `0x64`). Retail leaves each XY
word untouched and additionally leaves L10 RGB untouched.

## Independent oracle

`pc_port/tools/b54kg_30894_l10_oracle.py` imports no production code. It
requires the exact executable/window hashes, compares all 77 words, checks
the complete call order, proves the CLUT inputs/value, validates the L10
edge/bound/strides/extent, and checks both cut-side words.

```text
  OK retail window: 77 words, SHA-256 exact
  OK call census: wrapper, CLUT, wrapper, L10 wrapper
  OK complete word comparison: 77/77
  OK CLUT input/value: (0x130,0x1F9) -> 0x7E53
  OK L10: head 0x800312CC, bound 2, strides 56/28, extent 0x38
  OK cut: prior L9 delay slot and first excluded group word exact

B54K-G oracle: 7 check groups passed.
```

## Focused tests and gates

1. `B54KG_30894_fixed_sprites_and_l10` checks every written field, all three
   exact ranges, gaps/end sentinels, next-group base, named cut, and stop
   reason.
2. `B54KG_30894_l10_dirty_repeat_deterministic` poisons and snapshots the
   complete `0x70`-byte range union, compares repeated output, and proves
   fixed XY plus L10 RGB/XY remain untouched.

Normal suite:

```text
TEST B54KG_30894_fixed_sprites_and_l10... PASS
TEST B54KG_30894_l10_dirty_repeat_deterministic... PASS
Results: 942 run, 942 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 942 run, 942 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L10
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L10_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
POST_L10_D_8009E928_GROUP=absent
```
