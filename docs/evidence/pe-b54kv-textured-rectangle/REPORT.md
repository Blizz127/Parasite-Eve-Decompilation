# PE-B54K-V — GP0(64h) textured rectangle and PutDrawEnv frontier

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung implements the smallest generic GPU mechanism required by the
second DrawPrim in overlay-local `func_80190660`: an opaque, modulated,
variable-size 4bpp textured rectangle. It then translates the retail
continuation through the explicit DrawSync, canonical optional-upload bypass,
VSync, and ResetGraph(1). The next unresolved provider is PutDrawEnv
`func_80075424` at `0x8019093C`.

No overlay address, package name, destination token, story bit, or scene state
appears in the GPU provider.

## Authenticated overlay range

```text
PE.IMG SHA-1                 146c0ce7308bf9fdc2ba5a84230e198db0663f3b
overlay sectors             [0x03D2,0x0457)
overlay load                0x8018EFF0
function                    [0x80190660,0x801909B4) / 213 words
represented prefix          [0x80190660,0x8019093C)
represented size            0x2DC / 183 words
prefix SHA-256              820ca0d865218c5954ff6b15ba2a14af0d35aa43962e8331d60faa0f6b43cde0
```

The newly represented direct-call sequence is retail-authenticated:

```text
0x80190860  jal 0x80075358   first DrawPrim / E1
0x80190868  jal 0x80075358   second DrawPrim / SPRT
0x80190880  jal 0x80074DC0   explicit DrawSync
0x8019091C  jal 0x8007506C   conditional LoadImage
0x80190924  jal 0x80073A44   VSync(0)
0x8019092C  jal 0x80074A44   ResetGraph(1)
```

The canonical environment has signed halfword `environment+0x74 == 0`, so
the retail `blez` at `0x8019089C` branches directly to VSync. The positive
path remains state-driven in native: it reconstructs the unaligned RECT,
applies both retail 3/2 scalings and the parity Y adjustment, and calls the
existing generic LoadImage path. It is not forced for the canonical run.

The next two retail words are:

```text
0x8019093C  0x0C01D509  jal func_80075424
0x80190940  0x26520001   addiu s2,s2,1
```

## GP0(64h) packet and source geometry

The complete packet is:

```text
64000000 00580020 78000000 00400100
```

Independent decoding gives:

```text
opcode/color                0x64 / RGB 0,0,0 (modulated texture)
destination                 x=32, y=88
source UV                   u=0, v=0
CLUT code                   0x7800 -> x=0, y=480
size                        width=256, height=64
active E1                   0xE1000018
tpage                       x=512, y=256, depth=4bpp
```

Those addresses are exactly the two B54K-T uploads. The palette is:

```text
8000 0842 1084 18C6 2108 294A 318C 4210
4631 4E73 56B5 5EF7 6739 6F7B 77BD 7FFF
```

Expanding the 4096 texture halfwords to 16,384 nibbles produces SHA-256
`caa27d386763de7d6d33212e380bfe6505bbd1d4ae80dda90fcfca69c0d4196a`.
Every referenced palette value is nonzero. At frame zero, therefore, every
texel is opaque and modulation by RGB zero writes black across the 256x64
rectangle; this follows from retail data rather than a host clear shortcut.

Hardware behavior was cross-checked against the command/CLUT descriptions in
[psx-spx](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/) and the
independent scalar path in
[DuckStation's software rasterizer](https://github.com/stenzek/duckstation/blob/master/src/core/gpu_sw_rasterizer.inl).
The committed oracle imports neither implementation.

## Generic native contract

`PE_GPU_WriteGP0` now collects the four GP0(64h) words through explicit parser
states. On completion it:

- sign-extends 11-bit destination coordinates and clips to 1024x512 VRAM;
- wraps U/V at eight bits and applies E1 rectangle X/Y flip bits;
- fetches four 4bpp indices per texture halfword from the active tpage;
- resolves the 16-entry CLUT from the packet's CLUT attribute;
- preserves the destination when the resolved texture color is zero;
- applies `min(31, texture5 * vertex8 >> 7)` per channel without dithering;
- preserves CLUT bit 15 and records value-only completion telemetry.

The accepted subset is deliberately narrow: opcode 64h only, opaque,
modulated, variable-size, 4bpp, one-MiB texture page. An 8/15bpp or second-MiB
mode is rejected before entering the parser. Raw and semi-transparent opcodes
remain unsupported. Drawing-area/offset, texture-window, and mask registers
are not guessed; PutDrawEnv is the next retail owner that can establish those
states.

The second packet traverses the exact B54K-U DrawPrim wrapper and counted GP0
worker. It does not bypass DrawSync, GP1 DMA-off, or the jump-table identity
fences.

## Tests and independent oracle

Two new focused contracts prove:

1. four-nibble fetch order, a zero-color transparent texel, neutral RGB 128
   modulation, CLUT bit-15 preservation, parser completion, and telemetry;
2. U=252 wrap across a tpage row, RGB 64 modulation, signed-X clipping with
   continued UV progression, and mutation-free 8bpp/raw rejection.

Retained B54K-T, B54K-U, and B54K-R focused contracts all pass after the
frontier advance.

```text
focused B54K-V:    977 run, 2 passed, 0 failed, 975 skipped
focused B54K-U:    977 run, 2 passed, 0 failed, 975 skipped
focused B54K-T:    977 run, 2 passed, 0 failed, 975 skipped
focused B54K-R:    977 run, 8 passed, 0 failed, 969 skipped
normal full suite: 977 run, 977 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  977 run, 977 passed, 0 failed, 0 skipped
sanitizer diagnostics: 0
```

`pc_port/tools/b54kv_textured_rectangle_oracle.py` independently authenticates
all 183 overlay words, the next call/delay pair, call order, packet geometry,
palette and expanded-index identity, modulation examples, source fences,
focused tests, and real-disc behavior.

```text
  OK overlay: 183 words through ResetGraph; PutDrawEnv next
  OK model: 4bpp/CLUT decode, UV extent, transparency, modulation
  OK source: generic GPU state; exact continuation; no planting
  OK runtime: 2 focused contracts; PutDrawEnv frontier observed

B54K-V textured rectangle oracle: PASS.
```

## Production disposition

```text
strict provider:            func_80075424 from func_80190660
normal framebuffer:         vsyncs=7 drawsyncs=7 presents=3 mask=1
normal stop:                unresolved-boundary
DMA checkpoint census:      27 calls / 26 active tokens
disc1.candidate SHA-1:      452fb033f2eaa4b18aa20a5bca60b8125af3a37b

GP0_64=OPAQUE_MODULATED_4BPP_VARIABLE_RECTANGLE_IMPLEMENTED
FUNC_80190660_PREFIX=183_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80075424_from_func_80190660
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=translate_PutDrawEnv_execution_path
```
