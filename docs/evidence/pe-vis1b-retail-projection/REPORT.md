# PE-VIS1-B — retail actor projection contract

```text
PE-VIS1-B SUCCESS — RETAIL ACTOR PROJECTION CONTRACT PROVEN
```

Evidence only. No `native/` edit. No PT1 reopen. No push.

```text
authorities
  PE-VIS1-A   14c959b
  PE-UE0      45e6cd8
  CAM-B       1bf3832
  VIS-C       cad4598
  PE-RD7-R    authored Y is not gameplay Y
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
peimg_sha1  = 146c0ce7308bf9fdc2ba5a84230e198db0663f3b
m0002i_pkg  = 3705c782478453282efbbed0567a07fa5ff2f3aaeba6a12f204da9ed10af3359
m0003i_pkg  = bff4fbb28ee6fd49abdb812687ddb9a0aa5c8d44c3595dd4cc0a7720fc588a91
tool        = python3 tools/research/pe_vis1b_project.py --exe SLUS_006.62 --peimg PE.IMG
target_tree = native/   # UE0 worktree native/ at VIS1-A 14c959b
```

This supersedes fit-derived and bounds-derived sizing (CAM-A 68 px,
AABB room fit, VIS1-A unposed SY spans). Model and animation math
are not reopened.

## 1. Four layers

1. **Model bind / animation** — already correct (VIS-C). Not this rung.
2. **Actor world** — 16.16 `actor+0x28/+0x2C/+0x30`. Y is snapped.
3. **Field camera** — 52-byte view → PsyQ `MATRIX` (`func_80066800`).
4. **PS1 projection** — GTE `RTPS`/`RTPT`, `H`, `OFX/OFY` 16.16.

Walk matrix `D_800BD000` is not the view.

## 2. Y source = SNAPPED

RD7-R stands: authored Y is not gameplay Y. The handler is
`func_8001AA78` (classic group-height arm: `header+0x20 == 0`).

Retail EXE `0x8001AB60`: `sll v0, s1, 16` / `sw v0, 44(s4)` writes
`group.height << 16` over `actor+0x2C`. Inhibit bit `actor+0x98&0x80`
is clear on both accepted spawns (`0x1E` absent).

`func_80068B94` then `lh` the high halves, including `+0x2E` (snapped
integer Y), and issues `RTPS` `4A180001` at `0x80068CA0`.

| Room | `0x0B` authored Y | group height | projection Y |
|---|---:|---:|---:|
| m0002i `+0x00A0` | 0 | 0 | 0 |
| m0003i `+0x0144` | 0 | −4 | **−4** |

Keeping lobby authored 0 yields IR `(−16, 941, 1238)` instead of
`(−16, 939, 1235)`. That path is rejected.

## 3. World → camera

Re-read 52-byte records from the hashed packages. `MAC = (TR<<12)+R·v`,
`IR = MAC>>12`. No float.

```text
m0002i  world (−127, 0, −5377)  ->  cam (−145, 224, 2475)
m0003i  world (−272, −4, −176)  ->  cam (−16, 939, 1235)
```

Matrices: `WORLD_TO_CAMERA.md`. H = 307 / 251.

## 4. Screen / scale / height

```text
OFX=160 OFY=112   (ctc2 of arg<<16)
sx = 160 + H * IR1 / SZ3
sy = 112 + H * IR2 / SZ3
```

Exact rationals (host UNR stand-in):

```text
m0002i  sx=351485/2475  sy=345968/2475  scale=307/2475
        = 142.014141414141, 139.785050505050, 0.124040404040
        pan (0, 3)  ->  visible (142.014141414141, 142.785050505050)

m0003i  sx=193584/1235  sy=374009/1235  scale=251/1235
        = 156.748178137652, 302.841295546559, 0.203238866397
        pan (0, −144) -> visible (156.748178137652, 158.841295546559)
```

Retail GTE uses UNR; ±1 px vs this divide is the known residual.

**Pixel height is the posed idle mesh SY span**, not
`H/SZ * 229`, not a target, not native's unposed span:

```text
m0002i  26.09 px    VIS-C sy 102.09 .. 128.18
m0003i  29.71 px    VIS-C sy 154.20 .. 183.90
```

Lobby VIS-C world samples reprojected through this MATRIX match
VIS-C to four decimals (crown `SZ=859` → `151.5262, 171.9558`).

## 5. Background

Native 320×224, 1:1 tiles, pan/crop only. Retail does not scale the
background. `rec+0x24=4096` is not a height knob.

## 6. Native defects (not implemented here)

`native/src/pe_camera.cpp` `project_camera` and the submit/nclip
raster path store IEEE `double` SXY. Simulation authority is
fixed-point. That is `float_in_path=yes`. VIS1-A already ranked
it R1 (after R2 pose). This contract forbids leaving it there.

---

```text
base_vis1a_commit=14c959b
target_tree=native

y_source_consumed=SNAPPED
snap_handler=func_8001AA78

screen_origin=160,112
projection_scale_semantics=H/SZ3 (H=view[+0]; m0002i 307; m0003i 251)
fixed_point_format=actor 16.16 -> s16 int; R 12.4; TR s32; MAC>>12; H u16; OFX/OFY 16.16; SXY s16; SZ u16
float_in_path=yes

m0002i_screen_x=142.014141414141
m0002i_screen_y=139.785050505050
m0002i_actor_scale=307/2475
m0002i_pixel_height=26.09

m0003i_screen_x=156.748178137652
m0003i_screen_y=302.841295546559
m0003i_actor_scale=251/1235
m0003i_pixel_height=29.71

background_native_320x224=yes
background_scaling_required=no

projection_contract_ready=YES

hard_blockers=none
unknowns=GTE UNR vs host divide (+/-1 px); first-frame follow OFX/OFY vs raw 68B94 SXY on m0003i (raw sy=302.84 in 512-tall view; 677FC pan display_y=-144; 6DFA8 may apply D_800BCF94/96)
warnings=do not treat VIS1-A native unposed spans 24.7767/57.6353 as retail height; do not target 26/30/68 px; do not use authored lobby Y=0; do not treat 0x3C as FOV; rec+0x24=4096 is not copied by 66800

SUCCESS

PE-VIS1-B SUCCESS — RETAIL ACTOR PROJECTION CONTRACT PROVEN
```
