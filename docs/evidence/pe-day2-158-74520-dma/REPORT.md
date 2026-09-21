# DAY2-158 — Disc1 wall `func_80074520_dma_indirect_call` (tip `50b2a58`)

Status: **ROOT CAUSE IDENTIFIED — no code patch** (leaf `0x80191DC8` has no
high-confidence matched C in this repo or the khasinski donor).

Tip under dig: `50b2a58` (`cursor/movie-autonomous-stream-6f51` / DAY2-158o).
Throwaway branch: `dig/74520-dma-indirect`.

## Symptom (live Disc1)

After DAY2-158o cleared `func_801909B4_post_movie_title_cut`:

- One successful production C89C: `calls=1`, `out=7300`, `w0=0x38000720`
  (normal FMV001 frame-1 EOF pad; see `pe-day2-158o-c89c-eof-pad`).
- Title-cut path no longer the frontier.
- STOP: `Bootstrap_ReturnVoid4Indirect("func_80074520_dma_indirect_call", …)`
  → `PE_PORT_STOP_UNRESOLVED_BOUNDARY`.
- Multi-frame / `92934` TRACE not observed yet (see below — entry has no
  `Trace_Direct`).

## Where the STOP fires (not a missing `74520` body)

### Stub site

`pc_port/platform/pe_irq_delivery.c` — `DispatchDmaCallback`:

```c
/* recognized: 0x801214D4, 0x8007C214, 0x80076EE4 */
return GuestIndirectBoundary("func_80074520_dma_indirect_call",
                             "func_80074520", handler);
```

`GuestIndirectBoundary` → `Bootstrap_ReturnVoid4Indirect` +
`PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY)`.

`PE_func_80074520_Dispatch` / `func_80074520` themselves are **already a
complete 96-word semantic translation** (B53I-B2). The STOP name is the
*indirect DMA-table jalr*, not an unported `trapIntrDMA` leaf.

### Retail / donor identity for `func_80074520`

| Item | Value |
| --- | --- |
| VA | `0x80074520..0x8007469F` (96 words / `0x180`) |
| EXE offset | `0x64D20` |
| Body SHA-256 | `3dd9a2f9f85757a6e5c28f1fa6f48cf929370c6ad4da3ae8c08e08dc852259cd` |
| Psy-Q name | `trapIntrDMA` (khasinski `configs/USA/sym.main.txt`) |
| Donor | khasinski still lists `trapIntrDMA` as **asm** (`main.yaml`); helpers `startIntrDMA` / `setIntrDMA` / `memclrIntrDMA` are C |
| Port | `pc_port/platform/pe_irq_delivery.c` — DICR `(>>24)&0x7F` scan, W1C ack, live table `D_800956C0 + 4*ch`, resample, bus-error diagnostic tail |

Behavior summary: CPU source-3 handler that walks pending DMA channel flags,
acknowledges each with a fresh DICR W1C write, then `jalr`s the guest pointer
in the DMA callback slot. It is a **function-pointer dispatcher**, not GPU/MDEC
logic itself.

Recognized port leaves today:

| Handler | Channel role | Port |
| --- | --- | --- |
| `0x80076EE4` | DMA2 GPU pump | `PE_func_80076EE4_Pump` |
| `0x8007C214` | DMA3 CD stream | `func_8007C214` |
| `0x801214D4` | DMA1 movie-player slice | `movie_overlay_port.c` |
| **`0x80191DC8`** | **DMA1 title-overlay slice** | **MISSING** |

No `INCLUDE_ASM` for `74520` in this PC-port tree; no alternate stubbed body
elsewhere. Expanding the `74520` translation is the wrong next step.

## Caller chain from got_frame / C89C

```text
func_80192CE8
  jal func_801924F8
    … E0 poll / promote / got_frame …
    func_8010C0D8(0x80191DC8)          /* DecDCToutCallback → DMA slot 1 */
    … Stage-1b admit …
    func_8010C89C(...)                 /* calls=1, out=7300, pad EOF */
    func_8007C394(stream)
    EC: DBD=0, DBA++, DBC=1            /* DAY2-158o — clears title-cut */
    return 0
  post-E08 media loop:
    status = func_80192934()           /* DBA>=2 now enters body */
      func_8010BFA0(arena, mode)       /* DecDCTin → MDEC DMA0 + decode cmd */
      func_8010C01C(dst, words)        /* DecDCTout → program DMA1 output */
      poll:
        HostFB_PumpCdProgress()
          PE_MDEC_Service()
            LatchDMACompletionFlag(1)  /* channel 1 */
            PE_IRQ_BridgeDICRRisingEdge
          PE_IRQ_ServicePending…
            func_80073F00 → func_80074520
              handler = [0x800956C4] = 0x80191DC8
              DispatchDmaCallback → STOP dma_indirect_call
```

