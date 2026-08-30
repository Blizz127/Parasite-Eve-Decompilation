# PE-B54K-H — `func_80030894` fixed sprite and descriptor-driven L11

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the complete group after L10: one fixed wrapped sprite
and the thirteen descriptor-driven L11 sprites. The cut is the first word of
the function's final TPage/fixed-sprite/outer-loop epilogue.

## Retail identity and cut

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x80031320,0x80031438)
file range       [0x00021B20,0x00021C38)
size             0x118 bytes / 70 words
window SHA-256   bdfec78193cfc60b0ed14829f5f3fb42ce74db2cbfe0431ad402f174fd665538
prior word       0x8003131C  andi v0,s6,0x00FF (L10 delay slot)
first word       0x80031320  lui s2,%hi(D_8009E928)
last word        0x80031434  andi s1,s6,0x00FF (L11 delay slot)
next word        0x80031438  move a0,zero
```

The native prefix now covers:

```text
[0x80030894,0x80031438) = 0xBA4 bytes / 745 words
```

The full function is 788 words, leaving one exact 43-word / `0xAC`-byte
epilogue `[0x80031438,0x800314E4)`. The named strict boundary moves from
`func_80030894_L10_cut` to `func_80030894_L11_cut`; no epilogue instruction,
outer-bank increment, or second-bank packet construction executes.

## Call and descriptor contract

The exact window's static calls are:

| PC | Callee | Role |
| --- | --- | --- |
| `0x8003133C` | `func_800370DC` | fixed sprite wrapper |
| `0x80031394` | `func_8005DADC` | descriptor `0x6A + slot` |
| `0x800313AC` | `func_80077A64` | L11 TPage from `(0,0,0x1C0,0)` |
| `0x800313C8` | `func_800370DC` | L11 packet wrapper |

Retail invokes the descriptor, TPage, and wrapper calls once per slot. The
TPage result is masked to 16 bits and is `7` for the proven arguments. The
test descriptors deliberately give every slot distinct U, V, CLUT, width,
and height values, so an index, stride, field-offset, or stale-value error
cannot collapse into a uniform fixture.

## Address and packet geometry

For bank zero, production admits two separate ranges:

| Object | Range | Fields added by this rung |
| --- | --- | --- |
| fixed sprite | `[0x8009E928,0x8009E944)` | wrapper, RGB `80/80/00`, CLUT `7E13`; XY/UV/dimensions untouched |
| L11 sprites | `[0x8009E960,0x8009EACC)` | wrapper, RGB `80`, descriptor U/V/CLUT/width/height; XY untouched |

The fixed sprite uses bank stride 28. L11 uses bank stride 364, slot stride
28, unsigned bound 13, and closes exactly after:

```text
13 * 28 = 364 = 0x16C bytes
```

Each descriptor comes from `func_8005DADC(0x6A + slot)`. Bytes `+0/+1`
become packet U/V at `+0x14/+0x15`, the halfword at `+2` becomes CLUT at
`+0x16`, and bytes `+4/+5` are zero-extended into width/height halfwords at
`+0x18/+0x1A`.

## Independent oracle

`pc_port/tools/b54kh_30894_l11_oracle.py` imports no production code. It
requires the exact executable/window hashes, compares all 70 words, decodes
all four calls, checks descriptor/TPage constants, proves the L11 back edge,
bound, strides and extent, and verifies both cut-side words.

```text
  OK retail window: 70 words, SHA-256 exact
  OK call census: fixed wrapper, descriptor, TPage, L11 wrapper
  OK complete word comparison: 70/70
  OK descriptor/TPage contract: index 0x6A+slot, mode 7 masked
  OK L11: head 0x80031394, bound 13, strides 364/28, extent 0x16C
  OK cut: prior L10 delay slot and first excluded word exact

B54K-H oracle: 7 check groups passed.
```

## Focused tests and gates

1. `B54KH_30894_fixed_sprite_and_l11_descriptors` checks every written
   field with patterned descriptors, the two exact ranges, five boundary/gap
   sentinels, the named cut, and the unresolved-boundary stop.
2. `B54KH_30894_l11_dirty_repeat_deterministic` poisons and snapshots both
   packet ranges, compares repeated output, and proves the fixed packet's
   XY/UV/dimensions and every L11 XY word remain untouched.

Normal suite:

```text
TEST B54KH_30894_fixed_sprite_and_l11_descriptors... PASS
TEST B54KH_30894_l11_dirty_repeat_deterministic... PASS
Results: 944 run, 944 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 944 run, 944 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L11
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L11_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
FINAL_EPILOGUE_AND_OUTER_BANK_ADVANCE=absent
```
