# PROJECT GOAL AND PLAN — read this before planning any work

_Set 2026-09-17. This file records what the work is **aiming at**;
`docs/project_plan.md` holds the phased engineering roadmap and
`docs/ai_context/ACTIVE_HANDOFF.md` holds current working state. Where this file
and the roadmap disagree about **priority**, this one wins; where they disagree
about **procedure**, the roadmap and `docs/build_authority.md` win._

---

## The goal

**A native PC port of Parasite Eve in the mould of Ship of Harkinian (Zelda:
OoT) and the Silent Hill decomp/port projects — running natively, moddable, and
ultimately re-rendered with new graphics (HD-2D is the stated ambition).**

The decompilation is the means. The port is the deliverable, and its needs set
the priority order.

## What that means for prioritisation

The single most useful question when choosing work:

> **Does this put C behind a code path the port executes and will need to
> restyle?**

Recovered *assembly* satisfies byte parity and is worth **nothing** to a native
port — you cannot execute PS1 MIPS on x86. Any coverage metric that does not
separate "C" from "assembly" will overstate how close a playable native build
is. Report both.

This project is in good shape on that front: 566 C files under `src/`, no
`INCLUDE_ASM` shims, and a real host platform layer under `pc_port/platform/`
(`host_window`, `host_vram`, `host_framebuffer`, `host_frame_pacer`). The
current work — live Disc 1 runs chasing a CD-driver catch-up hang — is exactly
the right *kind* of work: driving real retail code on the host.

## Architecture: where this project stands against Ship of Harkinian

| Ship of Harkinian | This project | Notes |
|---|---|---|
| `libultraship` — SDL/GL/AL platform layer | `pc_port/platform/` + `pc_port/bootstrap/` | exists and runs |
| **Fast3D** — translates N64 display lists to OpenGL | PS1 GPU/DMA path (`pc_port/docs/b53b_gpu_dma2_platform.md`, `b53a_func_80076C34_gpu_contract.md`) | a documented GPU contract already exists — that is the seam Phase B widens |
| OTR assets — enables HD texture packs | `assets/`, `rom/` extraction + the CD device layer | assets stay out of git (`docs/legal.md`) |
| zeldaret/oot decomp — **100% C** | 566 C files; measure the executed-path share | ← the gap |
| *(nothing — never needed one)* | **no MIPS interpreter — good.** Keep it that way. | see below |

**Two structural facts specific to Parasite Eve that the plan must respect:**

1. **It is a two-disc game** — `SLUS_006.62` (disc 1) and `SLUS_006.68`
   (disc 2). A port is not "done" on disc 1. Disc-2 divergence needs to be a
   tracked deliverable, not a surprise; anything disc-specific in the CD driver
   and asset paths should be built to handle both from the start.
2. **Prerendered backgrounds with real-time characters** (the Resident Evil /
   Final Fantasy VII lineage) — *verify this against the actual rendering code
   before acting on it*, because if it holds, the HD strategy is completely
   different from a fully-3D game: the win comes from replacing/upscaling the
   prerendered background plates and raising character model and texture
   fidelity, not from re-rendering whole scenes. That also means the background
   depth/occlusion data the game already ships (if any) is far more valuable
   than it looks — see the depth note below.

**A note on a shortcut to avoid.** A sibling project (Xenogears) runs a MIPS
interpreter over its un-decompiled battle overlay so the game boots today. It
works, but interpreted code is a black box that **can never be re-rendered or
restyled**. If an interpreter or static recompiler is ever proposed here, treat
it as a debugging aid with an expiry date, never as a way to call a system
"ported".

## Plan

### Phase A — C coverage of executed paths (the real unlock)

1. **Finish the live-boot path.** The current CD-driver work (`DAY2-158*`,
   `dig/b0cd0-catchup-miss`) is the blocker to everything downstream — a port
   that cannot stream from the disc cannot run anything. Keep going.
2. **Get to first real gameplay on disc 1**, then hold that as a regression
   baseline the way the handoff already pins live runs to commits.
3. **Rendering and input paths next**, because those are what Phase B restyles.
4. **Then disc 2.**

Gate: stub count and any non-C bodies on the executed path trending to zero.

### Phase B — renderer abstraction (where HD-2D actually lives)

`pc_port/docs/b53a_func_80076C34_gpu_contract.md` means a GPU contract is
already written down — that document is the seam. Two things to plan for early:

- **Geometry precision.** PS1 rasterisation snaps vertices to integers. A naive
  translator inherits that wobble; PGXP-style recovered subpixel precision is
  what makes an HD render look intentional rather than upscaled. Decide it at
  the GPU-backend boundary, not after.
- **There is no depth buffer.** HD-2D post-processing (depth of field,
  tilt-shift, bloom) needs depth; the PS1 sorts with an ordering table, which
  gives draw *order*, not depth. Depth must be synthesised from the OT index or
  recovered from GTE transform output — **unless** the prerendered-background
  hypothesis above holds and the game ships background depth/mask data, in
  which case much of the problem is already solved for you. Establish which
  world you are in early; this is the largest technical unknown in the HD-2D
  goal.

### Phase C — asset replacement, then art

Higher-resolution background plates, textures and models through the asset
layer (the OTR equivalent), honouring `docs/legal.md` — no game data in git,
ever. Only meaningful once A and B land.

## When to byte-match

Keep `docs/build_authority.md` and the existing verification discipline; the
evidence-and-hashes habit in this repo is a real asset. But weigh cost: byte
parity is typically far more expensive than correctness. On the sibling
Xenogears project, five byte-exact functions in one session took minutes to
*understand* and hours to match register allocation — roughly 90% of the effort
went into byte parity alone.

- **Do** byte-match the low-level engine: CD driver, DMA, GPU, GTE, memory.
  Subtly-wrong code there produces the exact silent hangs this project has been
  chasing, and byte parity turns a multi-day live-run bisect into a compile-time
  check.
- **Do not** hold up gameplay or UI logic for byte parity; correct C plus a
  differential test is cheaper and nearly as strong. Record it honestly as
  "correct, not matched" rather than claiming a match.

## Measuring progress

```
tools/progress             # coverage
scripts/publish_progress.sh
```

Report **two** numbers, never one: total coverage, and the **C-only, executed-path**
subset. The first says how much of the game is understood; the second says how
much of the port can exist. Only the second predicts a playable, restylable
build. Add the disc-1 / disc-2 split to that report as soon as disc 2 work
begins.
