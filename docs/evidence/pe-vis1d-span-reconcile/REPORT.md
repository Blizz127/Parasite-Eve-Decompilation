# PE-VIS1-D — span measurement reconcile

```text
PE-VIS1-D SUCCESS — SPAN MEASUREMENTS RECONCILED
```

Evidence only. No `native/` edit. No H/SZ/Y/pan rewrite. No push.

```text
authorities
  PE-VIS1-A   14c959b     unposed census 24.7767 / 57.6353
  PE-VIS1-C   fb20034     posed native   26 / 29
  PE-VIS1-B   c72ea28     retail projection contract
  VIS-C       cad4598     posed float    26.09 / 29.71
  PE-RD7-R    authored Y is not gameplay Y
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
peimg_sha1  = 146c0ce7308bf9fdc2ba5a84230e198db0663f3b
tool        = python3 tools/research/pe_vis1d_span_reconcile.py --exe SLUS_006.62 --peimg PE.IMG
native_tree = pe-ue0-native-field-bootstrap @ fb20034 (pose) / d867f42 (integer stand-in used only as a later check)
```

Replay log: `MEASUREMENT_LOG.txt`. Row table: `SPAN_COMPARISON.csv`.
Cause write-up: `DIVERGENCE_CAUSE.md`.

## 1. What was compared

VIS1-A (`/tmp/pe-vis1a-measure`) projected **raw bind-local** verts
through production `world_to_camera` / `project_camera` / pan.
VIS1-C (`/tmp/pe-vis1c-measure` and `submit_player`) projected the
**idle-posed** mesh through the same camera helpers.

Independent replay of both vertex sets on one Disc 1, one MATRIX
pair, one actor origin:

```text
m0002i  unposed 24.776711   posed 26.092641   published 26
m0003i  unposed 57.635324   posed 29.707033   published 29
```

Census figures replay to the printed 4-decimal. Native 26 / 29 are
the posed float spans with per-vertex SY truncated (128−102=26,
183−154=29). A later integer-divide stand-in (`d867f42`) measures
m0002i as 25; that is truncation of the same posed path, not a
new camera. This card does not adopt 25.

## 2. Five checkpoints

| Check | Result |
|---|---|
| Same pose / vertex set | **no** — bind Y span 78 vs idle posed 229 |
| m0003i origin SZ | **same 1235** — native IR3 matches census |
| Entry Y | **SNAPPED −4** — not authored 0 |
| Pan | **identical** `(0,−144)`; span invariant to pan |
| H | **same 251 / 307**; not swapped across rooms |

Authored Y=0 on m0003i shifts origin to `(−16, 942, 1238)` and
moves spans by <0.5 px. Forcing H=307 on the lobby posed mesh
gives 36.33, not 57.63. Neither Y nor H is the 57-vs-29 lever.

## 3. Why the rooms now look similar

Origin `H/SZ` really does differ by ~1.64× (307/2475 vs 251/1235).
Retail pixel height is the **posed mesh SY span**, not that ratio
times 229. Lobby look-down folds standing height into SZ
(posed vertex SZ 846..1069 vs origin 1235), so posed SY is 29.71
next to sidewalk 26.09. VIS-C already published those posed
numbers. The census 57.64 is the unposed splat under the same
downward camera (VIS1-A R2).

## 4. Defect

None proven. Both published figures are correct for the vertex
set that was submitted. No production value is changed.

---

```text
m0002i_census_span=24.7767
m0002i_native_span=26
m0002i_agreement=yes

m0003i_census_span=57.6353
m0003i_native_span=29
m0003i_agreement=no

divergence_cause=unposed_bind_verts_vs_posed_idle_mesh
correct_figure=BOTH_CONTEXT_DEPENDENT

m0003i_y_source=SNAPPED
m0003i_origin_sz_native=1235
m0003i_h_native=251

defect_present=no
defect_rung=

hard_blockers=none
warnings=do not treat 57.6353 as posed/retail height; do not change H or origin SZ to force 57 toward 29; authored Y=0 is not the gap; later integer stand-in measures m0002i as 25 not the published 26

SUCCESS

PE-VIS1-D SUCCESS — SPAN MEASUREMENTS RECONCILED
```