Notes:

1. C89C itself is a pure RAM worker (no DMA/IRQ). The wall is **after** the
   first successful decode, on the first multi-frame MDEC output delivery.
2. `92934` has **no** entry `Trace_Direct`, so live TRACE can look like
   “92934 not reached” even though the post-158o `DBA++` path has entered it
   and stopped inside the first `BFA0`/`C01C` → pump cycle.
3. `924F8` still skips the retail **slice-wait** between `7C394` and EC
   (commented in-port). That gap is related, but the live STOP on this tip is
   the later `92934` DecDCTOut path with slot 1 still holding `0x80191DC8`.

Provenance for the registration (not delivery):
`docs/evidence/pe-b54kah-decdctoutcallback/REPORT.md` —
`DMA_CALLBACK_SLOT_1=0x80191DC8_REGISTERED_NOT_DELIVERED`. Handoff / DAY2 docs
already name `80191DC8` as the unported title-overlay DMA1 callback (distinct
from player `1214D4`).

## What retail does next

After frame-1 VLC output is in the arena:

1. `92934` feeds that buffer to `DecDCTin` / `DecDCTout` (`BFA0` / `C01C`).
2. MDEC DMA1 completion raises DICR ch1; `trapIntrDMA` (`74520`) acks and
   calls the registered DecDCTout callback.
3. Title overlay’s `0x80191DC8` is that callback — same *role* as player
   `0x801214D4` (slice continue / final-slice / VRAM upload), different
   overlay state words (`801D14xx` / title BSS vs `801228xx` player BSS).
4. Only after slices finish does the `91B64` poll / next C89C multi-frame path
   proceed.

Do **not** invent `91DC8` hardware behavior from the player callback.

## Smallest honest next port step

**Wire the real leaf `func_80191DC8`, do not expand the `74520` stub.**

Concrete sequence:

1. Carve title overlay PE.IMG `[0x03D2,0x0457)` @ `0x8018EFF0` and recover
   exact `[0x80191DC8, next_jr_ra)` words + SHA (same method as B54KAH /
   pe-92934).
2. Transcribe matched C (or high-confidence CFG port) into e.g.
   `pc_port/game/boot/func_80191DC8_port.c` using title-overlay state, not a
   blind alias of `func_801214D4`.
3. Add `handler == 0x80191DC8u` → `func_80191DC8()` in `DispatchDmaCallback`
   (same shape as the existing `1214D4` arm).
4. Optional dig aid only: `Trace_Direct("func_80192934_enter")` before `BFA0`
   so live dumps stop looking like “92934 skipped.”
5. Keep Stage-1b / C89C admit gates untouched; no synthetic DMA callback.

**Not recommended:** treating `dma_indirect_call` as a reason to rewrite
`PE_func_80074520_Dispatch`, or swapping title registration to `1214D4`
(wrong overlay / wrong BSS).

## Patch decision

No draft guest-body patch on this dig: `0x80191DC8` has no matched C body
in-tree or in khasinski. Shipping a guessed slice callback would invent
hardware/overlay behavior.

**Fold follow-up (PR lane, tip after this report):** wire-only
`DispatchDmaCallback` arm for `0x80191DC8` → named boundary
`func_80191DC8` (does **not** invent the overlay leaf). Optional
`Trace_Direct("func_80192934_enter")` so live TRACE shows the media worker.
Real leaf still waits on PE.IMG carve / Decomp matched C.

## Verify anchors (already in tree)

- `test_B54KAH_dec_dct_out_callback_registration` — registers `91DC8`, asserts
  `CountOrderLog("func_80074520_dma_indirect_call") == 0` (registration ≠
  delivery).
- B53I-B2 oracle / `pc_port/docs/b53i_b2_dma_irq_delivery.md` — complete
  `74520` word count + hash.
- DAY2-158o report — first C89C `out=7300` + `DBA++` enabling this wall.

## Branch / SHA

```text
branch  dig/74520-dma-indirect
base    50b2a58 docs: DAY2-158o Linux suite counts (1354/1308/0/46)
report  docs/evidence/pe-day2-158-74520-dma/REPORT.md
```
