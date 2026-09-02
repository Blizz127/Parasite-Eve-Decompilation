# PE-B54K-AT — native title screen vs retail (PCSX-Redux) frame parity

Status: **FIRST INTERACTIVE SCREEN EXACT; POST-START MENU DIFFERS ONLY IN
THE MEMORY-CARD-DEPENDENT REGION (documented, not called done)**.

## Native artefacts

```text
PE_PORT_SKIP_FMV=1 parasite-eve-port --headless --disc-image <Disc 1.bin> \
    --max-frames 620 --vram-dump title620.vram
PE_PORT_SKIP_FMV=1 parasite-eve-port --headless --disc-image <Disc 1.bin> \
    --max-frames 760 --pad 8@560-563 --vram-dump menu760.vram
```

`--pad 8@560-563` delivers raw Start (active-low `0xFFF7`) into the retail
PadInitDirect buffer for VSync indices 560..563 through `platform/pe_pad.c`;
otherwise the buffer carries a connected, idle digital pad (`00 41 FF FF`)
and no controller in port 2.  Both dumps have display buffer 0 (VRAM rows
0..239) byte-identical to buffer 1 (rows 240..479).

```text
native title display buffer SHA-256 033ae52bee471e5337961770da03797403341aaab30f4ddea41cbc81142f61ab
native menu  display buffer SHA-256 5d914c7f42a802336fcfc63281ca5f45f46bec1e5588b5293bed5275f3faf7c9
```

The title digest equals the independent ByteModel of
`pc_port/tools/b54kas_title_oracle.py`.

## Retail reference

PCSX-Redux build293 (`~/apps/pcsx-redux`, AppImage extracted), headless:
`-no-ui -run -iso "<Disc 1.bin>" -dofile capture.lua`, OpenBIOS, interpreter.
The Lua listener on `GPU::Vsync` reads guest byte `D_800B0DBA` through
`PCSX.getMemPtr()` to detect the movie (value 2 from frame 1211), presses
Start through `PCSX.SIO0.slots[1].pads[1].setOverride` 20 frames later to
skip it (retail pad-skip path; `D_800B0DBA` returns to 0 at frame 1237),
and stores `PCSX.GPU.takeScreenShot()` (320x240, 24 bpp, 230400 bytes).
Redux hangs (0 % CPU) after the first 24-bit screenshot and segfaults during
uninterrupted movie playback, so each reference frame is one run:

```text
title        movie end + 200 frames             SHA-256 29484af9f436eaef261e18578cc93a846549df4c671ad9fbcd14efa021ee3bf6
menu (card)  Start at +120, shot at +300        SHA-256 60ba5d3c04811ac6248ac4836a7398125d61e15d2611e3f0a558494750a8a78d
```

Redux frames the CRT output: 16 blank lines at the top and the last 16
display rows cut.  `pc_port/tools/pe_title_frame_compare.py
--retail-row-offset 16` compares native row y with retail row y+16, requires
the 16 retail rows above to be black and the 16 native rows that fell off
the capture to be black, and demands exact equality elsewhere.

## Result 1 — first interactive screen (Press Start)

```text
pe_title_frame_compare.py title620.vram retail_title.raw --retail-row-offset 16 \
    --negative retail_menu.raw
diff: 0 pixels / 0 bytes of 76800 pixels (rows compared: 224)
negative control diff: 3189 pixels / 9006 bytes (must be > 0)
RESULT: EXACT | negative control REJECTS
```

Exact over every captured row; the negative control (the retail menu frame
against the native title) is rejected.

## Result 2 — Start press, next state (menu)

```text
pe_title_frame_compare.py menu760.vram retail_menu.raw --retail-row-offset 16 \
    --negative retail_title.raw
diff: 669 pixels / 1705 bytes of 76800 pixels (rows compared: 224)
   differing native rows 187..216, columns 128..187
negative control diff: 3138 pixels / 8810 bytes (must be > 0)
RESULT: MISMATCH | negative control REJECTS
```

The screen does change to the menu on both sides (kind-2 "Press Start"
fades out, the New Game / Tutorial items and highlight appear, background
and everything outside rows 187..216 identical).  The differing block is
the "Continue" entry (task kind 4, whose fade ramp `func_80193084` starts
only when `func_80042770` reports a card slot) and the highlight row
(`func_80190064`'s card-present branch).  In that Redux run the emulator had
inserted a freshly formatted memory card, while the native port's
memory-card poll `func_800425DC` is an explicit recorded boundary and the
slot records of `D_800A0ED4` stay as `func_80042538` initialised them (no
card).  This diff is therefore the memory-card boundary, not the title
translation; it is documented here and the menu state is not claimed exact.
A cardless retail rerun (scratch config via a HOME override, both cards
marked not inserted) was attempted; the emulator hung with the movie still
active, so no cardless menu reference exists.  The reference emulator
honours `~/.config/pcsx-redux/pcsx.json` unless HOME is overridden; the
user's configuration file was not modified (mtime 2026-09-01 17:21, before
any capture).

Heatmaps (local, not committed: they contain the rendered frame):
`scratchpad/heat_title.png` (all grey), `scratchpad/heat_menu.png` (669 red
pixels in the Continue/highlight block).

```text
TITLE_FRAME_PARITY=EXACT_224_ROWS_16_LINE_REDUX_FRAMING
MENU_FRAME_PARITY=MISMATCH_669px_rows187-216_memory_card_boundary
NEGATIVE_CONTROLS=REJECT
NEXT_BOUNDARY=func_800425DC_libcard_poll_for_menu_parity
```
