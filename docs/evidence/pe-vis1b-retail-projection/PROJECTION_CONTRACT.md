# VIS1-B projection contract — native/

Read-only contract for the next native rung.
This file does not authorize `native/` edits in this commit.

Target tree is UE0 `native/` (VIS1-A `14c959b` on
`feature/pe-ue0-native-field-bootstrap`). Model and animation math
stay on the VIS-C / RD2C contract. Do not reopen them.

## Authority

Retail world→screen supersedes every fit, AABB, bounds, or target
pixel height. VIS1-A measured native's *current* unposed SY span
(24.7767 / 57.6353). That is an R2 symptom, not the size to hit.

## Must do

1. Keep the decoded background at native **1:1** in 320×224.
   Tiles are already screen-space (`func_80066F60`).
2. Apply `func_800677FC` display pan only:
   - m0002i `(0, 3)`
   - m0003i `(0, -144)`
   Never scale the framebuffer.
3. Install view index **0** through `func_80066800` semantics.
4. `SetGeomScreen(H)`: m0002i `307`, m0003i `251`.
5. `SetGeomOffset(160, 112)` unless follow `D_800BCF94/96` is live.
6. World input = integer halves of live `actor+0x28/+0x2C/+0x30`
   **after** `func_8001AA78`.
7. Y source is **SNAPPED** (`group.height << 16` on these rooms).
   m0003i authored `0` becomes `−4`.
8. Then already-correct model/animation matrices, then this view
   `MATRIX`, then GTE `RTPS`/`RTPT` (or bit-identical integer UNR).
9. Actor scale is `H/SZ` per vertex. Pixel height is the posed
   mesh SY span, not `scale * model_height`.
10. Every stage stays fixed-point. See `FIXED_POINT_STAGES.csv`.

## Must not

- Scale the background by any factor.
- Target 26 px, 30 px, 45 px, 68 px, or any chosen height.
- Use world bounds, AABB, or collision extents for scale.
- Feed authored `0x0B` Y into RTPS when `1AA78` has run.
- Reopen skeletal decode or invent child bone T.
- Replace UNR with IEEE `double` and call it done.
- Treat `0x3C` as FOV or a 56-byte layer as the 3D view.
- Feed walk matrix `D_800BD000` into projection.

## Frozen inputs

```text
m0002i view   package +0x48DA0 + 0x1040   H=307
m0003i view   package +0x686AC + 0x298C   H=251
m0002i spawn  (-127, 0, -5377) yaw 0x0860
m0003i entry  (-272, -4, -176) yaw 0x0800
idle posed Y span  229 model units (VIS-C; not a screen target)
```

Derived IR / SXY / H/SZ / pixel height:
`WORLD_TO_CAMERA.md`, `PROJECTION_MATH.md`, `SAMPLE_PROJECTIONS.csv`.

## Acceptance

```text
background_native_320x224     = yes
background_scaling_required   = no
framebuffer_zoom              = none
y_source                      = SNAPPED via func_8001AA78
view_source                   = 52-byte record index 0 via 66800
projection                    = GTE RTPS/RTPT (or bit-identical integer)
screen_origin                 = (160, 112) unless follow OFX/OFY live
actor_scale                   = H/SZ3 per vertex
pixel_height                  = posed mesh SY span (result, not target)
fixed_point_path              = yes
float_in_projection           = no
gameplay_changed              = no
model_animation_reopened      = no
```

`projection_contract_ready=YES` is the audit gate. Implementation
is a later rung.
