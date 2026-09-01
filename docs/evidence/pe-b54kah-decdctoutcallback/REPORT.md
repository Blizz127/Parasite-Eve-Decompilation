# PE-B54K-AH — retail `DecDCToutCallback` registration

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the complete libpress output-callback wrapper reached
immediately after retail `DecDCTReset(0)`. It registers a guest callback
identity; it does not invoke that callback, complete DMA, or decode a frame.

## Retail identity and function-hood

```text
movie module          PE.IMG sectors [0x039F,0x03C5)
module load VA        0x8010BCF8
wrapper               [0x8010C0D8,0x8010C0FC)
wrapper size          0x24 / 9 words
wrapper SHA-256       e74b3ac9231b36b93ae760b3c2cd8c018905679c0da656f07b8cbd164933d59b
production call       0x80192738 jal 0x8010C0D8
callback argument     0x80191DC8
caller prefix         [0x801924F8,0x80192740) / 146 words
caller SHA-256        34b3cf3eec558cde8a6979762bd762d50873bbc1d2c76f489e5e38d1762fad7e
```

Function-hood is proven independently of its SDK name: the wrapper has an
exact production call, a conventional prologue, `jr ra` plus `nop`, and real
neighboring code. The preceding function ends with its own `jr ra`/`nop` at
`0x8010C0D0/0x8010C0D4`; the next function begins with a real stack prologue
at `0x8010C0FC`. The callback argument is also an exact-start reference to a
real overlay function beginning at `0x80191DC8`.

## All wrapper words

```text
8010C0D8 27BDFFE8  addiu sp,sp,-0x18
8010C0DC AFBF0010  sw    ra,0x10(sp)
8010C0E0 00802821  move  a1,a0
8010C0E4 0C01CF3D  jal   0x80073CF4
8010C0E8 24040001  li    a0,1
8010C0EC 8FBF0010  lw    ra,0x10(sp)
8010C0F0 27BD0018  addiu sp,sp,0x18
8010C0F4 03E00008  jr    ra
8010C0F8 00000000  nop
```

The established complete `func_80073CF4`/`func_800746A0` path is the retail
DMA callback setter. Therefore the semantic wrapper is exactly:

```c
void func_8010C0D8(pe_addr_t callback)
{
    (void)func_80073CF4(1u, callback);
}
```

At the production call site, `lui/addiu` constructs `0x80191DC8`, the `jal`
targets the wrapper, and the delay slot sets `s0 = 1`. The first following
instruction at `0x80192740` is real code (`lui a0,0x801D`), not padding.

## Controls

- Reset leaves DMA callback slots 0, 1, and 2 clear.
- Registration writes only slot 1 to `0x80191DC8`.
- DICR becomes `0x00820000`: master enable plus channel-1 enable.
- No indirect callback boundary is reached, proving registration was not
  collapsed into delivery.
- The real Disc 1 path reaches the new strict cut with the same slot value.

## Verification

```text
B54K-AH independent oracle: PASS
B54K-AH focused test:       1/1 PASS
B54K-Y real-disc tests:     2/2 PASS
native suite:               990/990
normal CTest:               2/2 PASS
fresh ASan/UBSan CTest:     2/2 PASS
strict real-disc exit:      1
strict frontier:            func_801924F8_80192740_cut
retail EXE SHA-1:           452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No MDEC output, callback invocation, XA audio, scheduler state, story state,
persistence state, or destination state is fabricated.

```text
FUNC_8010C0D8=COMPLETE_RETAIL_DECDCTOUTCALLBACK_WRAPPER
DMA_CALLBACK_SLOT_1=0x80191DC8_REGISTERED_NOT_DELIVERED
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192740_cut
NEXT_ARTIFACT_FREE_RUNG=record_pool_initializer_at_0x80192748
```
