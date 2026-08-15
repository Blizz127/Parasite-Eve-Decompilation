# Atlas reachability on the live prefix

B54C translated `func_800718D0` and the pack stores but could not
reach them from `func_8006AD40` without inventing `poll=0`. B54E
consumed the AF54 wait. This rung takes the success continuation.

```text
AF54 wait  func_8006E7E8 == 0          # B54E, live
AF68       s0 = 0
AF88       issue D_800930EE / dest+0x180
AFA8       func_800718D0(lw(dest+0x174))
AFF8/B02C  pack records 0 and 1
B04C       second poll — NOT consumed
```

`718D0` argument is **dest+0x174**, the channel-2 TIM
PE.IMG `[180,197)` at canonical `0x8012F1A0`. Image then CLUT:

```text
IMAGE RECT = {320, 0, 64, 256}
CLUT RECT  = {320, 252, 16, 1}
record 0   = 0x0025 / 0x3F14
```

Focused `test_B54F_6AD40_live_atlas_and_record_packs` seeds that
TIM at `+0x174` and observes those two LoadImages plus the packs
from a single `func_8006AD40()` call.

```text
atlas_uploaded_on_live_path=yes
func_800718D0_reached=yes
```

The new `D_800930EE` issue is a different 3-sector TIM into
`dest+0x180`. It is not walked here. Host `D_8009B6B4=0` after
that issue is B54E-HOST-POLL-COLLAPSE, not retail completion.
Busy bits `0x01004000` stay set because `B04C` is not consumed.

## Framebuffer digest coverage boundary

The B49/real-disc SHA-256 hashes `HostFB`, a 320×240 host RGB
buffer (`PE_PORT_FB_WIDTH`). B53B already proves that buffer is
**not** aliased to PSX VRAM (`test_B53B_authority_separation_guard`).
So the digest is blind to every `LoadImage`, on- or off-display.

Separately, the atlas RECT starts at VRAM **x=320**. A 320-wide
retail display starting at x=0 would not show it either. The
unchanged digest is therefore expected. It cannot confirm the
atlas landed and cannot detect a regression that stopped the
issue.

Off-display uploads need their own assertion. That gate is
`test_B54F_6AD40_live_atlas_and_record_packs`: HostFB bytes stay
identical across the call, while the two atlas LoadImages
(`{320,0,64,256}` then `{320,252,16,1}`) and record-0 packs
must still occur. VRAM pixels are not that gate — this prefix
adds no DMA checkpoint, so image DMA need not have completed.
