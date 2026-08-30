# PE-B54K-E — `func_80030894` L8 sprite loop

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the complete L8 sprite loop after L7. The cut is placed
at the first instruction of the next fixed-sprite group, so no neighboring
record is partially initialized.

## Retail identity and cut

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x800310A4,0x80031110)
file range       [0x000218A4,0x00021910)
size             0x6C bytes / 27 words
window SHA-256   9b87877a27ab26756ab9cfbdf9f6f5d4e31958975ca654fcdeaf87660f7b7ecb
prior word       0x800310A0  sb s7,0x06(s0) (L7 delay slot)
first word       0x800310A4  move s6,zero
last word        0x8003110C  sb s7,0x06(s0) (L8 delay slot)
next word        0x80031110  lui s0,%hi(D_8009E768)
```

The native prefix now covers:

```text
[0x80030894,0x80031110) = 0x87C bytes / 543 words
```

The named strict boundary moves from `func_80030894_L7_cut` to
`func_80030894_L8_cut`. No instruction from the following fixed group
executes here.

## Retail control and address geometry

The window contains one static call at `0x800310E4` to
`func_800370DC`; the ten loop iterations make ten runtime calls.

Retail computes the bank stride as:

```text
bank * 8
+ bank       = bank * 9
* 4          = bank * 36
- bank       = bank * 35
* 8          = bank * 280
```

The per-slot arithmetic is `(slot * 8 - slot) * 4 = slot * 28`. The
unsigned-byte loop counter is bounded by `sltiu ...,10`, giving:

```text
base        D_8009E500
bank stride 280 bytes
slot stride 28 bytes
iterations  10
packet span 10 * 28 = 280 = 0x118 bytes
range       [0x8009E500,0x8009E618) for bank zero
```

Each iteration calls the already-proven wrapped-sprite builder, then writes
RGB `0x80/0x80/0x80`. The final packet state has compound length `6`, draw
mode word `0xE1000234`, zero appended tag, and sprite code `0x64`. Retail
does not write bytes `+0x10..+0x1B` in this loop; it supplies no CLUT or
dimensions here.

## Independent oracle

`pc_port/tools/b54ke_30894_l8_oracle.py` imports no production code. It
requires the exact executable and window hashes, compares all 27 words,
decodes the sole static call, proves the loop back edge/bound and both
strides, closes the `0x118` extent, and checks both cut-side boundary words.

```text
  OK retail window: 27 words, SHA-256 exact
  OK call census: one jal func_800370DC site
  OK complete word comparison: 27/27
  OK L8 loop: head 0x800310CC, bound 10, strides 280/28, extent 0x118
  OK cut: prior L7 delay slot and first excluded group word exact

B54K-E oracle: 6 check groups passed.
```

## Focused tests and gates

1. `B54KE_30894_l8_ten_sprites` checks all ten headers/RGB values, the byte
   before the array, the exact end, the next fixed-group base, the named cut,
   and the unresolved-boundary stop reason.
2. `B54KE_30894_l8_dirty_repeat_deterministic` poisons the entire `0x118`
   span, compares complete first/repeated results, and proves all twelve
   retail-untouched tail bytes in every packet retain poison.

Normal suite:

```text
TEST B54KE_30894_l8_ten_sprites... PASS
TEST B54KE_30894_l8_dirty_repeat_deterministic... PASS
Results: 938 run, 938 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 938 run, 938 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L8
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L8_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
POST_L8_FIXED_GROUP=absent
```
