# World → camera

Evidence-only. `native/` was not modified.

## Four transforms (do not collapse)

| Layer | What it is | Proven consumer | This rung |
|---|---|---|---|
| Model bind / animation | 23-node type-2 pose; `Ry*Rx*Rz`; `T = Rp*Tl + Tp`; `>>12` | VIS-C `cad4598` / RD1M-A / RD2C | **do not reopen** |
| Actor world | signed 16.16 at `actor+0x28/+0x2C/+0x30`; yaw `+0x3A` (`0x1000`/turn) | opcode `0x0B` then `func_8001AA78` | **Y source** |
| Field camera | PsyQ `MATRIX` from the 52-byte view record | `func_80066800` → `*D_800BCFA4` | this rung |
| PS1 projection | GTE `RTPS`/`RTPT` + `H` + `OFX/OFY` | `func_80068B94` / `func_80079244` / mesh `0x8003AEAC` | this rung |

`D_800BD000` / `func_80066CE8` is the walk heading matrix. It is not
the field view. Spawn heading is 0.

## Which Y the projection consumes

**SNAPPED.** Not authored.

Opcode `0x0B` subop 0 (`func_80012C20` case 0) stores the three 16.16
immediates, then `jal func_8001AA78`. The snap head is:

```text
lw    v0, 0x98(actor)     # flags
andi  v0, v0, 0x80        # inhibit
bnez  -> skip
lhu   fp, 0x2A(actor)     # integer X
lhu   s7, 0x32(actor)     # integer Z
lh    s1, 0(group)        # signed classic group height
jal   func_8001C614       # even-odd XZ
sll   v0, s1, 16          # height << 16
sw    v0, 0x2C(actor)     # overwrite Y
```

`header+0x20 == 0` on both rooms, so `0x468($gp)` is 0 and the
classic group-height arm is taken. The alt-28 plane equation is not
used. RD7-R is the same handler on m0377i (authored 0 → snapped 65).

Field-load `func_80068B94` then feeds GTE from the **live** integer
halves, after that store:

```text
lh  v0, 0x2A(actor)    # X
lh  v0, 0x2E(actor)    # Y  = high half of +0x2C
lh  v0, 0x32(actor)    # Z
```

Mesh `RTPT` uses the same live actor origin plus already-posed
model verts. Dropping Y, or keeping the `0x0B` immediate, is not
retail.

| Room | Authored `0x0B` Y | Group height | After `1AA78` | Projection input |
|---|---:|---:|---:|---:|
| m0002i spawn `+0x00A0` | `0x00000000` | 0 | `0x00000000` | 0 |
| m0003i entry `+0x0144` | `0x00000000` | −4 | `0xFFFC0000` | −4 |

m0002i authored equals snapped only because the group label is 0.
m0003i authored 0 is discarded. Both rooms still go through
`func_8001AA78`.

Using authored Y=0 on m0003i would shift IR by
`(R12,R22,R32)*(-4)` → `IR ≈ (−16, 941, 1238)` instead of
`(−16, 939, 1235)`. The projection path does not do that.

## Which function converts world → camera

`func_80066800` installs the view. The multiply is GTE `RTPS`
(`0x4A180001` at `0x80068CA0` and `func_80079244`) or `MVMVA`
(`func_800792D4`) when only camera-space is wanted.

```text
MAC1 = (TRX << 12) + R11*vx + R12*vy + R13*vz
MAC2 = (TRY << 12) + R21*vx + R22*vy + R23*vz
MAC3 = (TRZ << 12) + R31*vx + R32*vy + R33*vz
IR   = MAC >> 12          # sf=1
```

`vx,vy,vz` are signed 16-bit **integer** field units (`pos >> 16`).
`R` is 12.4 (`s16`). `TR` is `s32`. `MAC` is the 64-bit GTE
accumulator. No IEEE float.

## Installed first-view matrices

Both rooms have one 52-byte record. Field-load uses index 0
(`D_800BCFFD = 0` from `func_80065B70`). Re-read from PE.IMG
packages (hashes in `REPORT.md`):

### m0002i (`package +0x48DA0 + 0x1040` = file `0x49DE0`)

```text
H = 307
R = [ 4096,   10,     0 ]
    [   -7, 2932, -2861 ]
    [   -7, 2861,  2932 ]
TR = (-18, -3531, 6324)
viewport = 320 x 219
clamp    = (160,160, 107,112)
```

### m0003i (`package +0x686AC + 0x298C` = file `0x6B038`)

```text
H = 251
R = [ 4095,    5,   101 ]
    [   80, 2308, -3383 ]
    [  -60, 3384,  2307 ]
TR = (261, 802, 1334)
viewport = 320 x 512
clamp    = (160,160, 112,400)
```

`func_80066800` copies `H` (`lhu rec+0` → `*D_800BCFA8` +
`func_80079024`), nine `lhu` rotation halves from `rec+2`, and
`lw` TR from `rec+0x14/+0x18/+0x1C`. `rec+0x24 = 4096` is not
copied.

## Camera-space origin (integer `MAC>>12`)

| Scene | world int (snapped Y) | IR1 | IR2 | IR3/SZ |
|---|---|---:|---:|---:|
| m0002i spawn | (−127, 0, −5377) | −145 | 224 | 2475 |
| m0003i entry | (−272, −4, −176) | −16 | 939 | 1235 |

Values sit inside `s16`/`u16`. Hardware saturation was not required.

## What is not the 3D camera

- Header `+0x10 = 0x3C` is the 16-byte aux table. **Not FOV.**
- Header `+0x14` is the 56-byte tile-layer base. **Not FOV.**
- `func_800655D4` ORs bit 1 on **tile layers** 1 and 7 in the lobby.
- Collision AABB, world bounds, and a 68 px height target are not
  consumed by `66800` / `RTPS`.
- Native `submit_player` yaw-only bind verts are an assembly defect
  (VIS1-A R2). They are not this camera.
