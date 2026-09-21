# Rendering architecture (verified from translated code, not assumed)

**Verdict: prerendered background plates plus real-time characters.**
This is the Resident Evil / Final Fantasy VII lineage, confirmed from the
port's actual GPU/TIM/OT path, not from marketing copy.

## What the code does

1. **Backgrounds are TIM images uploaded into VRAM, then presented.**
   `func_800718D0` (`pc_port/game/boot/func_800718D0_port.c`) is a TIM
   walker: it reads the TIM flag word, optionally skips a CLUT chunk, and
   calls `func_8007506C` (LoadImage) twice — image RECT+pixels first, CLUT
   second. Boot streamer `func_8006AD40` feeds it buffers loaded from disc
   (`D_800B0CD8+0x174` / `+0x180`). There is no geometry rebuild of the
   museum / NY streets; the plate is a 2D framebuffer blit.

2. **Draw order is an ordering table, not a depth buffer.**
   The PS1 GPU has no Z buffer. `pc_port/docs/drawotag_decision.md`
   and `pc_port/docs/b53a_func_80076C34_gpu_contract.md` record the seam:
   ClearOTagR fills a guest-RAM OT, AddPrim chains primitives, DrawOTag /
   DrawOTagEnv hand that table to DMA2 (`func_80076C34`). Sort key is OT
   position (painter's algorithm), not per-pixel depth.

3. **Characters and FX are real-time GTE/GPU primitives** sitting in the
   same OT as the plate (and as UI). The GTE port (`pc_port/platform/pe_gte.c`)
   is the transform path for those models; it does not rasterize the
   background.

4. **HD-2D implication.** Replacing/upscaling the TIM plates plus
   higher-fidelity character models is the win. Re-rendering whole scenes
   is the wrong strategy. Any background depth or mask the game already
   ships (separate TIM, OT slot, or CLUT key) is valuable because the
   original hardware never stored a depth buffer the host can upscale.
   Widen `pc_port/docs/b53a_func_80076C34_gpu_contract.md` when a plate's
   companion mask/depth resource is identified; do not invent one.

## What this is not

- Not a full 3D scene graph of the field.
- Not a software renderer of the background mesh.
- Not proven to ship a dedicated per-pixel depth texture for every room;
  that remains a search, not a claim.
