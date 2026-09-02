# PE-B54K-AU — title loop exit (New Game) and the exe frame-flip pair

Status: **TRANSLATED TO THE ORDERING-TABLE BOUNDARY**.

Authority: PE.IMG overlay (SHA-1 `146c0ce7…`), executable SHA-1
`452fb033…`.  Spans (authenticated by `b54kas_title_oracle.py`):

```text
exit  [0x80191410,0x80191548)  78 words 56760df8649a14491528bf9ee04633aac889fc90c9d3f9da18be0f53bae85cb4
exit  [0x80191724,0x801918F8) 117 words 4293951edcd9e30490754a1c5809fc6039c5dce353cfc33a1feb75b37d557adf
func_8005E6F0 [0x8005E6F0,0x8005E788) 38 words 6fe8f5fa0b1723c5221715fb352e6b0909517d8a7148a374c232cdb0611c925e
func_8005E788 [0x8005E788,0x8005E84C) 49 words aa9d6845c79995fe3d03e88d933243b87b87c1552209b08813016889a1016088
```

`PE_Overlay_TitleExit` (in `func_801909B4_port.c`) runs retail's exit:
display off, MoveImage `(0x2C0,0,0xA0,0x100)->(0x140,0)`, ClearImage
`(0,0,0x140,0x1E0)`, DrawSync, two VSyncs, `func_8005E588`,
`func_8005E6F0`, `func_8005E788(2)`.  With `D_801D1380 >= 1001` the kind-7
row selects: `0xA0` New Game (`func_80036E34`, `func_80036DF8`, `$s2 = 1`),
`0xB4` Continue without a card slot or any row `>= 0xB5` (`$s2 = 3`).
The card/load arm `[0x80191548,0x80191724)` is
`func_801909B4_80191548_load_arm_cut`; the attract restart
(`bltz $s2, 0x80190BCC`) is `func_801909B4_80190BCC_attract_restart_cut`.
On return the DrawEnv/DispEnv templates are restored from the overlay
copies, `D_800BCE9E`/`D_800BCE8A` take `$s0 = 8`, then `func_8005C1EC(0)`
— whose zero path is the pre-existing named boundary `func_80042798`
(event-record cleanup, B54K-Q) — `func_8005E57C(0)`, return `$s2`.  The
tests drive the exit with `D_8009D120 == 0` so the OT cuts are skipped and
prove the arms up to that cleanup boundary.

`func_8005E6F0` selects the executable's draw environment / ordering table
pair by index (`D_8009D108`, stride 120 from `D_800A2180/21F0/21F4`) and
publishes `D_8009D0FC/D100/D104/D118/D11C`; when `D_8009D120` is set it
clears the 0x1000-entry reverse OT through `func_800752AC` (ClearOTagR,
DMA6) — the named cut `func_8005E6F0_func_800752AC_cut`.  `func_8005E788`
is inert without an OT; with one it syncs, resets, puts the environment
pair, optionally LoadImages a 320x204 image from `D_8009D134`, and walks
the OT through `func_800753B4` (DrawOTag) — cut
`func_8005E788_func_800753B4_cut`.

Driven run (`PE_PORT_SKIP_FMV=1 --pad 8@560-563 --pad 4000@700-703`):
the title loop exits on the Cross confirm and stops at
`func_8005E6F0_func_800752AC_cut` (frontier entry `skip_fmv_new_game`).
The exe consumer of the return value, `func_8006E9A0(1)`, is still the
Phase 6D-S HOST_ADAPTED approximation (single-pass fade loop); its retail
loop `[0x8006EB2C,0x8006EB70)` polls `D_800BCFEE & 3` against `$s1` per
frame through `func_80070E54` and is the boundary after the OT clear.

```text
PRODUCTION_REACHABILITY(skip_fmv,no_input)=func_800425DC_from_func_801909B4
PRODUCTION_REACHABILITY(skip_fmv,Start+Cross)=func_8005E6F0_func_800752AC_cut
NEXT_RUNG=ClearOTagR_DMA6_provider_then_func_8006E9A0_retail_fade_loop
```
