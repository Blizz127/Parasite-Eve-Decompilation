# PE-B54K-T — `func_80190660` image/fade prefix to DrawPrim

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

> **B54K-U supersession note (2026-08-30):** B54K-U translates the first
> DrawPrim wrapper/worker path and generic GP0(E1h) state. The current
> `func_80075358` boundary is now the second call at `0x80190868`, carrying
> the four-word SPRT. The 128-word/973-test measurements below are B54K-T
> history.

This rung proves `func_80190660` is a real overlay function, enters it from
the already translated `func_801909B4`, and translates its first 128 words.
The represented path loads two table-derived images, synchronizes the large
transfer through B54K-S DrawSync, builds two parity banks of draw-mode and
SPRT packets, disables both draw environments, enables display output,
selects the first frame's environment, and stops at the first DrawPrim call.

No destination token, event scheduler, scene special case, or
`persist[0] |= 4` store was added.

## Function-hood and retail identity

The function label is supported by control flow, not inferred from a name:

```text
overlay package             PE.IMG sectors [0x03D2,0x0457)
overlay load address        0x8018EFF0
PE.IMG SHA-1                146c0ce7308bf9fdc2ba5a84230e198db0663f3b

func_80190660               [0x80190660,0x801909B4)
size                        0x354 / 213 words
SHA-256                     fe2cc07abe6f50d8959ec5dbfe268d5ab6ac0f2fc91ffadc10f69a07e8a192ee
translated word range       [0x80190660,0x80190860)
translated size             0x200 / 128 words
translated SHA-256          ddd9aebf80a55e3ab7e04662fa090462361cd46214a3abd8a753d95046f05932
```

The preceding function returns at `0x80190658` with a nop delay slot at
`0x8019065C`. `func_80190660` itself returns at `0x801909AC` with a nop delay
slot at `0x801909B0`; the following word is the real prologue of
`func_801909B4`. An exhaustive aligned scan of the loaded overlay finds one
exact-start caller:

