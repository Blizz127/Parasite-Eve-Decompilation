# PE-B54K-AF — bounded libpress `DecDCTReset`

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung crosses `func_801924F8`'s first movie-library call and translates
the complete SDK wrapper. It stops at the first MDEC-hardware routine; that
routine is neither skipped nor represented as a no-op.

## Module and function identity

```text
movie module source   PE.IMG sectors [0x039F,0x03C5)
movie module size     38 sectors / 0x13000 bytes
movie module load VA  0x8010BCF8
movie module SHA-256  d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40
wrapper               [0x8010BE3C,0x8010BE70)
wrapper size          0x34 / 13 words
wrapper SHA-256       d29b6f4ff5d6c611a9193862caffbce26eae005f5cca46747f4fc780ffdc4f9d
internal next target  [0x8010C0FC,0x8010C1EC)
internal size         0xF0 / 60 words
internal SHA-256      53670ac94ba335b065bac7d63504e80a75a3bffe01cc3dff07c1582da260808b
production caller     0x80192728 jal 0x8010BE3C / delay move a0,zero
caller prefix         [0x801924F8,0x80192730) / 142 words
caller-prefix SHA-256 d633247323e92b405eedfcea7d79d2078bc4217f77c263740d4dbeccc7dab180
```

Function-hood is proven by the exact production call, conventional prologue,
`jr ra` plus stack-restore delay slot, and the real next function at
`0x8010BE70`. The preceding words are zero alignment, so they are not used as
function evidence.

## SDK identity

The payload starts with these retail diagnostics:

```text
MDEC_rest:bad option(%d)
MDEC_in_sync
MDEC_out_sync
```

The surrounding functions include a 256-byte environment get/put pair,
MDEC input/output and sync wrappers, and DMA callback setters that pass
channels 0 and 1 respectively. Their order and signatures match Sony's
`libpress.h` declarations for `DECDCTENV`, `DecDCTReset`,
`DecDCTinCallback`, and `DecDCToutCallback`. This closes the prior
`sdk_map.md` MDEC-family unknown.

## Exact wrapper contract

Retail `func_8010BE3C` performs only:

```c
if (mode == 0)
    ResetCallback();
func_8010C0FC(mode);
```

The production call supplies mode zero. The native wrapper executes the
already-complete generic `func_80073C94` path, records the unchanged mode at
the exact internal target, and requests the established unresolved-boundary
stop. It does not write MDEC registers, complete DMA, upload tables, decode a
frame, or register the later callbacks.

The internal function is separately authenticated. Its mode-zero path writes
the MDEC control register, clears DMA0/DMA1 channel state, updates DPCR, and
uploads the retail quantization/scale tables. Those effects require a generic
MDEC substrate and are the next implementation unit.

## Controls

- Mode zero begins from dirty DPCR and a clear ResetCallback guard. It must
  finish the real ResetCallback path: guard 1, registered CPU-source mask 9,
  and DPCR `0x33333333`, then stop at `func_8010C0FC` with argument zero.
- Mode one begins from a different dirty DPCR with the guard clear. It must
  leave the guard, registered mask, and DPCR unchanged, then stop at the same
  target with argument one.
- The real Disc 1 path must reach the same target through the complete module
  load, filename search, and movie-state setup.

## Verification

```text
B54K-AF independent oracle: PASS
B54K-AE regression oracle:  PASS
B54K-AD regression oracle:  PASS
B54K-AC regression oracle:  PASS
normal CTest:                2/2 PASS
native suite:                988/988
fresh ASan/UBSan CTest:      2/2 PASS
strict real-disc exit:       1
strict frontier:             func_8010C0FC from func_8010BE3C
retail EXE SHA-1:            452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No MDEC completion, movie frame, XA audio, scheduler state, story state, or
destination state is fabricated.

```text
FUNC_8010BE3C=COMPLETE_RETAIL_DECDCTRESET_WRAPPER
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_142_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_8010C0FC_from_func_8010BE3C
NEXT_ARTIFACT_FREE_RUNG=generic_mdec_reset_and_table_upload
```
