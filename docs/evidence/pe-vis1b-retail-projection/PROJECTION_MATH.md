# Projection math

Evidence-only. `native/` was not modified.

## Which function projects to screen?

Retail is **GTE `RTPS` / `RTPT`**. A host divide is a derived check
only. IEEE `double` in `native/src/pe_camera.cpp` `project_camera`
is a defect (VIS1-A R1), not the contract.

| Site | Address | Word / role |
|---|---|---|
| `func_80079004` | `0x80079004` | `sll a0/a1,16` then `ctc2 OFX/OFY` (rd 24/25) |
| `func_80079024` | `0x80079024` | `ctc2 a0, H` (rd 26) — `48C4D000` |
| `func_80068B94` | `0x80068CA0` | field-load `RTPS` `4A180001` of player integer XYZ |
| `func_80079244` | `0x80079250` | wrapper `RTPS`; returns `SZ3>>2` |
| `func_8006DFA8` | `0x8006DFA8` | actor-origin: follow OFX/OFY, MATRIX, H, `79244` |
| mesh | `0x8003AEAC` | `RTPT` `4A280030` through the same MATRIX |

Boot `func_8003E610` sets `OFX/OFY = 160/112` and `H=240`. Field
view overwrites `H` from the 52-byte record.

## Screen origin

```text
OFX = 160 << 16     # 16.16; 320/2
OFY = 112 << 16     # 16.16; 224/2
```

`func_80079004` is literally `sll` 16 then `ctc2`. Follow path
`func_800661A4` may substitute `D_800BCF94/96`. `func_800661CC`
restores 160/112 after actor-origin draw. Tile clip stays 320×224.

## Perspective

GTE:

```text
sx_16_16 = OFX + H * IR1 * UNR(SZ3)
sy_16_16 = OFY + H * IR2 * UNR(SZ3)
SXY      = sx_16_16 >> 16     # s16 pixel, saturated
SZ3      = IR3                # u16 depth
```

Derived exact-rational stand-in (same inputs, no UNR):

```text
sx = 160 + H * IR1 / SZ3
sy = 112 + H * IR2 / SZ3
scale = H / SZ3
```

`H` is view `+0x00`: m0002i **307**, m0003i **251**. Actor size is
this ratio **per vertex** after bone `MVMVA` + view `MATRIX`. There
is no 68 px term, no AABB term, no `rec+0x24`.

Hardware UNR vs this divide is typically ±1 px (CAM-B). The native
sim is fixed-point; replacing UNR with `double` is still a defect.

## Origin samples (exact rationals)

| Scene | cam (IR1,IR2,IR3) | H | sx | sy | H/SZ |
|---|---|---:|---:|---:|---|
| m0002i (−127,0,−5377) | (−145, 224, 2475) | 307 | 351485/2475 = 142.014141414141 | 345968/2475 = 139.785050505050 | 307/2475 |
| m0003i (−272,−4,−176) | (−16, 939, 1235) | 251 | 193584/1235 = 156.748178137652 | 374009/1235 = 302.841295546559 | 251/1235 |

Truncated GTE-style SXY: m0002i `(142, 139)`, m0003i `(156, 302)`.

## Pan is not scale

`func_800677FC` writes container `+0x38/+0x3A` from the 52-byte
clamp midpoint (X uses `sra` after sign fold; Y uses `srl` after
sign fold; both rooms are non-negative so `>>1`):

```text
display_x = container[+0x2C] - (mid_x - 160)
display_y = container[+0x2E] - (mid_y - 112)

m0002i: (0,  3)
m0003i: (0,-144)
```

Tiles (`func_80066F60`) and native actor submit add this 2D offset
after projection. The lobby authored view is 512 tall; raw `sy=303`
is expected. Visible origin after pan:

```text
m0002i  (142.014141414141, 142.785050505050)
m0003i  (156.748178137652, 158.841295546559)
```

Nearest-neighbour scale of the 320×224 framebuffer is **not**
retail.

## Actor pixel height

Origin `H/SZ` is the depth scale, not the silhouette.

`scale * 229` (VIS-C idle posed Y span) is **not** the screen
height: m0002i would be 28.41, m0003i 46.54. Perspective varies
across the figure (lobby crown `SZ=859`, foot `SZ=1059`, ratio
1.23). Height is the projected mesh `SY` span.

VIS-C posed idle, same MATRIX + this divide + pan, full 468 verts:

| Room | sy_min | sy_max | pixel height |
|---|---:|---:|---:|
| m0002i idle | 102.09 | 128.18 | **26.09** |
| m0003i idle | 154.20 | 183.90 | **29.71** |

Independent check: VIS-C lobby world samples reprojected through
the PE.IMG MATRIX in this rung match VIS-C to four decimals
(crown `SZ=859` → `(151.5262, 171.9558)`).

Native's current bind-pose SY spans (VIS1-A 24.7767 / 57.6353) are
the unposed R2 mesh through this same camera. They are **not** the
retail actor height. Do not target 26 or 30. Project the posed
verts.

## Native float census (defect)

| Site | Type | Allowed? |
|---|---|---|
| `world_to_camera` MAC `s64 >> 12` | integer | yes |
| `project_camera` `double(H)*double(xy)/double(sz)` | IEEE divide | **no** |
| `nclip_mac0` / `nclip_keep` | `double` | **no** |
| `submit_player` `vector<double> sx/sy` | IEEE SXY | **no** |
| `fill_tri` barycentric `double` | raster, not GTE | defect vs fixed-point sim |

`runtime_present` `double render_dt` is the display pump, not the
projection contract.