```text
0x80190D74  jal 0x80190660
0x80190D78  nop
```

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALL_AND_CANONICAL_RETURN`.

## Table-derived image records

Retail does not hard-code either image address. It computes:

```text
offset word                 [0x80193278] = 0x0003BAC8
anchor                      0x80193254
first record                0x80193254 + 0x0003BAC8 = 0x801CED1C
first size                  [first+8] = 0x2C
second record               first + 8 + (0x2C & ~3) = 0x801CED50
```

The authenticated records are:

| Record | RECT | Pixel source |
| --- | --- | --- |
| first | `{0,480,16,1}` at `first+0x0C` | `first+0x14 = 0x801CED30` |
| second | `{512,256,64,64}` at `second+0x04` | `second+0x0C = 0x801CED5C` |

Both calls go through the complete PsyQ LoadImage wrapper and exact dispatcher
path. The 16x1 image is CPU-fed. The 64x64 image issues DMA2, and the
immediately following DrawSync supplies exactly one checkpoint opportunity
in the focused contract. The transfer is neither pre-completed nor advanced
by a polling read.

## Packet and frame-zero state

The direct-call sequence in the 128 authenticated words is:

```text
8007506C  8007506C  80074DC0  80077C84  80077C84  80074D28
LoadImage LoadImage DrawSync  SetDrawMode SetDrawMode SetDispMask
```

Retail builds two parity banks. Each 16-byte draw-mode bank contains two
one-word commands, `0xE1000018` and `0xE1000019`. Each corresponding
40-byte primitive bank contains two 20-byte SPRTs:

| Field | SPRT 0 | SPRT 1 |
| --- | ---: | ---: |
| length/code | `4 / 0x64` | `4 / 0x64` |
| x,y | `32,88` | `284,80` |
| u,v | `0,0` | `0,0` |
| clut | `0x7800` | `0x7800` |
| w,h | `256,64` | `8,80` |

The prefix clears byte `+0x6D` in both environments, calls SetDispMask(1),
and starts frame zero. Retail's selector is `old == 0 ? 1 : 0` (the exact
`sltiu old,1` behavior, not XOR): it stores the new selector at
`D_801D11C8` and the selected pointer from
`D_801D11BC[next]` at `D_801D11C4`. The complete fade formula is retained;
frame zero produces RGB intensity zero.

## Honest DrawPrim boundary

The next instruction pair is:

```text
0x80190860  jal func_80075358
0x80190864  sb  s0,4(s1)       # final RGB byte in the delay slot
```

Native applies the authenticated delay-slot RGB store before exposing the
call. The packet lives on retail's stack, so native retains no guest or host
pointer to it. Bytes 0..2 of the stack tag are uninitialized in retail and
are deliberately excluded from evidence. The diagnostic boundary contains:

```text
symbol                      func_80075358
caller                      func_80190660
target                      0x80075358
arg0                        0 (no retained native pointer)
arg1                        1 (lbu packet[3] length)
payload_size                4
payload                     18 00 00 E1 (0xE1000018)
```

This is a value-only snapshot. It does not become queue authority and does
not disguise DrawPrim as implemented.

## Tests and independent oracle

Two new focused contracts prove:

1. both exact record geometries and sources, VRAM endpoints, one large-image
   DMA checkpoint, environment clears, zero-to-one toggle, selected pointer,
   display state, and exact DrawPrim payload;
2. a dirty nonzero toggle selects zero/environment 0, preserving retail's
   boolean conversion rather than assuming a one-bit input.

The retained eight B54K-R integration contracts now traverse this prefix as
well. Their full-RAM allowlist includes only the newly authenticated current
environment/toggle words and environment bytes.

```text
focused B54K-T:    973 run, 2 passed, 0 failed, 971 skipped
focused B54K-R:    973 run, 8 passed, 0 failed, 965 skipped
normal full suite: 973 run, 973 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  973 run, 973 passed, 0 failed, 0 skipped
sanitizer diagnostics: 0
```

`pc_port/tools/b54kt_190660_drawprim_prefix_oracle.py` imports no production
code. It authenticates PE.IMG, all 213 function words, the exact 128-word
prefix, both function boundaries, the sole caller, the DrawPrim instruction
pair, record arithmetic and RECTs, direct-call sequence, packet/fade model,
source fences, focused tests, and the measured real-disc frontier.

```text
  OK hood: 213 words, normal return, one caller, real boundaries
  OK prefix: 128 words; first DrawPrim + RGB delay slot next
  OK records: 801CED1C -> 801CED50; 16x1 and 64x64 images
  OK model: two DR_MODE words, SPRT constants, frame-zero RGB=0
  OK source: transient payload only; no scheduler or scene special case
  OK runtime: 2 focused contracts and real-disc DrawPrim frontier

B54K-T func_80190660 prefix oracle: PASS.
```

## Production result and gates

Strict real-disc execution now reports:

```text
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80075358
       called from: func_80190660
```

Normal execution reaches the same provider and reports:

```text
[FB] vsyncs=6 drawsyncs=4 presents=3 mask=1 main_iters=1
[HOST] stop_reason=unresolved-boundary
[DMA_CHECKPOINT] calls=27 queries=26 services=26 captured=26 serviced=26
```

The aggregate checkpoint count remains 27/26 on the complete run; the focused
`func_80190660` test separately proves its 64x64 transfer contributes exactly
one captured/serviced token. Earlier runtime phases and this prefix share the
same single checkpoint authority.

```text
git diff --check:       PASS
disc1.candidate SHA-1:  452fb033f2eaa4b18aa20a5bca60b8125af3a37b
FUNC_80190660_PREFIX=128_WORDS_TRANSLATED
OVERLAY_IMAGE_RECORDS=TABLE_DERIVED_AND_EXECUTED
TRANSIENT_PACKET_POINTER=NOT_RETAINED
PRODUCTION_REACHABILITY=blocked_at_func_80075358_from_func_80190660
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=translate_func_80075358_DrawPrim_and_exact_worker_subset
```
