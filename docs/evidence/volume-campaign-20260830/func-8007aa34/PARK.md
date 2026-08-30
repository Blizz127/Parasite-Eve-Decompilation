# func_8007AA34 — duplicate CdPosToInt residual family

Status: `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING-FAMILY`.

Disposition: `ACCEPTED-RESIDUAL`. This is evidence disposition only, not a
byte-identity claim. The leaf remains assembly and does not increment the
matching-C count.

An explicit new close-out campaign authorization reset the historical H5
stop recorded by VOLUME-108. This row was screened as a proven byte-identical
family member, so no duplicate compiler attempt was spent.

## Function hood and boundaries

The exact span is file `[0x6B234,0x6B2B4)`, VRAM
`[0x8007AA34,0x8007AAB4)`: `0x80` bytes / 32 words. It ends with `jr ra` at
`0x8007AAAC` and a live `addiu v0,v0,-150` in the delay slot at
`0x8007AAB0`.

The direct `jal` at caller PC `0x8007C2C0` (file `0x6CAC0`) targets the exact
start. Its word is `0C01EA8D` (little-endian file bytes `8D EA 01 0C`). The
caller passes `D_800A3490` and consumes the returned logical-block address.

The preceding real `func_8007A930` ends immediately at file
`0x6B22C/0x6B230` with `jr ra` and a live byte store. The following real
`func_8007AAB4` starts immediately at `0x6B2B4` with
`addiu sp,sp,-0x30`.

```text
FUNCTION_HOOD=PROVEN_BY_1_DIRECT_CALLER_AND_CANONICAL_RETURN
```

## Exact family proof

The complete retail span is byte-for-byte identical to the independently
hood-proven `func_80080C48` span at file `[0x71448,0x714C8)`:

```text
sha256(func_8007AA34 retail 0x80 bytes)
  = 91ea1fb723b4ee635699703080fc31c086af6ade091e87f0fb3c93316bdfecf2
sha256(func_80080C48 retail 0x80 bytes)
  = 91ea1fb723b4ee635699703080fc31c086af6ade091e87f0fb3c93316bdfecf2
word comparison = 32/32
```

The body decodes packed BCD minute/second/frame bytes and returns:

```text
((minute * 60 + second) * 75 + frame) - 150
```

All screens therefore transfer exactly from the proven twin: no frame,
callee, global, or loop; the input pointer remains in `$a0` until the delayed
byte-2 load; `$v0` is the arithmetic/return accumulator; and era
`-O2 -G0` is selected by the dense strength reductions and live return slot.
The independent native `CdPosToInt` vectors documented for `func_80080C48`
also prove the semantics.

## Inherited bounded compiler evidence

`func_80080C48` already exhausted the two permitted natural-C phrasings for
this exact 32-word target. Both generated 32 words, but cc1 hoisted the
independent byte-2 load to entry and changed the BCD accumulator schedule and
register homes. The closer explicit-accumulator object was:

```text
90830000 90860001 90840002 00031102 00022880 00A22821 00052840 3063000F
00A32821 00061102 00021880 00621821 00031840 30C6000F 00661821 00051100
00451023 00021080 00432821 00051080 00451021 00021900 00622823 00041902
00031080 00431021 00021040 3084000F 00441021 00A22821 03E00008 24A2FF6A
```

The shared retail target is:

```text
90830000 90860001 00032902 00051080 00451021 00021040 3063000F 00431021
00022900 00A22823 00052880 00061902 00031080 00431021 00021040 30C6000F
00461021 00A22821 00051880 00651821 00031100 90850002 00431023 00052102
00041880 00641821 00031840 30A5000F 00651821 00431021 03E00008 2442FF6A
```

Recompiling the same semantic source under a second symbol would test the
same cc1 decisions against an identical target and cannot provide new
evidence. The family screen therefore consumes zero phrasings.

```text
RESIDUAL_MECHANISM=INDEPENDENT_LOAD_AND_BCD_ACCUMULATOR_SCHEDULING_FAMILY
FUTURE_UNBLOCKER=the same compiler scheduler/allocation lever required by
  func_80080C48: keep byte 2 at its retail use point and pipeline decoded BCD
  fields through the retail accumulator homes
```

The semantic candidate is preserved in labeled stash
`park func_8007AA34 duplicate CdPosToInt residual`. There is no YAML, build,
or verifier integration. The exact matching-C count remains 340.
