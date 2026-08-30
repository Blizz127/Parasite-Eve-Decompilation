# PE-B54K-C — `func_80030894` fixed records and L6

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the next coherent retail group after L5: two G4
records, three fixed shaded sprites, and the complete L6 tile loop. The cut
was deliberately placed after L6 because retail interleaves each sprite's
setup with the next record; an earlier cut would leave a record half-filled.

## Retail identity and cut

```text
executable SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
VA range         [0x80030D20,0x80030F6C)
file range       [0x00021520,0x0002176C)
size             0x24C bytes / 147 words
window SHA-256   51fe2651b8b7df743b8fc277f79101757a0b0904c4fbee00d28dc4a30bf19125
prior word       0x80030D1C  andi v0,s6,0x00FF (L5 delay slot)
first word       0x80030D20  lbu s5,0x18(sp)    (bank reload)
last word        0x80030F68  andi s1,s6,0x00FF (L6 delay slot)
next word        0x80030F6C  lui s0,%hi(D_8009E460)
```

The native prefix now covers:

```text
[0x80030894,0x80030F6C) = 0x6D8 bytes / 438 words
```

The named strict boundary moves from `func_80030894_L5_cut` to
`func_80030894_L6_cut`. The next retail word begins a separate sprite group;
no part of that group executes here.

## Call census

The exact retail window contains nine static call sites:

| PC | Callee | Role |
| --- | --- | --- |
| `0x80030D3C` | `func_80077BC4` | first G4 header |
| `0x80030D4C` | `func_80077BC4` | second G4 header |
| `0x80030DF8` | `func_800370DC` | fixed sprite A |
| `0x80030E0C` | `func_80077B34` | shade-texture bit for A |
| `0x80030E5C` | `func_800370DC` | fixed sprite B |
| `0x80030E70` | `func_80077B34` | shade-texture bit for B |
| `0x80030EBC` | `func_800370DC` | fixed sprite C |
| `0x80030ED0` | `func_80077B34` | shade-texture bit for C |
| `0x80030F34` | `func_80077C44` | L6 direct tile header, inside loop |

All callees were already real native implementations. No new SDK collapse,
callback, provider, or host-only state was introduced.

## Address and state geometry

Retail's register arithmetic decodes to:

```text
G4 bank stride     = bank * 72
sprite bank stride = bank * 28
L6 bank stride     = bank * 48
L6 slot stride     = slot * 16
L6 bound           = (slot & 0xFF) < 3
```

The bank-zero write set is the exact union below; the production
write-footprint guard admits these ranges individually rather than one broad
envelope:

| Object | Range |
| --- | --- |
| G4 A | `[0x800B0130,0x800B0150)` |
| G4 B | `[0x800B0154,0x800B0174)` |
| sprite A | `[0x8009E0B8,0x8009E0D4)` |
| sprite B | `[0x8009E2E8,0x8009E304)` |
| sprite C | `[0x8009E320,0x8009E33C)` |
| L6 tiles | `[0x8009E358,0x8009E388)` |

The four-byte G4 gap at `0x800B0150`, both outer G4 sentinels, the first byte
after L6, and next-group base `0x8009E460` are preserved negative controls.

The first G4's two RGB vertices are `00/82/36` and `4A/FF/3B`; the second's
are `FF/3D/81` and `83/13/01`. The fixed sprites use U values
`0x50/0x58/0x60`, V `0xF4`, CLUT `0x7E13`, dimensions `8 x 4`, RGB
`0x80`, and the shade-adjusted sprite code `0x65`.

L6 builds three direct SetTile packets. Each uses one prologue byte from
`D_8009CD90[0..2]`, replicated across R/G/B. Focused inputs
`80/A5/FF` prove the original signed `lb` followed by `lbu` recovers the raw
byte values rather than sign-extending the packet color.

## Independent oracle

`pc_port/tools/b54kc_30894_l6_oracle.py` imports no production code. It
requires the exact executable and whole-window hashes, decodes the complete
nine-call census, checks 55 selected literal/address/store/control words,
proves the L6 branch target and three-iteration bound, and verifies the first
excluded word.

```text
  OK retail window: 147 words, SHA-256 exact
  OK call census: nine jal sites in retail order
  OK selected literal/control words: 55
  OK L6 loop: head 0x80030F2C, bound 3, extent 0x30
  OK cut: first excluded word materializes D_8009E460

B54K-C oracle: 5 check groups passed.
```

## Focused tests and gates

1. `B54KC_30894_fixed_records_and_l6` checks all proven G4/sprite/tile
   fields, non-contiguous sentinels, the named cut, and stop reason.
2. `B54KC_30894_dirty_repeat_deterministic` poisons all six exact ranges,
   snapshots their 0xC4-byte union, executes twice, and compares the complete
   result before rechecking semantic fields. It also proves all six G4
   coordinate words and all six unused L6 tail words retain the poison value,
   so the range whitelist does not conceal within-record over-writes.

All earlier B54K tests remain green:

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_fixed_setup_and_l4... PASS
TEST B54KB1_30894_dirty_repeat_deterministic... PASS
TEST B54KB2_30894_l5_five_packets... PASS
TEST B54KB2_30894_l5_dirty_repeat_deterministic... PASS
TEST B54KC_30894_fixed_records_and_l6... PASS
TEST B54KC_30894_dirty_repeat_deterministic... PASS
Results: 934 run, 934 passed, 0 failed, 0 skipped
```

Rebuilt ASan/UBSan suite:

```text
Results: 934 run, 934 passed, 0 failed, 0 skipped
sanitizer_diagnostics=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L6
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L6_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
POST_L6_EXECUTION=absent
```
