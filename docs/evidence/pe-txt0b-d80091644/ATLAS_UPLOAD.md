# Atlas upload

## Path

```text
D_800930EC = 180
D_800930EE = 197
func_8006AD40 channel-2 issue
  func_8006E6A8(lba_base+180, D_800B0CD8+0x174, 17)
    -> PE.IMG [180,197)  34816 bytes
    -> guest 0x8012F1A0 (canonical +0x174)

func_8006E7E8                 # 0x8006AF54  live B54D cut
  ; on success s2==0
func_800718D0(lw(+0x174))     # 0x8006AFA8
  LoadImage CLUT RECT
  LoadImage image RECT
func_8006AD40 pack a1=0..0x10 # 0x8006AFB4
  sh GetTPage -> D_80091650
  sh GetClut  -> D_80091652

func_80037870                 # glyph SPRT
  $fp = D_80091644
  lhu 0xC($fp) = tpage
  lhu 0xE($fp) = clut
```

`func_800718D0` is a 29-word TIM walker (flag bit 3 = has CLUT).
It is **not** implemented this rung. The TIM bytes are on the disc.

## TIM (PE.IMG [180,197))

```text
magic     = 0x10
flag      = 0x08
sha256    = 864088aed4b2bde72436404c6f6b6bd86302acba9d4e2671f07e7f5c16e27821
CLUT RECT = {320, 252, 16, 1}     16 colors, 4bpp
CLUT sha  = 52b1e9ebc16d0c0ad5280c1a9386dc2750277f6322c62516ddd3c2d2fb04bf86
IMAGE RECT= {320, 0, 64, 256}     256 x 256 px at 4bpp
IMAGE sha = 2156e25d353704ba31b568f7f19c3124492899f54fac09f79d6cd8b3c6d0e883
```

256 ≥ 252. 21 columns of 12 fit with 4 px leftover. Height 256
gives 21 glyph rows (441 cells).

## Packing identity

EXE record 0 sources: `A=0x0140 B=0 C=0x0140 D=0x00FC`.

```text
packed1 = ((A&0x3FF)>>6) | 0x20 | ((B&0x100)>>4) | ((B&0x200)<<2) = 0x0025
packed2 = (D<<6) | ((C>>4)&0x3F)                                 = 0x3F14
```

`0x0025` is GetTPage(0, 1, 320, 0) (4bpp, ABR bit forced).
`0x3F14` is GetClut(320, 252).

These equal the TIM dests. Not inferred from pixels.

B54B's 13-entry table is a different packet (sectors 157-180) and
does not contain this TIM.

## Frontier

Upload and record-0 packing sit **after** `0x8006AF54`. This
evidence does not implement them and does not move the cut.
