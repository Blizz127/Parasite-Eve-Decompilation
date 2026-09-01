# `func_8003F758` — screened handwritten COP2 helper

No C leaf is claimed. Matching-C count remains 332.

## Function hood

Retail span `[0x2FF58,0x2FF98)`, VRAM `[0x8003F758,0x8003F798)`, is
`0x40` bytes / sixteen words. It ends in canonical `jr ra` with the final
halfword store in the delay slot.

There is one exact direct caller at `0x8003F0D0`. The call supplies
`D_800BEA40` in `$a0`, `0x28` in `$a1/$a2/$a3`, with the final `$a3`
constant in the call delay slot.

The preceding real function `func_8003F3C4` owns `jr ra; nop` at
`0x8003F750/0x8003F754`. The following real handwritten function
`func_8003F798` begins at `0x8003F798` with `lhu v1,0x10(sp)`. Both boundary
sides are executable instructions; this is not padding or a function tail.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER_AND_CANONICAL_RETURN`.

## Retail body and semantics

The split itself marks this function handwritten. Its complete body is:

```text
00056100  sll   t4,a1,4
00066900  sll   t5,a2,4
00077100  sll   t6,a3,4
48CC6800  ctc2  t4,$13
48CD7000  ctc2  t5,$14
48CE7800  ctc2  t6,$15
A4800000  sh    zero,0x00(a0)
A4800002  sh    zero,0x02(a0)
A4800004  sh    zero,0x04(a0)
A4800006  sh    zero,0x06(a0)
A4800008  sh    zero,0x08(a0)
A480000A  sh    zero,0x0A(a0)
A480000C  sh    zero,0x0C(a0)
A480000E  sh    zero,0x0E(a0)
03E00008  jr    ra
A4800010  sh    zero,0x10(a0)
```

Repository hardware definitions independently identify GTE control registers
13–15 as `RBK/GBK/BBK`. The helper shifts the three scalar arguments by four,
writes those architectural background-color registers, and zeroes nine
consecutive halfwords at the argument buffer. This establishes its machine
semantics without assigning an unproven Psy-Q routine name.

## Screen and disposition

The nine ordinary stores are C-expressible, but omitting the three `ctc2`
operations would omit the function's architectural side effects. The
repository-wide COP2 screen proves that sanctioned ordinary C exposes no
intrinsic for these writes; pins and inline/file-scope assembly are forbidden.
An ordinary-C attempt would therefore only rediscover the established
instruction-set expressibility blocker.

This function is not assigned to the contiguous proven libGTE SDK family at
`func_80078E04..func_80079024`: the GTE role is proven, but exact SDK provenance
for this mixed register/buffer helper is not.

`POOL_DISPOSITION=SKIP-HANDWRITTEN-COP2`

`INTEGRATION=NONE`

`MATCHING_C_COUNT=332`
