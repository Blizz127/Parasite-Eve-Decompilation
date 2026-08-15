# Atlas upload proof

TXT0-B (`3d9afc7`) identified the TIM and the writers. This rung
implements that path. It does not invent `poll=0` to reach it from
the live `func_8006AD40` prefix.

## TIM

PE.IMG `[180,197)` (`D_800930EC..EE`), dest
`lw(D_800B0CD8+0x174) = 0x8012F1A0` on Disc 1.

```text
flag      = 0x08                  # bit 3 = has CLUT
CLUT RECT = {320, 252, 16, 1}
IMAGE RECT= {320, 0, 64, 256}     # 256x256 4bpp
```

`func_800718D0` issues **image then CLUT** (literal jal order
`0x80071910` then `0x80071920`). TXT0-B listed CLUT then image as
file order, not call order.

## Record 0 pack

EXE sources `A=0x0140 B=0 C=0x0140 D=0x00FC`:

```text
tpage = ((A&0x3FF)>>6) | 0x20 | ((B&0x100)>>4) | ((B&0x200)<<2) = 0x0025
clut  = (D<<6) | ((C>>4)&0x3F)                                 = 0x3F14
```

`0x0025` = GetTPage(0, 1, 320, 0). `0x3F14` = GetClut(320, 252).
These equal the TIM dests. `func_80037870` reads them as
`lhu 0xC($fp)` / `lhu 0xE($fp)` with `$fp = D_80091644`.

Record 1 (`0x0026` / `0x3F15`) is packed in the same `a1<0x20`
loop and is not the 37870 pair.

## Live prefix

`func_8006AD40` still stops at `jal func_8006E7E8` (`0x8006AF54`).
Canonical channel-2 is busy. A `poll==0` assignment would be an
invention. Tests call `func_800718D0` and
`PE_func_8006AD40_PackFontRecords` directly.

## Next unresolved

After the packs, `0x8006B0AC` is `jal func_80030894` (`0xC50`
bytes). Not taken.
