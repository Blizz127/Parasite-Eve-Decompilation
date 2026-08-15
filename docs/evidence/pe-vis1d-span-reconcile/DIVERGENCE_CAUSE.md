# Why the SY spans diverge

Evidence only. No value was edited to force agreement.

## Published pair

| Room | VIS1-A census (unposed, float) | VIS1-C native (posed) |
|---|---:|---:|
| m0002i | 24.7767 | 26 |
| m0003i | 57.6353 | 29 |

m0002i is the same ~25 px sidewalk silhouette. m0003i is not: 57.6 vs 29,
and the two rooms now look similar in height even though origin SZ is
2475 vs 1235.

## Same pose / vertex set?

**No.**

| | VIS1-A `/tmp/pe-vis1a-measure` | VIS1-C `/tmp/pe-vis1c-measure` + `submit_player` |
|---|---|---|
| Verts | 468 raw type-2 bind-local | same 468 indices after clip `0x15` frame 0 |
| Model Y | `[-42, 36]` span **78** | `[-447, -218]` span **229** |
| Compose | actor yaw + integer XYZ only | hierarchy + root T + clip R, then yaw + XYZ |

Replay through the same `world_to_camera` / `H*IR/SZ` / pan adder:

```text
m0002i unposed  24.776711     posed  26.092641
m0003i unposed  57.635324     posed  29.707033
```

Swapping only the vertex set recovers both published numbers. Nothing
else needs to move.

## Same origin SZ?

**Yes, for the actor origin.** Native m0003i consumes census SZ **1235**.

```text
world (-272, -4, -176) -> IR (-16, 939, 1235)
```

What changes is **vertex** SZ, not the origin:

| Vertex set | m0003i SZ range | vs origin 1235 |
|---|---|---|
| unposed (census) | 1188 .. 1343 | clustered at the origin |
| posed idle (native) | 846 .. 1069 | figure extends toward camera |

VIS-C `cad4598` already recorded lobby crown SZ 859 / foot SZ 1059.
Those are posed-mesh depths. They are not a second camera.

## Snapped Y or authored Y?

**SNAPPED.** Native m0003i uses gameplay Y = −4.

```text
kLobbyY        = 0xFFFC0000     # -4 << 16
group_height   = -4             # classic collision group
pos_y live     = 0xFFFC0000
integer_y()    = -4
```

`0x0B` authored Y is 0. Feeding 0 (RD7-R negative) yields origin
`(−16, 942, 1238)` and spans 57.4345 (unposed) / 29.3179 (posed).
That is a 0.2 / 0.4 px shift. It does **not** turn 57 into 29.
Census and native both already used −4 (`integer_y()` after
`apply_lobby_actor`).

## Same pan?

**Yes.** Both paths add `view.pan` after projection.

```text
m0002i pan (0,  3)
m0003i pan (0,-144)
```

Pan is a uniform 2D offset. Span with pan off equals span with pan on
to all recorded decimals (24.776711 / 26.092641 / 57.635324 / 29.707033).
Lobby raw SY without pan sits near 300 in the authored 512-tall view;
that is expected and is not a scale term.

## Different H?

**No.** Both measurement sites use the 52-byte record:

```text
m0002i H = 307
m0003i H = 251
```

Forcing lobby H to 307 on the same snapped pose gives posed 36.33 /
unposed 70.49. H is not the 57-vs-29 lever.

## Why m0002i agrees and m0003i does not

Sidewalk camera is closer to horizontal (`R22=2932`, `R23=−2861`,
origin IR2=224). Bind splat and standing idle have the **same**
camera-Y span (190). Pose only pulls the mesh a little nearer
(SZ 2415..2601 → 2136..2341), so 24.78 → 26.09.

Lobby camera looks down (`R22=2308`, `R23=−3383`, origin IR2=939).
Unposed parts sit around the actor origin, so the downward look
sees the bind splat as a large SY blob (cam-Y span 214, SZ≈origin,
SY span 57.64). Idle pose stands the figure up the look-down axis:
model Y span grows 78 → 229, but camera-Y span **shrinks** 214 → 169
and vertex SZ drops to 846..1069. Height goes into depth. SY span
falls to 29.71.

That is why the two rooms now render at similar heights despite
origin SZ differing by ~2×. Origin `H/SZ` is 307/2475 vs 251/1235
(1.64×). Posed SY span is 26.09 vs 29.71. VIS-C independently
measured those posed numbers (`SCREEN_PROPORTIONS.csv`, frozen
`cad4598`). `H/SZ * 229` (28.41 / 46.54) is still the wrong
estimator; perspective varies across the mesh.

## Which figure is correct

Both, in their own context.

- **24.7767 / 57.6353** are the correct float SY spans of the
  **unposed** bind mesh through the production GTE stand-in.
  That was VIS1-A R2, not retail height.
- **26 / 29** are the VIS1-C published integer spans of the
  **posed** idle mesh (float 26.09 / 29.71; `trunc(sy_max) −
  trunc(sy_min)` on the fb20034 double path is 26 / 29).
  Those match VIS-C and are the gameplay silhouette.

Do not average them. Do not change H, origin SZ, pan, or Y to
make 57 meet 29.

## Not a defect

No measurement site is wrong. No live H/SZ/Y/pan mismatch was
found. The lobby drop 57 → 29 is the R2 assembly repair already
landed at `fb20034`. This rung does not reopen projection, pose,
or any production value.
