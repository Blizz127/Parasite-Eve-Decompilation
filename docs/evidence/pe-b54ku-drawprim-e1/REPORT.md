# PE-B54K-U — DrawPrim wrapper and GP0(E1h) first packet

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates PsyQ `func_80075358` and the execution-proven command
word path through `func_80076B58`, adds generic GP0(E1h) draw-mode state to
the native GPU authority, and executes the first DrawPrim in
`func_80190660`. The next boundary is the immediately following four-word
textured SPRT packet.

No scene, overlay address, destination token, or scheduler state appears in
the GPU provider.

## Retail identity and indirect path

```text
executable SHA-1           452fb033f2eaa4b18aa20a5bca60b8125af3a37b

func_80075358              [0x80075358,0x800753B4)
size                       0x5C / 23 words
SHA-256                    b16699f3b147f2e86daf0cb43680613cfe2893f8aa5c7cbe06ccb34a74d3920e

func_80076B58              [0x80076B58,0x80076B98)
size                       0x40 / 16 words
SHA-256                    c34c4cc1323c3d3ff00222500fd91dee13e581d3ebd75141953149a7e943646e

D_80095744                 0x80095704
jtb[15] / +0x3C           0x80077294 (DrawSync drain)
jtb[5]  / +0x14           0x80076B58 (command-word worker)
```

The wrapper preserves retail order:

1. read `jtb[15]` and call DrawSync(0);
2. re-read `D_80095744` after DrawSync;
3. load packet length with `lbu packet[3]`;
4. call `jtb[5](packet+4, length)`.

Dirty slot identities remain typed indirect boundaries. They do not fall
through to a guessed worker.

`func_80076B58` always writes GP1 `0x04000000` (DMA direction off), including
for a zero-length packet. For nonzero length it writes exactly that many
32-bit words to GP0 in increasing address order. Unsupported GP1/GP0 state
stops at `func_80076B58_gp1_cut` or `func_80076B58_gp0_cut` after only the
retail-ordered effects already performed.

## Generic GP0(E1h) state

`PE_GPU_WriteGP0` now accepts GP0(E1h) only while GP0 is ready, the parser is
idle, and DMA2 is inactive. It stores the complete command word and increments
value-only telemetry while leaving the parser idle. A rejected E1 command is
mutation-free. This is generic hardware state: later primitive decoding can
read texture-page/depth fields from the same authority.

The first overlay command is independently derived as:

```text
SetDrawMode(a1=0,a2=0,a3=24)
0xE1000000 | (24 & 0x09FF) = 0xE1000018
worker writes: GP1 0x04000000, then GP0 0xE1000018
```

## Overlay advancement and next packet

The authenticated overlay prefix grows by the first DrawPrim instruction and
delay slot:

```text
represented range           [0x80190660,0x80190868)
size                        0x208 / 130 words
SHA-256                     7669d51cc1cd029bdc176627932e690b4f30d7846c4b233ed3a3c27e3bdca29a
```

The current boundary is:

```text
0x80190868  jal func_80075358
0x8019086C  move a0,s1
```

Its native value-only snapshot has length 4 and all 16 initialized command
bytes:

```text
64000000 00580020 78000000 00400100
```

This is GP0(64h), a variable-size textured rectangle at `{32,88}`, UV `{0,0}`,
CLUT `0x7800`, size `256x64`. The prior E1 word selects tpage `0x18`, so the
two B54K-T image records are structurally the CLUT at `{0,480}` and 4-bit
texture page at `{512,256}`. Rasterization is not claimed by this rung.

The transient packet adapter consumes words synchronously. It never stores a
native pointer in guest RAM or any persistent host authority. Diagnostic
payload capacity was widened from 8 to 16 bytes solely to preserve this
complete four-word boundary by value; all existing payload contracts remain
unchanged.

## Tests and oracle

Two focused tests prove:

1. generic E1 acceptance/replacement, idle-parser state, and mutation-free
   not-ready rejection;
2. exact DrawSync → GP1 DMA-off → GP0 command ordering, clean return for the
   retail jump-table identities, and a typed dirty-slot-5 negative control.

The two B54K-T and eight B54K-R contracts continue to pass while traversing
the new first-packet path.

```text
focused B54K-U:    975 run, 2 passed, 0 failed, 973 skipped
focused B54K-T:    975 run, 2 passed, 0 failed, 973 skipped
focused B54K-R:    975 run, 8 passed, 0 failed, 967 skipped
normal full suite: 975 run, 975 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  975 run, 975 passed, 0 failed, 0 skipped
sanitizer diagnostics: 0
```

`pc_port/tools/b54ku_drawprim_e1_oracle.py` imports no production code. It
authenticates both executable bodies and jump-table entries, the worker's
GP1-always/counted-GP0 shape, all 130 overlay words, the second DrawPrim
boundary, an independent E1 value/order model, source fences, focused tests,
and real-disc behavior.

```text
  OK retail: 23-word wrapper, 16-word worker, jtb[15]/jtb[5]
  OK overlay: first DrawPrim represented; four-word SPRT next
  OK model: DrawSync, GP1 DMA-off, one GP0(E1h) word
  OK source: generic E1 authority; exact indirect fences; no planting
  OK runtime: 2 focused contracts; second DrawPrim is next

B54K-U DrawPrim/E1 oracle: PASS.
```

## Production and disposition

The provider name is unchanged because two consecutive instructions call the
same wrapper, but the exact PC/payload has advanced from the E1 packet at
`0x80190860` to the SPRT packet at `0x80190868`.

```text
strict provider:            func_80075358 from func_80190660
normal framebuffer:         vsyncs=6 drawsyncs=5 presents=3 mask=1
normal stop:                unresolved-boundary
DMA checkpoint census:      27 calls / 26 active tokens
disc1.candidate SHA-1:      452fb033f2eaa4b18aa20a5bca60b8125af3a37b

FUNC_80075358=EXECUTION_PROVEN_PATH_TRANSLATED
FUNC_80076B58=COUNTED_COMMAND_WORD_PATH_TRANSLATED
GP0_E1=DRAW_MODE_STATE_IMPLEMENTED
FUNC_80190660_PREFIX=130_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80075358_sprite_at_80190868
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=implement_generic_GP0_64_textured_rectangle
```
