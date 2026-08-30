# PE-B54K-B1 — `func_80030894` fixed setup and L4 continuation

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung advances the native production implementation of
`func_80030894` from the end of its L2/L3 packet nest through the complete
bank-local fixed setup and L4 packet loop. It does not implement or infer any
event scheduler behavior, destination token, `m0360i` route, or persistent
story-state write.

## Retail identity and cut

The executable used by both the oracle and the port tests is the exact Disc 1
candidate:

```text
SHA-1  452fb033f2eaa4b18aa20a5bca60b8125af3a37b
load   0x8000F800
```

The newly translated continuation is:

```text
VA range       [0x80030AC4, 0x80030C9C)
file range     [0x000212C4, 0x0002149C)
size           0x1D8 bytes / 118 words
window SHA-256 79fa2086406d087e2781025634ec533d50e262f3543f5916eedcdd383da8f0f6
prior word     0x80030AC0  addu s3,zero,zero   (L2 back-edge delay slot)
first word     0x80030AC4  addu a0,zero,zero   (fixed-setup argument 0)
last word      0x80030C98  andi v0,s6,0x00FF  (L4 back-edge delay slot)
next word      0x80030C9C  addu s6,zero,zero   (first L5 instruction)
```

The full function remains the B54J-proven real function
`[0x80030894,0x800314E4)`, called by the sole direct `jal` at `0x8006B0AC`.
This rung is an internal continuation cut, not a new function claim.

The implemented prefix is now exactly:

```text
[0x80030894, 0x80030C9C) = 0x408 bytes / 258 words
```

The named strict boundary moves from `func_80030894_L2L3_cut` to
`func_80030894_L4_cut`. The stop remains explicit and unresolved; no host
continuation is invented beyond the retail cut.

## Call census

The independent oracle decodes every word in the new retail window and finds
exactly seven `jal` instructions, in this order:

| Call PC | Callee | Proven role in this window |
| --- | --- | --- |
| `0x80030AD0` | `func_80077A64` | build tile tpage from `(0,0,0,0)` |
| `0x80030AF4` | `func_80037140` | initialize/wrap the fixed tile packet |
| `0x80030B18` | `func_80077B04` | enable semi-transparency on its tail |
| `0x80030B30` | `func_80077C44` | initialize the fixed tile/G4 record header |
| `0x80030B78` | `func_80077BC4` | replace that header with the retail G4 header |
| `0x80030B94` | `func_800370DC` | initialize the fixed sprite record |
| `0x80030C58` | `func_800370DC` | initialize each of four L4 sprite packets |

All seven callees were already real native implementations before this rung.
No new SDK collapse, callback binding, or unresolved provider was added.

## Decoded state and address geometry

The outer bank byte is still zero in the translated prefix. The C retains the
retail bank formulas rather than flattening the addresses:

| Object | Retail address formula | Bank-0 written extent |
| --- | --- | --- |
| fixed tile + append tail | `0x8009E068 + bank*24` | `[0x8009E068,0x8009E080)` |
| fixed G4 record | `0x8009E098 + bank*16` | `[0x8009E098,0x8009E0A8)` |
| two state triplets | `0x800B00E8 + bank*36` | `[0x800B00E8,0x800B010C)` |
| fixed sprite record | `0x800B6920 + bank*28` | `[0x800B6920,0x800B693C)` |
| L4 packet `slot` | `0x8009E0F0 + bank*112 + slot*28` | `[0x8009E0F0,0x8009E160)` |

For L4, `slot` is the masked byte counter and the retail test is `slot < 4`.
Thus `4 * 28 = 112 = 0x70` bytes, closing exactly at `0x8009E160`.
The implementation writes the retail CLUT `0x7E13` and dimensions `6 x 10`
to each packet. A sentinel at `0x8009E160` proves there is no fifth packet.
`0x8009E1D0`, the base first materialized by L5, remains untouched.

The fixed records reproduce the retail constants, including RGB `30/30/30`,
G4 RGB `1D/3E/32`, state bytes `00/46/82` and `9F/FF/F9`, sprite UV
`C8/E0`, dimensions `4 x 8`, and the shared CLUT value.

## Focused native contract

Two new tests exercise the continuation:

1. `B54KB1_30894_fixed_setup_and_l4` verifies every fixed packet/state
   field, all four L4 packets, the exact L4/L5 extent, the named cut, and the
   unresolved-boundary stop reason.
2. `B54KB1_30894_dirty_repeat_deterministic` poisons the entire L4 extent,
   executes twice, compares the complete 0x70-byte result, and preserves an
   out-of-range sentinel. This is the negative/replay control.

The older B54K-A direct and live-through-`func_8006AD40` tests remain green,
as does the production write-footprint guard after extending its whitelist
only by the five retail-proven ranges above.

Focused output from the full run:

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_fixed_setup_and_l4... PASS
TEST B54KB1_30894_dirty_repeat_deterministic... PASS
```

## Independent oracle

`pc_port/tools/b54kb1_30894_l4_oracle.py` imports no production code. It
requires the exact executable SHA-1, hashes all 118 words, decodes the full
`jal` census, checks 39 selected address/store/control words, proves the L4
back edge and bound, and checks the first excluded L5 word.

```text
  OK retail window: 118 words, SHA-256 exact
  OK call census: seven jal sites in retail order
  OK selected literal words: 39
  OK L4 loop: head 0x80030C44, bound 4, cut before L5

B54K-B1 oracle: 4 check groups passed.
```

## Full gates and sanitizer audit

Normal native suite:

```text
Results: 930 run, 930 passed, 0 failed, 0 skipped
```

The fresh ASan/UBSan run initially exposed a pre-existing signed-left-shift
defect in GTE translation math (`pe_gte.c`, negative translation scaled by
12 bits). The hardware operation is signed multiplication by 4096, so commit
`469f13c` replaced both duplicated shifts with exact 64-bit multiplication.
The normal suite stayed 930/930 and the rebuilt sanitizer suite then reported:

```text
Results: 930 run, 930 passed, 0 failed, 0 skipped
sanitizer_diagnostics=0
```

## Negative-control verdict

```text
SEMANTIC_IMPLEMENTATION=func_80030894_through_L4
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L4_cut
SCHEDULER_PROVENANCE=unchanged_NEEDS_ARTIFACT
M0360I_SPECIAL_CASE=absent
DESTINATION_TOKEN_PLANT=absent
PERSIST_BIT_PLANT=absent
L5_EXECUTION=absent
```
