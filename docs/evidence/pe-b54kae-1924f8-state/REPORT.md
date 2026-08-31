# PE-B54K-AE — `func_801924F8` movie-state setup

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung advances the authenticated `func_801924F8` prefix through the
call-free setup immediately after the retail FMV file search. It stops before
the first unresolved overlay-local call and does not approximate that call.

## Retail identity and boundary

```text
complete function   [0x801924F8,0x80192934)
complete size       0x43C / 271 words
complete SHA-256    ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a
new setup block     [0x80192614,0x80192728)
setup size          0x114 / 69 words
setup SHA-256       dc9862aea7cf86ce063394d2c9ddae42774de12940250c8c219de3ab42ef1eca
translated prefix  [0x801924F8,0x80192728)
prefix size         0x230 / 140 words
prefix SHA-256      526fb8ca0e5558585d3fc06b8d6977d9999d648c57a5c259d69b214f3fd2fc1e
next instruction   0x80192728 jal 0x8010BE3C
delay-slot word    0x8019272C move a0,zero
normal return       0x8019292C jr ra / 0x80192930 nop
exact-start caller  0x80192E00
```

The new block has no `jal` instructions. The cut is therefore on the first
new semantic dependency, not in the middle of a local state operation.

## Proven state contract

Retail performs these effects after `DsSearchFile` succeeds:

1. copy the four-byte `CdlLOC` at `0x801D0DC4` to `0x801D0DDC`;
2. reload the selected 20-byte record through `0x801D11AC`;
3. copy `0x801D0DE8`, `0x801D0DEC`, `0x801D0DF0`, and `0x801D0DF4`
   into the pointer fields beginning at `0x801D1464`;
4. clear bytes `0x801D146C` and `0x801D1478`;
5. read the record halfwords at `+0x0A` and `+0x0C`, then build pairs
   `(x,y+240)` and `(x,y)` at `0x801D147A` and `0x801D1482`;
6. store the low byte of `D_800ACDDC` at `0x801D148A`, select a pair at
   `0x801D1464 + ((D_800ACDDC & 0xFF) << 3)`, and copy its `+0x16/+0x18`
   halfwords to `0x801D148C/0x801D148E`;
7. store 24 at `0x801D1490` when the signed movie-kind byte is nonzero,
   otherwise 16, and clear `0x801D1494`.

The paired values and `+240` establish coordinate behavior. No unproven PsyQ
type or overlay-local semantic name is assigned to the surrounding structure.

## Independent controls

The real Disc 1 path reaches this block through the complete 133-sector
overlay load and the authentic `FMV001.STR;1` search. For record index 1 it
proves the four pointer values already produced by `func_80191FB8`, buffer 0
selection, selected coordinates `(0,240)`, and nonzero-kind result 24.

A synthetic FMV2 fixture poisons the output block before entry, then supplies:

```text
record index       21
record coordinates (17,33)
display buffer     1
movie kind         0
four pointer words distinct valid guest addresses
```

The result must select `(17,33)`, store 16, reproduce all four pointers, copy
the exact `CdlLOC`, and clear all three retail zero bytes. This independently
covers the opposite active-pair and kind branches. The fixture is test-only;
it contains no scene, scheduler, persistence, or destination state.

## Verification

```text
B54K-AE independent oracle: PASS
B54K-AD regression oracle:  PASS
B54K-AC regression oracle:  PASS
normal CTest:                2/2 PASS
native suite:                987/987
fresh ASan/UBSan CTest:      2/2 PASS
strict real-disc exit:       1
strict frontier:             func_801924F8_80192728_cut
retail EXE SHA-1:            452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No behavior from `func_8010BE3C`, movie playback, scheduler selection, story
state, or destination state is fabricated.

```text
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_140_WORDS
MOVIE_STATE_SETUP=RETAIL_69_WORD_BLOCK_PROVEN
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192728_cut
NEXT_ARTIFACT_FREE_RUNG=audit_func_8010BE3C_call_and_following_movie_setup
```
